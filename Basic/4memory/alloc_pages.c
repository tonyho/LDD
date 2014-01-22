#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/gfp.h> 
#include <linux/mm_types.h> 
#include <linux/mm.h> 
static int __init alloc_pages_init(void); 
static void __exit alloc_pages_exit(void);  
struct page * pages = NULL;
int __init alloc_pages_init(void) 
{ 
    pages = alloc_pages(GFP_KERNEL,3);  //分配8个物理页    
    if(!pages) 
    { 
        return -ENOMEM; 
    } 
    else 
    { 
        printk("<0>alloc_pages Successfully!\n"); 
        printk("<0>pages = 0x%lx\n",(unsigned long)pages);  //输出pages值，即第一个页的页描述符地址

        //输出mem_map数组的起始地址
        printk("<0>mem_map = 0x%lx\n",(unsigned long)mem_map); 

        //分配的第一个页相对mem_map数组起始位置的偏移
        printk("<0>pages-mem_map = 0x%lx\n",(unsigned long)pages-(unsigned long)mem_map);

        //第一个页pages的物理地址
        printk("(pages-mem_map)*4096 = 0x%lx\n",(unsigned long)(pages-mem_map)*4096);

        //第一个页pages的逻辑地址
        printk("<0>page_address(pages) = 0x%lx\n",(unsigned long)page_address(pages));
    }     
    return 0; 
}
void __exit alloc_pages_exit(void) 
{ 
    if(pages) 
    {
        __free_pages(pages,3);   //释放所分配的8个页 
        printk("<0>__free_pages ok!\n"); 
    }
    printk("<0>exit\n"); 
}

module_init(alloc_pages_init); 
module_exit(alloc_pages_exit);
MODULE_LICENSE("GPL"); 
