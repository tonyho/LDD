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

//For input subsystem
#include <linux/input.h>

struct timer_list btn_timer;
static unsigned int pinValue;

struct btn_pin {
	unsigned int pin;
	char *KeyName;
	unsigned int KeyValue;
};

static struct btn_pin * btnpin;

struct btn_pin btn_pin_array[4] = {
		{S3C2410_GPF1,"l",KEY_L},
		{S3C2410_GPF4,"s",KEY_S},
		{S3C2410_GPF2,"Enter",KEY_ENTER},
		{S3C2410_GPF0,"L",KEY_LEFTSHIFT}		
};

struct input_dev *btn_input_dev;

static void btn_timer_server(unsigned long data){
	struct btn_pin *p_btnpin = btnpin;

	printk(KERN_ERR "Enter the timer server\n");

	if(p_btnpin == NULL){
		printk(KERN_ERR "Enter the timer server p=NULL\n");
		return;
	}
	
	pinValue = s3c2410_gpio_getpin(btnpin->pin);

	//btn down
	if(pinValue == 0){
		btnpin->KeyValue = btnpin->KeyValue;
		//printk(KERN_ERR "irq=%d, keyvalue=0x%x, down\n",irq_all,btnpin->KeyValue);
				/* 松开 : 最后一个参数: 0-松开, 1-按下 */
		input_event(btn_input_dev, EV_KEY, btnpin->KeyValue, 1);
		input_sync(btn_input_dev);
	}
	else{			
		//printk(KERN_ERR "irq=%d, keyvalue=0x%x, up\n",irq_all,btnpin->KeyValue);		
		input_event(btn_input_dev, EV_KEY, btnpin->KeyValue, 0);
		input_sync(btn_input_dev);
	}

}

static irqreturn_t btn_handler(int irq, void * dev_id){
	btnpin = (struct btn_pin *)dev_id;

	printk(KERN_ERR "irq number=%d\n",irq);
	mod_timer(&btn_timer, jiffies+HZ/20);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int btn_input_init(void){
	btn_input_dev = input_allocate_device();
	set_bit(EV_KEY,btn_input_dev->evbit);

	set_bit(KEY_L,btn_input_dev->keybit); //0x26
	set_bit(KEY_S,btn_input_dev->keybit); //0x1f
	set_bit(KEY_LEFTSHIFT,btn_input_dev->keybit); //0x2a
	set_bit(KEY_ENTER,btn_input_dev->keybit); //0x1c

	input_register_device(btn_input_dev);

	init_timer(&btn_timer);
	btn_timer.function = btn_timer_server;
	add_timer(&btn_timer);

	
	request_irq(IRQ_EINT1, btn_handler, IRQ_TYPE_EDGE_BOTH, btn_pin_array[0].KeyName, &btn_pin_array[0]);
	request_irq(IRQ_EINT4, btn_handler, IRQ_TYPE_EDGE_BOTH, btn_pin_array[1].KeyName, &btn_pin_array[1]);	
	request_irq(IRQ_EINT2, btn_handler, IRQ_TYPE_EDGE_BOTH, btn_pin_array[2].KeyName, &btn_pin_array[2]);
	request_irq(IRQ_EINT0, btn_handler, IRQ_TYPE_EDGE_BOTH, btn_pin_array[3].KeyName, &btn_pin_array[3]);

	return 0;
}

static void btn_input_exit(void){

}

module_init(btn_input_init);
module_exit(btn_input_exit);

MODULE_LICENSE("GPL");

