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
    pages = alloc_pages(GFP_KERNEL,3);  //����8������ҳ    
    if(!pages) 
    { 
        return -ENOMEM; 
    } 
    else 
    { 
        printk("<0>alloc_pages Successfully!\n"); 
        printk("<0>pages = 0x%lx\n",(unsigned long)pages);  //���pagesֵ������һ��ҳ��ҳ��������ַ

        //���mem_map�������ʼ��ַ
        printk("<0>mem_map = 0x%lx\n",(unsigned long)mem_map); 

        //����ĵ�һ��ҳ���mem_map������ʼλ�õ�ƫ��
        printk("<0>pages-mem_map = 0x%lx\n",(unsigned long)pages-(unsigned long)mem_map);

        //��һ��ҳpages�������ַ
        printk("(pages-mem_map)*4096 = 0x%lx\n",(unsigned long)(pages-mem_map)*4096);

        //��һ��ҳpages���߼���ַ
        printk("<0>page_address(pages) = 0x%lx\n",(unsigned long)page_address(pages));
    }     
    return 0; 
}
void __exit alloc_pages_exit(void) 
{ 
    if(pages) 
    {
        __free_pages(pages,3);   //�ͷ��������8��ҳ 
        printk("<0>__free_pages ok!\n"); 
    }
    printk("<0>exit\n"); 
}

module_init(alloc_pages_init); 
module_exit(alloc_pages_exit);
MODULE_LICENSE("GPL"); 
