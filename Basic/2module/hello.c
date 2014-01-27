#include <linux/init.h>
#include <linux/module.h>
#include <linux/smp.h>

MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void){
    int cpunum = smp_processor_id();
    printk(KERN_EMERG "Hello world CPUNum=%d\n",cpunum);
    return 0;

}

static int hello_exit(void){
    int cpunum = smp_processor_id();
    printk(KERN_EMERG "Goodbye, hello world! CPUnum=%d\n",cpunum);
    return 0;
}

module_init(hello_init);
module_exit(hello_exit);

