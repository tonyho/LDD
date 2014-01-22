#include <linux/init.h>
#include <linux/module.h>
#include<linux/mm.h>
#include<linux/highmem.h>
#include<linux/mm_types.h>
#include<linux/gfp.h>
#include<linux/cdev.h>
#include<linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");

#define KSTR ("Hello From the Kernel Virtual Space")

static struct cdev *pcdev;
static  dev_t ndev;
static struct page *pg;
static struct timer_list timer;



void timer_hander(unsigned long data){
	printk(KERN_ERR "[%s:%d] %s\n",__FUNCTION__,__LINE__,(char *)data);
	timer.expires = 3 * HZ+jiffies;
	add_timer(&timer);
}

static int mmap_demo_open(struct inode *inode, struct file* filp){
    return 0;
}

static int mmap_demo_release(struct inode *inode, struct file* filp){
    return 0;
}

static int mmap_demo_mmap(struct file* filp,struct vm_area_struct *vma){
	int err =0 ;
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end-start;
	err = remap_pfn_range(vma, start, vma->vm_pgoff, size, vma->vm_page_prot);
    return err;
}


static struct file_operations mmap_demo_fops={
	.owner = THIS_MODULE,
	.open = mmap_demo_open,
	.release=mmap_demo_release,
	.mmap=mmap_demo_mmap,		
};


static int mmap_demo_init(void){
	int err=0;
	char * kstr;
	printk(KERN_EMERG "Hello world\n");
	pg = alloc_pages(GFP_HIGHUSER,0);
	SetPageReserved(pg);

	kstr = (char *)kmap(pg);
	strcpy(kstr,KSTR);
	printk(KERN_ERR "kpa=0x%X, kernel string=%s\n",page_to_phys(pg),kstr);

	pcdev = cdev_alloc();
	cdev_init(pcdev, &mmap_demo_fops);
	alloc_chrdev_region(&ndev, 0, 1, "mmap_demo");
	pcdev->owner = THIS_MODULE;
	cdev_add(pcdev,ndev,1);

	init_timer(&timer);
	timer.expires = HZ*3+jiffies;
	timer.function = timer_hander;
	timer.data = (unsigned long)kstr;
	add_timer(&timer);

	return err;
}

static void mmap_demo_exit(void){
    printk(KERN_EMERG "Goodbye, hello world!\n");
	del_timer_sync(&timer);
	cdev_del(pcdev);
	unregister_chrdev_region(ndev,1);
	kunmap(pg);
	ClearPageReserved(pg);
	__free_pages(pg,0);
}

module_init(mmap_demo_init);
module_exit(mmap_demo_exit);

