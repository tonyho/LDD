#include <linux/version.h>

#include <linux/sched.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#if 0
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
#include <linux/config.h>
#else
#include <linux/autoconf.h>
#endif
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
//#include <asm/hardware.h>

static struct cdev *btn_cdev;
static dev_t btn_dev_t;
static struct class *btn_class;
static struct class_device *btn_class_device;

 volatile unsigned long *btn_cfg_base;
 volatile unsigned long *btn_data_base;


static ssize_t btn_read(struct file *file, char __user *user, size_t size,loff_t*o){
	unsigned char keyValue[4]={100,100,100,100};
	unsigned long ulvalue = *btn_data_base;

	if(size != sizeof(keyValue)){
		printk(KERN_ERR "--[%s:%d] Size error\n",__FUNCTION__,__LINE__);
		return -EINVAL;
	}
	keyValue[0] = ( ulvalue & (0x1<<1)) ? 1 : 0;;
	keyValue[1] = ( ulvalue & (0x1<<4)) ? 1 : 0;
	keyValue[2] = ( ulvalue & (0x1<<2)) ? 1 : 0;
	keyValue[3] = ( ulvalue & (0x1<<0)) ? 1 : 0;

	copy_to_user(user,keyValue,sizeof(keyValue));
	return sizeof(keyValue);
}

static ssize_t btn_write(struct file *file, const char __user *in, size_t size, loff_t *off){
	return 0;

}

static struct file_operations btn_ops = {
	.owner = THIS_MODULE,
	.read = btn_read,
	.write = btn_write,
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
	btn_class = class_create(THIS_MODULE,"btn_query");
	btn_class_device = device_create(btn_class,NULL,btn_dev_t,NULL,"btn_query");

	// Io port init
	btn_cfg_base = (volatile unsigned long *)ioremap(0x56000050,4);
	btn_data_base = (volatile unsigned long *)ioremap(0x56000054,4);

	// IO input
	*btn_cfg_base &= ~((0x3<<(0*2))|(0x3<<(1*2))|(0x3<<(2*2))|(0x3<<(4*2))) ;
	//btn_cfg_base &= ~((0x3<<0)|(0x3<<1)|(0x3<<2)|(0x3<<4)) ;

	return 0;
}

void __init btn_exit(void){
	//printk(KERN_ERR "--[%s:%d] Enter \n",__FUNCTION__,__LINE__);
	cdev_del(btn_cdev);
	//printk(KERN_ERR "--[%s:%d] Enter \n",__FUNCTION__,__LINE__);
	kfree(btn_cdev);
	//printk(KERN_ERR "--[%s:%d] Enter \n",__FUNCTION__,__LINE__);
	unregister_chrdev_region(btn_dev_t,1);
	//printk(KERN_ERR "--[%s:%d] Enter \n",__FUNCTION__,__LINE__);
	
	if(btn_class_device){
		device_destroy(btn_class,btn_dev_t);
		//printk(KERN_ERR "--[%s:%d] Enter \n",__FUNCTION__,__LINE__);
	}
	if(btn_class){
		class_destroy(btn_class);
		//printk(KERN_ERR "--[%s:%d] Enter \n",__FUNCTION__,__LINE__);
	}
}

module_init(btn_init);
module_exit(btn_exit);

MODULE_LICENSE("GPL");

