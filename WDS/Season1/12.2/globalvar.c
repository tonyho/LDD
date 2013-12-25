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

//#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

static int globalvar = 0;
int globalvar_major = 0;
int globalvar_minor = 0;
int globalvar_num = 1;

static dev_t dev_g ;


MODULE_LICENSE("Dual BSD/GPL");

//#define MAJOR_NUM 254

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
    //dev_t dev ;
    //dev =MKDEV(globalvar_major,globalvar_minor);
    ret = alloc_chrdev_region(&dev_g,0,globalvar_num,"globalvar");
    if(ret){
        printk(KERN_ERR "globalvar region failed major=%d\n!!",MAJOR(dev_g));
    }
    else{
        printk(KERN_ERR "globalvar region Success major=%d\n!!",MAJOR(dev_g));
	//dev_g=dev;
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
	//gc = class_create(THIS_MODULE,"globalvar");
	//gc_device = device_create(gc,NULL,dev,NULL,"GC");
	//device_create_file(gc_device,&flag_attr);
    return ret;
}

static void __exit globalvar_exit(void){
#if 0
    int ret = -1;
#endif
    //dev_t dev;
    //dev =MKDEV(globalvar_major,globalvar_minor);
    int major_g = MAJOR(dev_g);
    cdev_del(globalvar_dev);
    kfree(globalvar_dev);
    unregister_chrdev_region(dev_g,globalvar_num);

#if 0
    if(ret){
        printk(KERN_ERR "globalvar unregchr failed!!");
    }
    else{
        printk(KERN_ERR "globalvar unregchr Success!!");
    }
#endif
    printk(KERN_ERR "globalvar unregchr Success!!\n");
}


static ssize_t globalvar_read(struct file *filp,char *buf,size_t len,loff_t *off){
    int ret = -1;
    ret = copy_to_user(buf, &globalvar, sizeof(globalvar)); 
    if(ret){
        return -EFAULT;    
    }
    return sizeof(globalvar);
}

static ssize_t globalvar_write(struct file *filp,const char *buf,size_t len,loff_t *off){
    int ret = -1;
    if(sizeof(buf)>sizeof(globalvar)){
        printk(KERN_ERR "You can only pass a int");
    }

    ret = copy_from_user(&globalvar, buf, sizeof(globalvar)); 
    if(ret){
        return -EFAULT;    
    }
    return sizeof(globalvar);
}

module_init(globalvar_init);
module_exit(globalvar_exit);
MODULE_LICENSE("GPL");

