/*
 * Dummy Ethernet Driver
 *
 * Copyright (C) 2026 Roopak
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

static struct net_device *dummy_dev;

/* TX function */
static netdev_tx_t dummy_xmit(struct sk_buff *skb, struct net_device *dev)
{
    printk("Dummy TX packet len=%d\n", skb->len);

    /* Just free the packet */
    dev_kfree_skb(skb);

    return NETDEV_TX_OK;
}

/* Open interface */
static int dummy_open(struct net_device *dev)
{
    printk("Dummy device opened\n");
    netif_start_queue(dev);
    return 0;
}

/* Stop interface */
static int dummy_stop(struct net_device *dev)
{
    printk("Dummy device stopped\n");
    netif_stop_queue(dev);
    return 0;
}

/* Netdevice operations */
static const struct net_device_ops dummy_ops = {
    .ndo_open       = dummy_open,
    .ndo_stop       = dummy_stop,
    .ndo_start_xmit = dummy_xmit,
};

/* Setup function */
static void dummy_setup(struct net_device *dev)
{
    ether_setup(dev);  // Ethernet defaults
    dev->netdev_ops = &dummy_ops;
    dev->flags |= IFF_NOARP;
}

/* Module init */
static int __init dummy_init(void)
{
    dummy_dev = alloc_netdev(0, "dummy%d", NET_NAME_UNKNOWN, dummy_setup);
    if (!dummy_dev)
        return -ENOMEM;

    if (register_netdev(dummy_dev)) {
        free_netdev(dummy_dev);
        return -ENODEV;
    }

    printk("Dummy driver loaded\n");
    return 0;
}

/* Module exit */
static void __exit dummy_exit(void)
{
    unregister_netdev(dummy_dev);
    free_netdev(dummy_dev);
    printk("Dummy driver unloaded\n");
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_LICENSE("GPL");