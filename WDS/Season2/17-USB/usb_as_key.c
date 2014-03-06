#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

struct input_dev *input_dev;

static char *usb_buf;

static unsigned long len;
static struct urb *uk_urb;
static 	dma_addr_t dma;

static void usbmouse_as_key_irq(struct urb *urb)
{
	static unsigned char pre_val;

#if 1
	int i;
	static int cnt = 0;
	printk("data cnt %d: ", ++cnt);
	for (i = 0; i < len; i++)
	{
		printk("%02x ", usb_buf[i]);
	}
	printk("\n");
#endif
	/* USB鼠标数据含义
	 * data[0]: bit0-左键, 1-按下, 0-松开
	 *          bit1-右键, 1-按下, 0-松开
	 *          bit2-中键, 1-按下, 0-松开 
	 *
     */
	if ((pre_val & (1<<0)) != (usb_buf[0] & (1<<0)))
	{
		/* 左键发生了变化 */
		input_event(input_dev, EV_KEY, KEY_L, (usb_buf[0] & (1<<0)) ? 1 : 0);
		input_sync(input_dev);
	}

	if ((pre_val & (1<<1)) != (usb_buf[0] & (1<<1)))
	{
		/* 右键发生了变化 */
		input_event(input_dev, EV_KEY, KEY_S, (usb_buf[0] & (1<<1)) ? 1 : 0);
		input_sync(input_dev);
	}

	if ((pre_val & (1<<2)) != (usb_buf[0] & (1<<2)))
	{
		/* 中键发生了变化 */
		input_event(input_dev, EV_KEY, KEY_ENTER, (usb_buf[0] & (1<<2)) ? 1 : 0);
		input_sync(input_dev);
	}
	
	pre_val = usb_buf[0];

	/* 重新提交urb */
	usb_submit_urb(uk_urb, GFP_KERNEL);
}


static int mouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	int pipe;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;



	
	//struct usb_host_interface *interface;
	struct usb_device *udev ;
	udev = interface_to_usbdev(intf);
	interface = intf->cur_altsetting;

	printk(KERN_INFO "usb mouse Probe\n");
	
	//printk(KERN_DEBUG "\nspeed=%d,devnum=%d,product=%s\n",udev->speed,udev->devnum,udev->product);
	//printk(KERN_DEBUG "\niManufactureer=%d,iProduct=%d,type=%d\n",udev->descriptor.iManufacturer,udev->descriptor.iProduct,udev->descriptor.bDescriptorType);
	printk("bcdUSB = %x\n", udev->descriptor.bcdUSB);
	printk("VID    = 0x%x\n", udev->descriptor.idVendor);
	printk("PID    = 0x%x\n", udev->descriptor.idProduct);

	input_dev = input_allocate_device();
	printk(KERN_INFO "input dev alloc\n");
	set_bit(EV_ABS,input_dev->evbit);
	set_bit(EV_REP,input_dev->evbit);

	set_bit(KEY_L,input_dev->evbit);
	set_bit(KEY_S,input_dev->evbit);	
	set_bit(KEY_ENTER,input_dev->evbit);	

	input_register_device(input_dev);
	printk(KERN_INFO "input dev reg\n");

	/*3: source destination Len */
	//1  Source
	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	//2 Destination
	len = endpoint->wMaxPacketSize;
	usb_buf = usb_buffer_alloc(dev, len, GFP_ATOMIC,&dma);
	if(NULL == usb_buf){
		printk(KERN_INFO "usb buffer alloc failed!!!!\n");
	}
	printk(KERN_INFO "usb buffer alloced\n");

	/* 使用"3要素设置urb" */
	printk(KERN_ERR "dev=%p,pipe=%p,usb_buf=%p,len=%d,usb_irq=%p,endpoint->bInterval=%d",dev,pipe,usb_buf,len,
	usbmouse_as_key_irq,endpoint->bInterval);
	uk_urb = usb_alloc_urb(0, GFP_KERNEL);
	usb_fill_int_urb(uk_urb, dev, pipe, usb_buf, len, usbmouse_as_key_irq, NULL, endpoint->bInterval);
	printk(KERN_INFO "fill urb\n");
	uk_urb->transfer_dma = dma;
	uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* 使用URB */
	usb_submit_urb(uk_urb, GFP_KERNEL);
	printk(KERN_INFO "submit urb\n");
	return 0;
}

static void mouse_as_key_disconnect(struct usb_interface *intf)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	printk(KERN_INFO "usb mouse disconnect\n");

	//printk("disconnect usbmouse!\n");
	usb_kill_urb(uk_urb);
	usb_free_urb(uk_urb);

	usb_buffer_free(dev, len, usb_buf, dma);
	input_unregister_device(input_dev);
	input_free_device(input_dev);
}

static struct usb_device_id mouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* Terminating entry */
};


static struct usb_driver mouse_as_key_driver = {
	.name		= "mouse_as_key",
	.probe		= mouse_as_key_probe,
	.disconnect	= mouse_as_key_disconnect,
	.id_table	= mouse_as_key_id_table,
};


static int mouse_as_key_init(void){
	usb_register(&mouse_as_key_driver);
	return 0;
}

static void mouse_as_key_exit(void)
{
	usb_deregister(&mouse_as_key_driver);
}

module_init(mouse_as_key_init);
module_exit(mouse_as_key_exit);


MODULE_LICENSE("GPL");


