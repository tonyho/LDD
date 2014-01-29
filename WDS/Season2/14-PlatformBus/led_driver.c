#include <linux/module.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/suspend.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/cdev.h>

static volatile unsigned long *gpio_cfg,*gpio_data;
static unsigned long pinIndex;
static struct class *tq_led_class;
static struct class_device *tq_led_class_device;

static dev_t tq_led_dev_t;

static struct cdev *tq_led_cdev;


static int tq_led_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/* ≈‰÷√Œ™ ‰≥ˆ */
	*gpio_cfg &= ~(0x3<<(pinIndex*2));
	*gpio_cfg |=  (0x1<<(pinIndex*2));
	return 0;	
}

static ssize_t tq_led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("first_drv_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// µ„µ∆
		*gpio_data &= ~(1<<pinIndex);
	}
	else
	{
		// √µ∆
		*gpio_data |= (1<<pinIndex);
	}
	
	return 0;
}



static struct file_operations tq_led_fops ={
	.owner = THIS_MODULE,
	.open = tq_led_open,
	.write = tq_led_write,
};


static int tq_led_probe(struct platform_device *dev)
{
	struct resource *res;
	int ret = -1;
	res = platform_get_resource(dev,IORESOURCE_MEM,0);
	gpio_cfg = ioremap(res->start,res->end - res->start + 1);
	gpio_data= gpio_cfg + 1;

	res = platform_get_resource(dev,IORESOURCE_IRQ,0);
	pinIndex = res->start;

	
	ret = alloc_chrdev_region(&tq_led_dev_t,0,1,"tq_led");
	if(ret){
		printk(KERN_ERR "--[%s:%d] alloc error ret=%d\n",__FUNCTION__,__LINE__,ret);
	}

	tq_led_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	cdev_init(tq_led_cdev,&tq_led_fops);
	
	ret = cdev_add(tq_led_cdev,tq_led_dev_t,1);	
	if(ret){
			printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	}


	// class create
	tq_led_class = class_create(THIS_MODULE,"tq_led");
	tq_led_class_device = device_create(tq_led_class,NULL,tq_led_dev_t,NULL,"tq_led");
	return 0;
}

static int tq_led_remove(struct platform_device *dev)
{
	int ret = 0;
	printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	cdev_del(tq_led_cdev);
	printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	kfree(tq_led_cdev);
	printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	unregister_chrdev_region(tq_led_dev_t,1);
	printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	
	if(tq_led_class_device){
		//device_destroy(btn_class,btn_dev_t);
		printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	}
	if(tq_led_class){
		//class_destroy(btn_class);
		printk(KERN_ERR "--[%s:%d] cdev_add error ret=%d\n",__FUNCTION__,__LINE__,ret);
	}
	iounmap(gpio_cfg);

	return 0;
}

#define tq_led_resume NULL


static struct platform_driver tq_led_driver= {
	.probe	= tq_led_probe,
	.remove	= tq_led_remove,
	.resume	= tq_led_resume,
	.driver	= {
		.name	= "tq_led",
	},
};

static int __init tq_led_driver_init(void)
{
	return platform_driver_register(&tq_led_driver);
}

static void __exit tq_led_driver_exit(void)
{
	platform_driver_unregister(&tq_led_driver);
}

module_init(tq_led_driver_init);
module_exit(tq_led_driver_exit);

MODULE_DESCRIPTION("TonyHo Tq-Led Platform Driver example!");
MODULE_LICENSE("GPL");


