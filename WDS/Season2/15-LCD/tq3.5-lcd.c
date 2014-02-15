#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>

struct lcd_regs {
	unsigned long	lcdcon1;	
	unsigned long	lcdcon2;	
	unsigned long	lcdcon3;	
	unsigned long	lcdcon4;	
	unsigned long	lcdcon5;    
	unsigned long	lcdsaddr1;    
	unsigned long	lcdsaddr2;    
	unsigned long	lcdsaddr3;    
	unsigned long	redlut;    
	unsigned long	greenlut;    
	unsigned long	bluelut;    
	unsigned long	reserved[9];    
	unsigned long	dithmode;    
	unsigned long	tpal;    
	unsigned long	lcdintpnd;    
	unsigned long	lcdsrcpnd;    
	unsigned long	lcdintmsk;    
	unsigned long	lpcsel;
};


static struct fb_info * tq_fb_info;
static volatile unsigned long *gpioc_cfg;
static volatile unsigned long *gpiod_cfg;
static volatile unsigned long *gpioc_data;
static volatile unsigned long *gpiog_cfg;

struct lcd_regs *lcd_regs;


static u32 pseudo_palette[16];


/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int tq_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	
	if (regno > 16)
		return 1;

	/* ��red,green,blue��ԭɫ�����val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	//((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;
	return 0;
}

static struct fb_ops tq_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= tq_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static int __init tq_lcd_init(void)
{
	int ret;

	tq_fb_info = framebuffer_alloc(sizeof(struct fb_info), NULL);
	if (!tq_fb_info) {
		printk(KERN_ERR "Cannot allocate memory\n");
	}

	//1. Fill the via data
	tq_fb_info->var.xres = 320;
	tq_fb_info->var.yres = 240;
	tq_fb_info->var.xres = 320;
	tq_fb_info->var.yres = 240;	

	tq_fb_info->var.bits_per_pixel = 32; //3*8=24(Red Green Blue 3types , which is 8 bit) extend to 32 for align

	/* RGB:888 */
	tq_fb_info->var.red.offset     = 16;
	tq_fb_info->var.red.length     = 8;
	
	tq_fb_info->var.green.offset   = 8;
	tq_fb_info->var.green.length   = 8;

	tq_fb_info->var.blue.offset    = 0;
	tq_fb_info->var.blue.length    = 8;

	tq_fb_info->var.activate       = FB_ACTIVATE_NOW;

	//2. Fill the fix data
	//Question1: strcpy(s3c_lcd->fix.id, "mylcd");

	strcpy(tq_fb_info->fix.id,"tqLCD");
	tq_fb_info->fix.type = FB_TYPE_PACKED_PIXELS;		
	tq_fb_info->fix.smem_len = 320*240*32/8;        /* TQ2440��LCDλ����24,����2440������4�ֽڼ�32λ(�˷�1�ֽ�) */
	tq_fb_info->fix.visual   = FB_VISUAL_TRUECOLOR; /* TFT */
	tq_fb_info->fix.line_length = 320*4;

	//3.Set Ops
	tq_fb_info->fbops = &tq_lcdfb_ops;

	//4. Other setting
	tq_fb_info->pseudo_palette = pseudo_palette; 
	//s3c_lcd->screen_base  = ;  /* �Դ�������ַ */ 
	tq_fb_info->screen_size   = 320*240*32/8;

	//5. Pin Cfg
	//gpioc_cfg = ioremap(0x56000010, 8);
	
	gpioc_cfg = ioremap(0x56000020, 4);
	gpioc_data= gpioc_cfg+1;
	gpiod_cfg = ioremap(0x56000030, 4);
	gpiog_cfg = ioremap(0x56000060, 4);

	*gpioc_cfg = 0xaaaaaaaa;
	*gpiod_cfg = 0xaaaaaaaa;
	*gpiog_cfg |= (3<<(4*2));

	//5. LCD Control
	lcd_regs = ioremap(0x4D000000, sizeof(struct lcd_regs));

	//C1
	/* bit[17:8]: VCLK = HCLK / [(CLKVAL+1) x 2], LCD�ֲ�P22 (Dclk=6.4MHz)
	 *            5MHz(200ns) = 100MHz / [(CLKVAL+1) x 2] //ȡ5MHz
	 *            CLKVAL = 10 -1=9
	 * bit[6:5]: 0b11, TFT LCD
	 * bit[4:1]: 0b1101, 24 bpp for TFT
	 * bit[0]  : 0 = Disable the video output and the LCD control signal.
	 */
	lcd_regs->lcdcon1 = (9<<8) | (3<<5) | (0xd<<1);
	//ret = register_framebuffer(tq_fb_info);
	
	/* ��ֱ�����ʱ�����
		 * bit[31:24]: VBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
		 *			   LCD�ֲ� tvb=15
		 *			   VBPD=14
		 * bit[23:14]: ������, 240, ����LINEVAL=240-1=239
		 * bit[13:6] : VFPD, �������һ������֮���ٹ��೤ʱ��ŷ���VSYNC
		 *			   LCD�ֲ�tvf=12, ����VFPD=12-1=11
		 * bit[5:0]  : VSPW, VSYNC�źŵ�������, LCD�ֲ�tvp=3, ����VSPW=3-1=2
		 */
		lcd_regs->lcdcon2  = (14<<24) | (239<<14) | (11<<6) | (2<<0);

	
	/* ˮƽ�����ʱ�����
	 * bit[25:19]: HBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
	 *             LCD�ֲ� thb=38
	 *             HBPD=37
	 * bit[18:8]: ������, 480, ����HOZVAL=320-1=319
	 * bit[7:0] : HFPD, �������һ�������һ����������֮���ٹ��೤ʱ��ŷ���HSYNC
	 *             LCD�ֲ�thf=20, ����HFPD=20-1=19
	 */
	lcd_regs->lcdcon3 = (37<<19) | (319<<8) | (19<<0);

	/* ˮƽ�����ͬ���ź�
	 * bit[7:0]	: HSPW, HSYNC�źŵ�������, LCD�ֲ�Thp=30, ����HSPW=30-1=29
	 */	
	lcd_regs->lcdcon4 = 29;

	/* �źŵļ��� 
	 * bit[11]: 1=565 format, ����24bpp���������
	 * bit[10]: 0 = The video data is fetched at VCLK falling edge
	 * bit[9] : 1 = HSYNC�ź�Ҫ��ת,���͵�ƽ��Ч 
	 * bit[8] : 1 = VSYNC�ź�Ҫ��ת,���͵�ƽ��Ч 
	 * bit[6] : 0 = VDEN���÷�ת
	 * bit[3] : 0 = PWREN���0
	 *
	 * BSWP = 0, HWSWP = 0, BPP24BL = 0 : ��bpp=24ʱ,2440���ÿһ�����ط���32λ��4�ֽ�,��һ���ֽ��ǲ�ʹ�õ�? ��2440�ֲ�P412
         * bit[12]: 0, LSB valid, ������ֽڲ�ʹ��
	 * bit[1] : 0 = BSWP
	 * bit[0] : 0 = HWSWP
	 */
	lcd_regs->lcdcon5 = (0<<10) | (1<<9) | (1<<8) | (0<<12) | (0<<1) | (0<<0);

	/* 6. �����Դ�(framebuffer), ���ѵ�ַ����LCD������ */
	tq_fb_info->screen_base = dma_alloc_writecombine(NULL, tq_fb_info->fix.smem_len, (dma_addr_t*)&tq_fb_info->fix.smem_start, GFP_KERNEL);
	
	lcd_regs->lcdsaddr1  = (tq_fb_info->fix.smem_start >> 1) & ~(3<<30);
	lcd_regs->lcdsaddr2  = ((tq_fb_info->fix.smem_start + tq_fb_info->fix.smem_len) >> 1) & 0x1fffff;
	lcd_regs->lcdsaddr3  = (320*32/16);  /* һ�еĳ���(��λ: 2�ֽ�) */	

	lcd_regs->lcdcon1 |= (1<<0); /* ʹ��LCD������ */
	lcd_regs->lcdcon5 |= (1<<3); /* ʹ��LCD����: LCD_PWREN */

	/* 7. ע�� */
	register_framebuffer(tq_fb_info);

	return 0;
}

static void __exit tq_lcd_exit(void)
{
	lcd_regs->lcdcon1 &= ~(1<<0); /* �ر�LCD������ */
	lcd_regs->lcdcon1 &= ~(1<<3); /* �ر�LCD���� */
	
	unregister_framebuffer(tq_fb_info);	
	iounmap(gpioc_cfg);
	iounmap(gpiod_cfg);
	iounmap(gpiog_cfg);
	iounmap(lcd_regs);
	dma_free_writecombine(NULL, tq_fb_info->fix.smem_len, tq_fb_info->screen_base, tq_fb_info->fix.smem_start);
	framebuffer_release(tq_fb_info);


}

module_init(tq_lcd_init);
module_exit(tq_lcd_exit);

MODULE_LICENSE("GPL");


