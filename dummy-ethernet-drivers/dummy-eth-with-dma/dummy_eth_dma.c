// dummy_eth_dma.c
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/skbuff.h>

#define DUMMY_RING_SIZE 64

/* -------- Descriptor -------- */
struct dummy_desc {
    struct sk_buff *skb;
    unsigned int len;
    int used;
};

/* -------- Private Data -------- */
struct dummy_priv {
    struct net_device *ndev;

    struct dummy_desc tx_ring[DUMMY_RING_SIZE];
    struct dummy_desc rx_ring[DUMMY_RING_SIZE];

    int tx_head, tx_tail;
    int rx_head, rx_tail;
};

/* ---------------- TX Path ---------------- */
static netdev_tx_t dummy_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    struct dummy_priv *priv = netdev_priv(ndev);
    struct dummy_desc *desc;
    int next;

    next = (priv->tx_head + 1) % DUMMY_RING_SIZE;

    if (next == priv->tx_tail) {
        netif_stop_queue(ndev);
        return NETDEV_TX_BUSY;
    }

    desc = &priv->tx_ring[priv->tx_head];
    desc->skb = skb;
    desc->len = skb->len;
    desc->used = 1;

    priv->tx_head = next;

    /* Simulate immediate TX completion */
    dev_kfree_skb(skb);
    ndev->stats.tx_packets++;
    ndev->stats.tx_bytes += desc->len;

    /* Free descriptor */
    desc->used = 0;
    priv->tx_tail = (priv->tx_tail + 1) % DUMMY_RING_SIZE;

    return NETDEV_TX_OK;
}

/* ---------------- RX Simulation ---------------- */
static void dummy_rx_simulate(struct net_device *ndev)
{
    struct dummy_priv *priv = netdev_priv(ndev);
    struct sk_buff *skb;
    int len = 64;

    skb = netdev_alloc_skb(ndev, len);
    if (!skb)
        return;

    skb_put(skb, len);
    skb->protocol = eth_type_trans(skb, ndev);

    netif_rx(skb);

    ndev->stats.rx_packets++;
    ndev->stats.rx_bytes += len;
}

/* ---------------- Open/Stop ---------------- */
static int dummy_open(struct net_device *ndev)
{
    netif_start_queue(ndev);
    return 0;
}

static int dummy_stop(struct net_device *ndev)
{
    netif_stop_queue(ndev);
    return 0;
}

/* ---------------- Netdev Ops ---------------- */
static const struct net_device_ops dummy_netdev_ops = {
    .ndo_open       = dummy_open,
    .ndo_stop       = dummy_stop,
    .ndo_start_xmit = dummy_xmit,
};

/* ---------------- Probe ---------------- */
static int dummy_probe(struct platform_device *pdev)
{
    struct net_device *ndev;
    struct dummy_priv *priv;
    int ret, i;

    ndev = alloc_etherdev(sizeof(struct dummy_priv));
    if (!ndev)
        return -ENOMEM;

    priv = netdev_priv(ndev);
    priv->ndev = ndev;

    /* Init rings */
    priv->tx_head = priv->tx_tail = 0;
    priv->rx_head = priv->rx_tail = 0;

    for (i = 0; i < DUMMY_RING_SIZE; i++) {
        priv->tx_ring[i].used = 0;
        priv->rx_ring[i].used = 0;
    }

    SET_NETDEV_DEV(ndev, &pdev->dev);
    ndev->netdev_ops = &dummy_netdev_ops;
    eth_hw_addr_random(ndev);

    ret = register_netdev(ndev);
    if (ret) {
        free_netdev(ndev);
        return ret;
    }

    platform_set_drvdata(pdev, ndev);

    pr_info("dummy_eth_dma: registered %s\n", ndev->name);
    return 0;
}

/* ---------------- Remove ---------------- */
static int dummy_remove(struct platform_device *pdev)
{
    struct net_device *ndev = platform_get_drvdata(pdev);

    unregister_netdev(ndev);
    free_netdev(ndev);

    pr_info("dummy_eth_dma: removed\n");
    return 0;
}

/* ---------------- Platform Driver ---------------- */
static struct platform_driver dummy_driver = {
    .probe  = dummy_probe,
    .remove = dummy_remove,
    .driver = {
        .name = "dummy_eth_dma",
    },
};

/* ---------------- Module Init/Exit ---------------- */
static int __init dummy_init(void)
{
    int ret;

    ret = platform_driver_register(&dummy_driver);
    if (ret)
        return ret;

    platform_device_register_simple("dummy_eth_dma", -1, NULL, 0);

    pr_info("dummy_eth_dma: module loaded\n");
    return 0;
}

static void __exit dummy_exit(void)
{
    platform_driver_unregister(&dummy_driver);
    pr_info("dummy_eth_dma: module unloaded\n");
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenAI");
MODULE_DESCRIPTION("Dummy Ethernet Driver with TX/RX Ring Simulation");
