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

#include <linux/ip.h>


#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

struct net_device *dev;

static void emulator_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
	/* 参考LDD3 */
	unsigned char *type;
	struct iphdr *ih;
	__be32 *saddr, *daddr, tmp;
	unsigned char	tmp_dev_addr[ETH_ALEN];
	struct ethhdr *ethhdr;
	
	struct sk_buff *rx_skb;
		
	// 从硬件读出/保存数据
	/* 对调"源/目的"的mac地址 */
	ethhdr = (struct ethhdr *)skb->data;
	memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
	memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
	memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

	/* 对调"源/目的"的ip地址 */    
	ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;

	tmp = *saddr;
	*saddr = *daddr;
	*daddr = tmp;
	
	//((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
	//((u8 *)daddr)[2] ^= 1;
	type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
	//printk("tx package type = %02x\n", *type);
	// 修改类型, 原来0x8表示ping
	*type = 0; /* 0表示reply */
	
	ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
	
	// 构造一个sk_buff
	rx_skb = dev_alloc_skb(skb->len + 2);
	skb_reserve(rx_skb, 2); /* align IP on 16B boundary */	
	memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);

	/* Write metadata, and then pass to the receive level */
	rx_skb->dev = dev;
	rx_skb->protocol = eth_type_trans(rx_skb, dev);
	rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;
	printk(KERN_ERR "Rx the data\n");
	// 提交sk_buff
	netif_rx(rx_skb);
}


static int virnet_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	//static int cnt = 0;

	netif_stop_queue(dev);
	emulator_rx_packet(skb,dev);
	dev_kfree_skb(skb);
	netif_wake_queue(dev);

	
	dev->stats.tx_bytes += skb->len;
	dev->stats.tx_packets ++;
	//printk("virt_net_send_packet cnt = %d\n", ++cnt);
	return 0;
}


static int virnet_init(void){
	int ret = -1;
	dev = alloc_netdev_mq(0, "virnet%d", ether_setup, 1);

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


