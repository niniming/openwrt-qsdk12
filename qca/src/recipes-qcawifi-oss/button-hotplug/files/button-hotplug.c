/*
 *  Button Hotplug driver
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 *  Based on the diag.c - GPIO interface driver for Broadcom boards
 *    Copyright (C) 2006 Mike Baker <mbm@openwrt.org>,
 *    Copyright (C) 2006-2007 Felix Fietkau <nbd@openwrt.org>
 *    Copyright (C) 2008 Andy Boyett <agb@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kmod.h>
#include <linux/input.h>

#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/kobject.h>

#define DRV_NAME	"button-hotplug"
#define DRV_VERSION	"0.4.1"
#define DRV_DESC	"Button Hotplug driver"

#define BH_SKB_SIZE	2048

#define PFX	DRV_NAME ": "

#undef BH_DEBUG

#ifdef BH_DEBUG
#define BH_DBG(fmt, args...) printk(KERN_DEBUG "%s: " fmt, DRV_NAME, ##args )
#else
#define BH_DBG(fmt, args...) do {} while (0)
#endif

#define BH_ERR(fmt, args...) printk(KERN_ERR "%s: " fmt, DRV_NAME, ##args )

#ifndef BIT_MASK
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#endif

struct bh_priv {
	unsigned long		*seen;
	struct input_handle	handle;
	struct kobject      bh_kobj;
};

struct bh_event {
	const char		*name;
	char			*action;
	unsigned long		seen;
	struct bh_priv		*priv;
	struct work_struct	work;
};

static struct kset *bh_kset;

static struct attribute *bh_attrs[] = {
   NULL,
};

static struct sysfs_ops bh_attr_ops = {
};

static struct kobj_type bh_ktype = {
   .default_attrs = bh_attrs,
   .sysfs_ops     = &bh_attr_ops,
};

struct bh_map {
	unsigned int	code;
	const char	*name;
};


#define BH_MAP(_code, _name)		\
	{				\
		.code = (_code),	\
		.name = (_name),	\
	}

static struct bh_map button_map[] = {
	BH_MAP(BTN_0,		"BTN_0"),
	BH_MAP(BTN_1,		"BTN_1"),
	BH_MAP(BTN_2,		"BTN_2"),
	BH_MAP(BTN_3,		"BTN_3"),
	BH_MAP(BTN_4,		"BTN_4"),
	BH_MAP(BTN_5,		"BTN_5"),
	BH_MAP(BTN_6,		"BTN_6"),
	BH_MAP(BTN_7,		"BTN_7"),
	BH_MAP(BTN_8,		"BTN_8"),
	BH_MAP(BTN_9,		"BTN_9"),
	BH_MAP(KEY_RESTART,	"reset"),
	BH_MAP(KEY_POWER,	"power"),
	BH_MAP(KEY_RFKILL,	"rfkill"),
	BH_MAP(KEY_WPS_BUTTON,	"wps"),
	BH_MAP(KEY_WIMAX,	"wwan"),
};

static void button_hotplug_work(struct work_struct *work)
{
	struct bh_event *event = container_of(work, struct bh_event, work);
	struct bh_priv   *priv = event->priv;
	char env_baction[32];
	char env_button[32];
	char env_seen[32];
	char *envp[] = { env_baction, env_button, env_seen, NULL };

	sprintf(env_baction, "ACTION=%s", event->action);
	sprintf(env_button, "BUTTON=%s", event->name);
	sprintf(env_seen, "SEEN=%ld", event->seen);
	kobject_uevent_env(&priv->bh_kobj, KOBJ_CHANGE, envp);

	kfree(event);
}

static int button_hotplug_create_event(const char *name, unsigned long seen,
		int pressed, struct bh_priv *thispriv)
{
	struct bh_event *event;

	BH_DBG("create event, name=%s, seen=%lu, pressed=%d\n",
		name, seen, pressed);

	event = kzalloc(sizeof(*event), GFP_KERNEL);
	if (!event)
		return -ENOMEM;

	event->name = name;
	event->seen = seen;
	event->action = pressed ? "pressed" : "released";
	event->priv = thispriv;

	INIT_WORK(&event->work, (void *)(void *)button_hotplug_work);
	schedule_work(&event->work);

	return 0;
}

/* -------------------------------------------------------------------------*/

static int button_get_index(unsigned int code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(button_map); i++)
		if (button_map[i].code == code)
			return i;

	return -1;
}
static void button_hotplug_event(struct input_handle *handle,
			   unsigned int type, unsigned int code, int value)
{
	struct bh_priv *priv = handle->private;
	unsigned long seen = jiffies;
	int btn;

	BH_DBG("event type=%u, code=%u, value=%d\n", type, code, value);

	if (type != EV_KEY)
		return;

	btn = button_get_index(code);
	if (btn < 0)
		return;

	button_hotplug_create_event(button_map[btn].name,
			(seen - priv->seen[btn]) / HZ, value,priv);
	priv->seen[btn] = seen;
}

static int button_hotplug_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id)
{
	struct bh_priv *priv;
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(button_map); i++)
		if (test_bit(button_map[i].code, dev->keybit))
			break;

	if (i == ARRAY_SIZE(button_map))
		return -ENODEV;

	priv = kzalloc(sizeof(*priv) +
		       (sizeof(unsigned long) * ARRAY_SIZE(button_map)),
		       GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->seen = (unsigned long *) &priv[1];
	priv->handle.private = priv;
	priv->handle.dev = dev;
	priv->handle.handler = handler;
	priv->handle.name = DRV_NAME;


	priv->bh_kobj.kset = bh_kset;
	ret = kobject_init_and_add(&priv->bh_kobj, &bh_ktype, NULL, "bh-%s", dev->name);
	if (ret)
		goto err_free_priv;
	ret = input_register_handle(&priv->handle);
	if (ret)
		goto err_free_priv;

	ret = input_open_device(&priv->handle);
	if (ret)
		goto err_unregister_handle;

	BH_DBG("connected to %s\n", dev->name);

	return 0;

 err_unregister_handle:
	input_unregister_handle(&priv->handle);

 err_free_priv:
	kfree(priv);
	return ret;
}

static void button_hotplug_disconnect(struct input_handle *handle)
{
	struct bh_priv *priv = handle->private;

	input_close_device(handle);
	input_unregister_handle(handle);

	kobject_put(&priv->bh_kobj);
	kfree(priv);
}

static const struct input_device_id button_hotplug_ids[] = {
	{
                .flags = INPUT_DEVICE_ID_MATCH_EVBIT,
                .evbit = { BIT_MASK(EV_KEY) },
        },
	{
		/* Terminating entry */
	},
};

MODULE_DEVICE_TABLE(input, button_hotplug_ids);

static struct input_handler button_hotplug_handler = {
	.event =	button_hotplug_event,
	.connect =	button_hotplug_connect,
	.disconnect =	button_hotplug_disconnect,
	.name =		DRV_NAME,
	.id_table =	button_hotplug_ids,
};

/* -------------------------------------------------------------------------*/

static int __init button_hotplug_init(void)
{
	int ret;

	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");
	bh_kset = kset_create_and_add("button", NULL, kernel_kobj);
	if (!bh_kset) {
		BH_ERR("unable to create kset\n");
		return -ENOMEM;
	}
	ret = input_register_handler(&button_hotplug_handler);
	if (ret)
		BH_ERR("unable to register input handler\n");

	return ret;
}
module_init(button_hotplug_init);

static void __exit button_hotplug_exit(void)
{
	input_unregister_handler(&button_hotplug_handler);
	kset_unregister(bh_kset);
}
module_exit(button_hotplug_exit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");

