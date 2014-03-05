#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static int mouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk(KERN_INFO "usb mouse Probe\n");
	struct usb_device *udev = interface_to_usbdev(intf);
	//printk(KERN_DEBUG "\nspeed=%d,devnum=%d,product=%s\n",udev->speed,udev->devnum,udev->product);
	//printk(KERN_DEBUG "\niManufactureer=%d,iProduct=%d,type=%d\n",udev->descriptor.iManufacturer,udev->descriptor.iProduct,udev->descriptor.bDescriptorType);
	printk("bcdUSB = %x\n", udev->descriptor.bcdUSB);
	printk("VID    = 0x%x\n", udev->descriptor.idVendor);
	printk("PID    = 0x%x\n", udev->descriptor.idProduct);

	
	return 0;
}

static void mouse_as_key_disconnect(struct usb_interface *intf)
{
	printk(KERN_INFO "usb mouse disconnect\n");
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


