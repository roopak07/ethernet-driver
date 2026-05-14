// dummy_eth.c
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>

struct dummy_priv {
    struct net_device *ndev;
};

/* ---------------- TX Path ---------------- */
static netdev_tx_t dummy_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    unsigned int len = skb->len;

    /* Just drop packet (like dummy driver) */
    dev_kfree_skb(skb);

    /* Update stats */
    ndev->stats.tx_packets++;
    ndev->stats.tx_bytes += len;

    return NETDEV_TX_OK;
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
    int ret;

    ndev = alloc_etherdev(sizeof(struct dummy_priv));
    if (!ndev)
        return -ENOMEM;

    priv = netdev_priv(ndev);
    priv->ndev = ndev;

    SET_NETDEV_DEV(ndev, &pdev->dev);
    ndev->netdev_ops = &dummy_netdev_ops;
    eth_hw_addr_random(ndev);

    ret = register_netdev(ndev);
    if (ret) {
        free_netdev(ndev);
        return ret;
    }

    platform_set_drvdata(pdev, ndev);

    pr_info("dummy_eth: registered %s\n", ndev->name);
    return 0;
}

/* ---------------- Remove ---------------- */
static int dummy_remove(struct platform_device *pdev)
{
    struct net_device *ndev = platform_get_drvdata(pdev);

    unregister_netdev(ndev);
    free_netdev(ndev);

    pr_info("dummy_eth: removed\n");
    return 0;
}

/* ---------------- Platform Driver ---------------- */
static struct platform_driver dummy_driver = {
    .probe  = dummy_probe,
    .remove = dummy_remove,
    .driver = {
        .name = "dummy_eth",
    },
};

/* ---------------- Module Init/Exit ---------------- */
static int __init dummy_init(void)
{
    int ret;

    ret = platform_driver_register(&dummy_driver);
    if (ret)
        return ret;

    platform_device_register_simple("dummy_eth", -1, NULL, 0);

    pr_info("dummy_eth: module loaded\n");
    return 0;
}

static void __exit dummy_exit(void)
{
    platform_driver_unregister(&dummy_driver);
    pr_info("dummy_eth: module unloaded\n");
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenAI");
MODULE_DESCRIPTION("Minimal Dummy Ethernet Driver");
