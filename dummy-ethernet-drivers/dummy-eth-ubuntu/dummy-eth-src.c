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
    /*
     * ether_setup(dev)
     *
     * Initializes a network device as a standard Ethernet device by
     * setting default Ethernet-specific parameters and behaviors.
     *
     * Key initializations performed:
     *
     * 1. Device type and header configuration:
     *    - dev->type = ARPHRD_ETHER          (marks device as Ethernet)
     *    - dev->hard_header_len = ETH_HLEN   (Ethernet header length)
     *    - dev->addr_len = ETH_ALEN          (MAC address length = 6 bytes)
     *
     * 2. Device flags:
     *    - Enables broadcast and multicast support
     *      (IFF_BROADCAST | IFF_MULTICAST)
     *
     * 3. MTU (Maximum Transmission Unit):
     *    - dev->mtu = ETH_DATA_LEN           (~1500 bytes)
     *
     * 4. Header operations:
     *    - Assigns default Ethernet header handling functions
     *      via dev->header_ops = &eth_header_ops
     *
     * 5. Device features:
     *    - Sets baseline Ethernet capabilities (kernel-version dependent)
     *
     * Notes:
     *    - This provides a default Ethernet configuration.
     *    - Driver-specific fields (e.g., netdev_ops) should be set
     *      after calling ether_setup().
     *    - Typically used inside the device setup function passed
     *      to alloc_netdev().
     */
    dev->netdev_ops = &dummy_ops;
    /*
        1. dev->netdev_ops = &dummy_ops;
            This overwrites whatever was previously set in the ether_setup().
            After ether_setup(dev), netdev_ops is typically NULL (not fully assigned)
            When we do:
            Cdev->netdev_ops = &dummy_ops;
            you're assigning your driver's operations
    */
    dev->flags |= IFF_NOARP;
    /*
        This is not overwriting, it's adding a flag.
        ether_setup() already sets: dev->flags = IFF_BROADCAST | IFF_MULTICAST;
        ev->flags |= IFF_NOARP means keep existing flags + add IFF_NOARP
        So final flags become: IFF_BROADCAST | IFF_MULTICAST | IFF_NOARP
    */
}

/* Module init */
static int __init dummy_init(void)
{
    dummy_dev = alloc_netdev(0, "dummy%d", NET_NAME_UNKNOWN, dummy_setup);
    /*
        alloc_netdev()
           └── alloc memory
           └── call dummy_setup(dev)   <-- happens here
           └── return dev
        register_netdev(dev)           <-- happens later
    */
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
