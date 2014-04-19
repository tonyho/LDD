//Ref the cs89x0.c
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

struct net_device *dev;

static int virnet_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	static int cnt = 0;
	dev->stats.tx_bytes += skb->len;
	dev->stats.tx_packets ++;
	//printk("virt_net_send_packet cnt = %d\n", ++cnt);
	return 0;
}


static int virnet_init(void){
	int ret = -1;
	dev = alloc_netdev_mq(NULL, "virnet%d", ether_setup, 1);

	//Set MAC address
	dev->dev_addr[0] = 0x48;
	dev->dev_addr[1] = 0x32;
	dev->dev_addr[2] = 0xee;
	dev->dev_addr[3] = 0x80;
	dev->dev_addr[4] = 0xde;
	dev->dev_addr[5] = 0x94;	

	dev->hard_start_xmit = virnet_send_packet;

	ret = register_netdev(dev);
	if(ret<0){
		printk(KERN_ERR "register netdev failed!!!\n");
	}
	
	
	return 0;
}

static void virnet_exit(void){
	unregister_netdev(dev);
	free_netdev(dev);
}

module_init(virnet_init);
module_exit(virnet_exit);

MODULE_LICENSE("GPL");


