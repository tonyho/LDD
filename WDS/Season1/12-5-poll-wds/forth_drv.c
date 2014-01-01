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


//#include <linux/smp_lock.h>
#include <linux/workqueue.h>
#include <linux/poll.h>

//static struct class *forthdrv_class;
//static struct class_device	*forthdrv_class_dev;

static struct class *btn_class;
static struct class_device *btn_class_device;
//volatile unsigned long *gpfcon;
//volatile unsigned long *gpfdat;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，forth_drv_read将它清0 */
static volatile int ev_press = 0;


struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};


/* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
/* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

/*
 * K1,K2,K3,K4对应GPF1、GPF4、GPF2、GPF0
 */
struct pin_desc pins_desc[4] = {
	{S3C2410_GPF1, 0x01},
	{S3C2410_GPF4, 0x02},
	{S3C2410_GPF2, 0x03},
	{S3C2410_GPF0, 0x04},
};


/*
  * 确定按键值
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if (pinval)
	{
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;
	}

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int forth_drv_open(struct inode *inode, struct file *file)
{
	/* GPF1、GPF4、GPF2、GPF0为中断引脚 */
	request_irq(IRQ_EINT1, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K1", &pins_desc[0]);
	request_irq(IRQ_EINT4, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K2", &pins_desc[1]);
	request_irq(IRQ_EINT2, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K3", &pins_desc[2]);
	request_irq(IRQ_EINT0, buttons_irq, IRQ_TYPE_EDGE_BOTH, "K4", &pins_desc[3]);	

	return 0;
}

ssize_t forth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(button_waitq, ev_press);

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}


int forth_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT1, &pins_desc[0]);
	free_irq(IRQ_EINT4, &pins_desc[1]);
	free_irq(IRQ_EINT2, &pins_desc[2]);
	free_irq(IRQ_EINT0, &pins_desc[3]);
	return 0;
}

static unsigned forth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}



static struct file_operations sencod_drv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  forth_drv_open,     
	.read	 =	forth_drv_read,	   
	.release =  forth_drv_close,
	.poll    =  forth_drv_poll,
};

#define BTN_CLASS_NAME "buttons"

int major;
static int forth_drv_init(void)
{
	major = register_chrdev(0, "forth_drv", &sencod_drv_fops);

	//forthdrv_class = class_create(THIS_MODULE, "forth_drv");

	//forthdrv_class_dev = class_device_create(forthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	// class create
	btn_class = class_create(THIS_MODULE,BTN_CLASS_NAME);
	btn_class_device = device_create(btn_class,NULL,MKDEV(major,0),NULL,BTN_CLASS_NAME);
//	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
//	gpfdat = gpfcon + 1;

	return 0;
}

static void forth_drv_exit(void)
{
	unregister_chrdev(major, "forth_drv");
	//class_device_unregister(forthdrv_class_dev);
	//class_destroy(forthdrv_class);
	if(btn_class_device){
		device_destroy(btn_class,MKDEV(major,0));
	}
	if(btn_class){
		class_destroy(btn_class);
	}
//	iounmap(gpfcon);
	return 0;
}


module_init(forth_drv_init);

module_exit(forth_drv_exit);

MODULE_LICENSE("GPL");

