#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>

static struct input_dev *tq_ts_dev;
#define tq2440ts_name "TQ2440-TouchPanel" //Question1

struct ADC_TS_regs  {
	unsigned long rADCCON; //0x5800000
	unsigned long rADCTSC; //0x5800004
	unsigned long rADCDLY; //0x5800008 ADCDAT0
	unsigned long rADCDATA0; //0x580000c	
	unsigned long rADCDATA1; //0x5800010 		
	unsigned long rADCUPDN;// ADC TOUCH SCREEN UP-DOWN INT CHECK REGISTER 0x5800014
};

static volatile struct ADC_TS_regs *pRegs; 

#define UP   1
#define DOWN 0


static int wait_stylus_UPorDOWN(int UPorDown){
	if(UPorDown == DOWN){
		//pRegs->rADCTSC &= ~(1<<8); 
		//printk(KERN_ERR "WaitforUpDown for Down\n");
		pRegs->rADCTSC = 0xd3; 
	}
	else if(UPorDown == UP){
		//printk(KERN_ERR "WaitforUpDown for Up\n");
		//pRegs->rADCTSC |= (1<<8);
		pRegs->rADCTSC = 0x1d3; 
	}
	else{
		printk(KERN_ERR "WaitforUpDown Parameter error\n");
		return -1;
	}
	return 0;
}

static void Enable_start_ADC(void){
	//pRegs->rADCCON |= 1;
	pRegs->rADCCON |= (1<<0);
}
static void enter_measure_xy_mode(void){
//	pRegs->rADCTSC |= ((1<<3)|(1<<2));
	pRegs->rADCTSC = (1<<3)|(1<<2);//((1<<3)|(1<<2));

}



static irqreturn_t stylus_action_irq_handler(int irq, void *dev_id){
	printk(KERN_ERR "Stylus Action\n");

	if(!(pRegs->rADCDATA0 & (1<<15))){//Sytlus down detected
		printk(KERN_DEBUG "Stylus Down\n");
		
		//enter_measure_xy_mode();
		//Enable_start_ADC();
		wait_stylus_UPorDOWN(UP);
		
	}
	if((pRegs->rADCDATA0 & (1<<15))){//else{
		printk(KERN_DEBUG "Stylus Up\n");
		//TS_PostEvent(UP,0,0);
		wait_stylus_UPorDOWN(DOWN);
	}
	
	return IRQ_HANDLED;
}
static irqreturn_t adc_irq_handler(int irq, void *dev_id){
	printk(KERN_ERR "ADC Irq\n");
		if(!(pRegs->rADCDATA0 & (1<<15))){//Sytlus down detected
		//TS_PostEvent(DOWN,pRegs->rADCDATA0&0x3ff,pRegs->rADCDATA1&0x3ff);
		wait_stylus_UPorDOWN(UP);
	}
	else{
		//TS_PostEvent(UP,0,0);
	}
	return IRQ_HANDLED;
}
		

static int __init tq_ts_init(void)
{	
	struct clk* clk;
	printk(KERN_ERR "Init........\n");

	//1 Add a input device
	tq_ts_dev = input_allocate_device();
	if(tq_ts_dev == NULL){
		printk(KERN_ERR "Error to allocte input device\n");
	}
	
	//2 Set input device: Input Event type, Subtype
	
	set_bit(EV_KEY, tq_ts_dev->evbit);
	set_bit(EV_ABS, tq_ts_dev->evbit);
	
	set_bit(BTN_TOUCH, tq_ts_dev->keybit);
	
	input_set_abs_params(tq_ts_dev, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(tq_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(tq_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);

	input_register_device(tq_ts_dev);
	
	//0 Enable Clk
	clk = clk_get(NULL, "adc");
	clk_enable(clk);

	//3 Set the ADC regs
	//Using the 
	pRegs = (struct ADC_TS_regs *)ioremap(0x5800000,sizeof(struct ADC_TS_regs));
	/*
	* Prescaler
	*A/D converter freq. = 50MHz/(49+1) = 1MHz
	*Conversion time = 1/(1MHz / 5cycles) = 1/200KHz = 5 us
	*/
	pRegs->rADCCON = (1<<14)|(49<<6);
	
	//4 

	//5 Request the IRQ
	request_irq(IRQ_TC , stylus_action_irq_handler,IRQF_SAMPLE_RANDOM, "TQ-TouchPanel", NULL);
	request_irq(IRQ_ADC, adc_irq_handler,          IRQF_SAMPLE_RANDOM, "TQ-ADC",        NULL);



	// Set the TouchPanel regs
	/*Ref the Page442  Waiting for Interrupt Mode : rADCTSC=0xd3 Wating for stylus down*/
	pRegs->rADCTSC = 0xd3;
	

	return 0;
}

static void __exit tq_ts_exit(void)
{
	
	free_irq(IRQ_TC,NULL);
	free_irq(IRQ_ADC,NULL);
	iounmap(pRegs);
	input_unregister_device(tq_ts_dev);
	input_free_device(tq_ts_dev);

}

module_init(tq_ts_init);
module_exit(tq_ts_exit);

MODULE_LICENSE("GPL");


