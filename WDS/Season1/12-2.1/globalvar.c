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




static int globalvar = 0;
int globalvar_num = 1;

static dev_t dev_g ;

//the gpio config and set
volatile unsigned long *gconf,*gdata;

MODULE_LICENSE("Dual BSD/GPL");

struct cdev *globalvar_dev;	/* allocated in scull_init_module */
struct class *gc;
struct class_device *gc_device;

static ssize_t globalvar_read(struct file *,char *,size_t,loff_t *);
static ssize_t globalvar_write(struct file *,const char *,size_t,loff_t *);

static struct file_operations globalvar_fops={
    .owner = THIS_MODULE,
    .read = globalvar_read,
    .write= globalvar_write,
};
//static struct device_attribute flag_attr = __ATTR(flag,S_IRUGO|S_IWUSR,read_flag,write_flag);
static int __init globalvar_init(void){
    int ret = -1;
    
    ret = alloc_chrdev_region(&dev_g,0,globalvar_num,"globalvar");
    if(ret){
        printk(KERN_ERR "globalvar region failed major=%d\n!!",MAJOR(dev_g));
    }
    else{
        printk(KERN_ERR "globalvar region Success major=%d\n!!",MAJOR(dev_g));
    }
	
    globalvar_dev = kmalloc(sizeof(struct cdev),GFP_KERNEL);
    cdev_init(globalvar_dev,&globalvar_fops);
    ret = cdev_add(globalvar_dev,dev_g,1);
    if(ret){
        printk(KERN_ERR "globalvar regchr failed!!");
    }
    else{
        printk(KERN_ERR "globalvar regchr Success!!\n");
    }
	gc = class_create(THIS_MODULE,"globalvar");
	gc_device = device_create(gc,NULL,dev_g,NULL,"globalvar");
	//device_create_file(gc_device,&flag_attr);
	gconf = (volatile unsigned long *)(ioremap(0x56000010,4));
	gdata = (volatile unsigned long *)(ioremap(0x56000014,4));

	//Set output
	//*gconf &= ~((0x3<<5*2) | (0x3<<6*2) | (0x3<<7*2) |(0x3<<8*2));
	//*gconf |=    ((0x1<<5*2) | (0x1<<6*2) | (0x1<<7*2) |(0x1<<8*2));

	//out put 0101
	//*gdata |=    ((0x1<<5*1) | (0x0<<6*1) | (0x0<<7*1) |(0x0<<8*1));
	//udelay(500);
	//*gdata &= ~((0x1<<5*1) | (0x1<<6*1) | (0x1<<7*1) |(0x1<<8*1));
	*gconf &= ~((0x3<<(5*2)) | (0x3<<(6*2)) | (0x3<<(7*2)) | (0x3<<(8*2)));
	*gconf |= ((0x1<<(5*2)) | (0x1<<(6*2)) | (0x1<<(7*2)) | (0x1<<(8*2)));


	//jiffies
    return ret;
}

static void __exit globalvar_exit(void){
   // int major_g = MAJOR(dev_g);
    cdev_del(globalvar_dev);
    kfree(globalvar_dev);
    unregister_chrdev_region(dev_g,globalvar_num);
	device_destroy(gc,dev_g);
	class_destroy(gc);
    printk(KERN_ERR "globalvar unregchr Success!!\n");
}

static ssize_t globalvar_read(struct file *filp,char *buf,size_t len,loff_t *off){
    int ret = -1;
	 printk(KERN_ERR "Read Enter !!\n");
    ret = copy_to_user(buf, &globalvar, sizeof(globalvar)); 
    if(ret){
		 printk(KERN_ERR "Copy_to_user Read error !!\n");
        return -EFAULT;    
    }
    return sizeof(globalvar);
}

static ssize_t globalvar_write(struct file *filp,const char *buf,size_t len,loff_t *off){
    int ret = -1;
		 printk(KERN_ERR "Write Enter !!\n");
    if(sizeof(buf)>sizeof(globalvar)){
        printk(KERN_ERR "You can only pass a int");
    }

    ret = copy_from_user(&globalvar, buf, sizeof(globalvar)); 
    if(ret){
	 printk(KERN_ERR "You can only pass a int copy_from_user error !!\n");
        return -EFAULT;    
    }
    printk(KERN_ERR "You input the buf = [%s],[%d]\n",buf,buf[0]);
	if(buf[0] == 49)
	{
		printk(KERN_ERR "Will close all leds\n");		
		*gdata |=    ((1<<5) | (1<<6*1) | (0x1<<7*1) |(0x1<<8*1));
	}
	if(buf[0] == 48)
	{
		printk(KERN_ERR "Will open all leds\n");
		*gdata &=   ~ ((0x1<<5*1) | (0x1<<6*1) | (0x1<<7*1) |(0x1<<8*1));
	}
    return sizeof(globalvar);
}

module_init(globalvar_init);
module_exit(globalvar_exit);
MODULE_LICENSE("GPL");

