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


struct resource	tq_led_resource[] = {
	[0] ={
		.start = 0x56000010,
		.end = 0x56000010 + 8 -1,
		.flags = IORESOURCE_MEM,
		},
	[1] ={
		.start = 5,
		.end = 5,
		.flags = IORESOURCE_IRQ,
		}
};

static void tq_led_release(struct device * dev)
{
}

struct platform_device tq_led_dev ={
	.name = "tq_led",
	.id = -1,
	.num_resources = ARRAY_SIZE(tq_led_resource),
	.resource = tq_led_resource,
	.dev = { 
    	.release = tq_led_release, 
	},
};

static int __init tq_led_device_init(void)
{
	//led_dev = platform_device_alloc("tq_led", -1);
	int ret;
	ret = platform_device_register(&tq_led_dev);

	return 0;
}

static void __exit tq_led_device_exit(void)
{
	platform_device_unregister(&tq_led_dev);

}

module_init(tq_led_device_init);
module_exit(tq_led_device_exit);

MODULE_LICENSE("GPL");



