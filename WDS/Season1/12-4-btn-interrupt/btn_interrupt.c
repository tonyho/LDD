#include <linux/version.h>

#include <linux/sched.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>



//#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */
#include <asm/io.h>
#include <asm/delay.h>
#include <mach/regs-gpio.h>
#include<linux/irq.h>
#include<asm/irq.h>
#include<mach/hardware.h>
#include<linux/miscdevice.h>
#include<linux/interrupt.h>




static struct cdev *btn_cdev;
static dev_t btn_dev_t;
static struct class *btn_class;
static struct class_device *btn_class_device;

 volatile unsigned long *btn_cfg_base;
 volatile unsigned long *btn_data_base;

#define BTN_CLASS_NAME "btn_interrupt"

struct btn_pin {
	unsigned int pin;
	unsigned int KeyValue;
};

struct btn_pin btn_pin_array[4] = {
		{S3C2410_GPF1,1},
		{S3C2410_GPF4,2},
		{S3C2410_GPF2,3},
		{S3C2410_GPF0,4}		
};

static unsigned int pinValue;
static volatile int ev_press = 0;

static DECLARE_WAIT_QUEUE_HEAD(btn_wait_irq);

static ssize_t btn_read(struct file *file, char __user *user, size_t size,loff_t*o){
	if(size != sizeof(pinValue)){
		printk(KERN_ERR "--[%s:%d] Size error\n",__FUNCTION__,__LINE__);
		return -EINVAL;
	}

	wait_event_interruptible(btn_wait_irq,ev_press);
	ev_press = 0;
	
	copy_to_user(user,pinValue,sizeof(pinValue));

	return sizeof(pinValue);
}

static ssize_t btn_write(struct file *file, const char __user *in, size_t size, loff_t *off){
	return 0;
}

static int btn_open(struct inode *inode, struct file *file){		
	return 0;
}

static irqreturn_t btn_handler(int irq, void * dev_id){
	struct btn_pin *btnpin = (struct btn_pin *)dev_id;

	pinValue = s3c2410_gpio_getpin(btnpin->pin);

	//btn down
	if(pinValue == 0){
		pinValue = btnpin->KeyValue;
		printk(KERN_ERR "irq=%d, down\n",irq);
	}
	else{
		pinValue = btnpin->KeyValue | 0x80;		
		printk(KERN_ERR "irq=%d, down\n",irq);		
	}
	
	wake_up_interruptible(&btn_wait_irq);
	ev_press = 1;
	return IRQ_RETVAL(IRQ_HANDLED);
}


int btn_close(struct inode *inode, struct file *file){
	free_irq(IRQ_EINT1, &btn_pin_array[0]);
	free_irq(IRQ_EINT4, &btn_pin_array[1]);
	free_irq(IRQ_EINT2, &btn_pin_array[2]);
	free_irq(IRQ_EINT0, &btn_pin_array[3]);	
}

static struct file_operations btn_ops = {
	.owner = THIS_MODULE,
	.read = btn_read,
	.write = btn_write,
	.release = btn_close,
	.open = btn_open,
};

int __init btn_init(void){
	int ret;
	//printk(KERN_ERR "------------\n");
	//printk(KERN_ERR "--[%s:%d] Enter \n",__FUNCTION__,__LINE__);
	ret = alloc_chrdev_region(&btn_dev_t,0,1,"btn_query");
	if(ret){
		printk(KERN_ERR "--[%s:%d] alloc error ret=%d\n",__FUNCTION__,__LINE__,ret);
	}

	btn_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	cdev_init(btn_cdev,&btn_ops);
	
	ret = cdev_add(btn_cdev,btn_dev_t,1);	
	if(ret){
			printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	}

	// class create
	btn_class = class_create(THIS_MODULE,BTN_CLASS_NAME);
	btn_class_device = device_create(btn_class,NULL,btn_dev_t,NULL,BTN_CLASS_NAME);

	request_irq(IRQ_EINT1, btn_handler, IRQ_TYPE_EDGE_BOTH, "K1", &btn_pin_array[0]);
	request_irq(IRQ_EINT4, btn_handler, IRQ_TYPE_EDGE_BOTH, "K2", &btn_pin_array[1]);	
	request_irq(IRQ_EINT2, btn_handler, IRQ_TYPE_EDGE_BOTH, "K3", &btn_pin_array[2]);
	request_irq(IRQ_EINT0, btn_handler, IRQ_TYPE_EDGE_BOTH, "K4", &btn_pin_array[3]);

	return 0;
}

void __init btn_exit(void){
	cdev_del(btn_cdev);
	kfree(btn_cdev);
	unregister_chrdev_region(btn_dev_t,1);
	
	if(btn_class_device){
		device_destroy(btn_class,btn_dev_t);
	}
	if(btn_class){
		class_destroy(btn_class);
	}
}

module_init(btn_init);
module_exit(btn_exit);

MODULE_LICENSE("GPL");

