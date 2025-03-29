// SPDX-License-Identifier: GPL-2.0+
/*
 * hid.c -- HID Composite driver
 *
 * Based on multi.c
 *
 * Copyright (C) 2010 Fabien Chouteau <fabien.chouteau@barco.com>
 */


#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/usb/composite.h>
#include <linux/usb/g_hid.h>

#define DRIVER_DESC		"HID Gadget"
#define DRIVER_VERSION		"2010/03/16"

#include "u_hid.h"

#ifdef CONFIG_ARCH_SSTAR
static int intf_num;
static char *hid_mode = "key";
module_param(hid_mode, charp, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(hid_mode, "can choose one or more of the follwing interfaces: key, mouse, bidi_com, touch");


/****************************** User define ******************************/

static struct hidg_func_descriptor hidg_composite_pdata[4] = {0};

/* key */
static struct hidg_func_descriptor hidg_key_pdata = {
	.subclass		= 0,
	.protocol		= 1,
	.report_length		= 8,
	.report_desc_length	= 63,
	.report_desc		= {
		0x05, 0x01,	/* USAGE_PAGE (Generic Desktop)	          */
		0x09, 0x06,	/* USAGE (Keyboard)                       */
		0xa1, 0x01,	/* COLLECTION (Application)               */
		0x05, 0x07,	/*   USAGE_PAGE (Keyboard)                */
		0x19, 0xe0,	/*   USAGE_MINIMUM (Keyboard LeftControl) */
		0x29, 0xe7,	/*   USAGE_MAXIMUM (Keyboard Right GUI)   */
		0x15, 0x00,	/*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x01,	/*   LOGICAL_MAXIMUM (1)                  */
		0x75, 0x01,	/*   REPORT_SIZE (1)                      */
		0x95, 0x08,	/*   REPORT_COUNT (8)                     */
		0x81, 0x02,	/*   INPUT (Data,Var,Abs)                 */
		0x95, 0x01,	/*   REPORT_COUNT (1)                     */
		0x75, 0x08,	/*   REPORT_SIZE (8)                      */
		0x81, 0x03,	/*   INPUT (Cnst,Var,Abs)                 */
		0x95, 0x05,	/*   REPORT_COUNT (5)                     */
		0x75, 0x01,	/*   REPORT_SIZE (1)                      */
		0x05, 0x08,	/*   USAGE_PAGE (LEDs)                    */
		0x19, 0x01,	/*   USAGE_MINIMUM (Num Lock)             */
		0x29, 0x05,	/*   USAGE_MAXIMUM (Kana)                 */
		0x91, 0x02,	/*   OUTPUT (Data,Var,Abs)                */
		0x95, 0x01,	/*   REPORT_COUNT (1)                     */
		0x75, 0x03,	/*   REPORT_SIZE (3)                      */
		0x91, 0x03,	/*   OUTPUT (Cnst,Var,Abs)                */
		0x95, 0x06,	/*   REPORT_COUNT (6)                     */
		0x75, 0x08,	/*   REPORT_SIZE (8)                      */
		0x15, 0x00,	/*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x65,	/*   LOGICAL_MAXIMUM (101)                */
		0x05, 0x07,	/*   USAGE_PAGE (Keyboard)                */
		0x19, 0x00,	/*   USAGE_MINIMUM (Reserved)             */
		0x29, 0x65,	/*   USAGE_MAXIMUM (Keyboard Application) */
		0x81, 0x00,	/*   INPUT (Data,Ary,Abs)                 */
		0xc0		/* END_COLLECTION                         */
	}
};

/* mouse */
static struct hidg_func_descriptor hidg_mouse_pdata = {
	.subclass		= 0,
	.protocol		= 2,
	.report_length		= 4,
	.report_desc_length	= 46,
	.report_desc		= {
		0x05,0x01,	/* Usage Page (Generic Desktop Controls)  */
		0x09,0x02,	/* Usage (Mouse)                          */
		0xa1,0x01,	/* Collction (Application)                */
		0x09,0x01,	/* Usage (pointer)                        */
		0xa1,0x00,	/* Collction (Physical)                   */
		0x05,0x09,	/* Usage Page (Button)                    */
		0x19,0x01,	/* Usage Minimum(1)                       */
		0x29,0x03,	/* Usage Maximum(3)                       */
		0x15,0x00,	/* Logical Minimum(1)                     */
		0x25,0x01,	/* Logical Maximum(1)                     */
		0x95,0x08,	/* Report Count(5)                        */
		0x75,0x01,	/* Report Size(1)                         */
		0x81,0x02,	/* Input(Data,Variable,Absolute,BitFiled) */
		0x05,0x01,	/* Usage Page (Generic Desktop Controls)  */
		0x09,0x30,	/* Usage(x)                               */
		0x09,0x31,	/* Usage(y)                               */
		0x09,0x38,	/* Usage(Wheel)                           */
		0x15,0x81,	/* Logical Minimum(-127)                  */
		0x25,0x7f,	/* Logical Maximum(127)                   */
		0x75,0x08,	/* Report Size(8)                         */
		0x95,0x03,	/* Report Count(2)                        */
		0x81,0x06,	/* Input(Data,Variable,Relative,BitFiled) */
		0xc0,		/* End Collection                         */
		0xc0		/* End Collection                         */
	}
};

/* bidi com */
static struct hidg_func_descriptor hidg_bidi_com_pdata = {
	.subclass		= 0,
	.protocol		= 0,
	.report_length		= 64,
	.report_desc_length	= 34,
	.report_desc		= {
		0x06,0x00,0xFF,		/* Usage Page (Vendor Defined Page 1) */
		0x09,0x01,			/* Usage (Vendor Usage 1)             */
		0xa1,0x01,			/* Collction (Application)            */
		0x09,0x02,			/* Usage (Vendor Usage 2)             */
		0x15,0x00,			/* LOGICAL_MINIMUM(0)                 */
		0x26,0xFF,0x00,		/* LOGICAL_MINIMUM(255)               */
		0x75,0x08,			/* Report Size(8)                     */
		0x95,0x40,			/* Report Count(64)                   */
		0x81,0x02,			/* Input(Data,Variable,Absolute)      */
		0x09,0x02,			/* Usage (Vendor Usage 2)             */
		0x15,0x00,			/* LOGICAL_MINIMUM(0)                 */
		0x26,0xFF,0x00,		/* LOGICAL_MINIMUM(255)               */
		0x75,0x08,			/* Report Size(8)                     */
		0x95,0x40,			/* Report Count(64)                   */
		0x91,0x02,			/* Output(Data,Variable,Absolute)     */
		0xc0				/* End Collection                     */
	}
};

/* touch */
static struct hidg_func_descriptor hidg_touch_pdata = {
	.subclass		= 0,
	.protocol		= 0,
	.report_length		= 64,
	.report_desc_length	= 87,
	.report_desc		= {
		0x05, 0x0d,			/* USAGE_PAGE (Digitizers)           0*/
		0x09, 0x04,			/* USAGE (Touch Screen)              2*/
		0xa1, 0x01,			/* COLLECTION (Application)          4*/
		0x85, 0xaa,			/*   REPORT_ID (Touch)               6*/
		0x09, 0x20,			/*   USAGE (Stylus)                  8*/
		0xa1, 0x00,			/*   COLLECTION (Physical)          10*/
		0x09, 0x42,			/*     USAGE (Tip Switch)           12*/
		0x15, 0x00,			/*     LOGICAL_MINIMUM (0)          14*/
		0x25, 0x01,			/*     LOGICAL_MAXIMUM (1)          16*/
		0x75, 0x01,			/*     REPORT_SIZE (1)              18*/
		0x95, 0x01,			/*     REPORT_COUNT (1)             20*/
		0x81, 0x02,			/*     INPUT (Data,Var,Abs)         22*/
		0x95, 0x03,			/*     REPORT_COUNT (3)             24*/
		0x81, 0x03,			/*     INPUT (Cnst,Ary,Abs)         26*/
		0x09, 0x32,			/*     USAGE (In Range)             28*/
		0x09, 0x37,			/*     USAGE (Data Valid-Finger)    30*/
		0x95, 0x02,			/*     REPORT_COUNT (2)             32*/
		0x81, 0x02,			/*     INPUT (Data,Var,Abs)         34*/
		0x95, 0x0a,			/*     REPORT_COUNT (10)            36*/
		0x81, 0x03,			/*     INPUT (Cnst,Ary,Abs)         38*/
		0x05, 0x01,			/*     USAGE_PAGE (Generic Desktop) 40*/
		0x26, 0xff, 0x7f,	/*     LOGICAL_MAXIMUM (32767)      42*/
		0x75, 0x10,			/*     REPORT_SIZE (16)             45*/
		0x95, 0x01,			/*     REPORT_COUNT (1)             47*/
		0xa4,				/*     PUSH                         49*/
		0x55, 0x0d,			/*     UNIT_EXPONENT (-3)           50*/
		0x65, 0x00,			/*     UNIT (None)                  52*/
		0x09, 0x30,			/*     USAGE (X)                    54*/
		0x35, 0x00,			/*     PHYSICAL_MINIMUM (0)         56*/
		0x46, 0x00, 0x00,	/*     PHYSICAL_MAXIMUM (0)         58*/
		0x81, 0x02,			/*     INPUT (Data,Var,Abs)         61*/
		0x09, 0x31,			/*     USAGE (Y)                    63*/
		0x46, 0x00, 0x00,	/*     PHYSICAL_MAXIMUM (0)         65*/
		0x81, 0x02,			/*     INPUT (Data,Var,Abs)         68*/
		0xb4,				/*     POP                          70*/
		0x05, 0x0d,			/*     USAGE PAGE (Digitizers)      71*/
		0x09, 0x60,			/*     USAGE (Width)                73*/
		0x09, 0x61,			/*     USAGE (Height)               75*/
		0x95, 0x02,			/*     REPORT_COUNT (2)             77*/
		0x81, 0x02,			/*     INPUT (Data,Var,Abs)         79*/
		0x95, 0x01,			/*     REPORT_COUNT (1)             81*/
		0x81, 0x03,			/*     INPUT (Cnst,Ary,Abs)         83*/
		0xc0,				/*   END_COLLECTION                 85*/
		0xc0				/* END_COLLECTION                   86*/
	}
};

#endif

/*-------------------------------------------------------------------------*/

#define HIDG_VENDOR_NUM		0x0525	/* XXX NetChip */
#define HIDG_PRODUCT_NUM	0xa4ac	/* Linux-USB HID gadget */

/*-------------------------------------------------------------------------*/

struct hidg_func_node {
	struct usb_function_instance *fi;
	struct usb_function *f;
	struct list_head node;
	struct hidg_func_descriptor *func;
};

static LIST_HEAD(hidg_func_list);

/*-------------------------------------------------------------------------*/
USB_GADGET_COMPOSITE_OPTIONS();

static struct usb_device_descriptor device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,

	/* .bcdUSB = DYNAMIC */

	/* .bDeviceClass =		USB_CLASS_COMM, */
	/* .bDeviceSubClass =	0, */
	/* .bDeviceProtocol =	0, */
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,
	/* .bMaxPacketSize0 = f(hardware) */

	/* Vendor and product id can be overridden by module parameters.  */
	.idVendor =		cpu_to_le16(HIDG_VENDOR_NUM),
	.idProduct =		cpu_to_le16(HIDG_PRODUCT_NUM),
	/* .bcdDevice = f(hardware) */
	/* .iManufacturer = DYNAMIC */
	/* .iProduct = DYNAMIC */
	/* NO SERIAL NUMBER */
	.bNumConfigurations =	1,
};

static const struct usb_descriptor_header *otg_desc[2];

/* string IDs are assigned dynamically */
static struct usb_string strings_dev[] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "",
	[USB_GADGET_PRODUCT_IDX].s = DRIVER_DESC,
	[USB_GADGET_SERIAL_IDX].s = "",
	{  } /* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};



/****************************** Configurations ******************************/

static int do_config(struct usb_configuration *c)
{
	struct hidg_func_node *e, *n;
	int status = 0;

	if (gadget_is_otg(c->cdev->gadget)) {
		c->descriptors = otg_desc;
		c->bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	list_for_each_entry(e, &hidg_func_list, node) {
		e->f = usb_get_function(e->fi);
		if (IS_ERR(e->f)) {
			status = PTR_ERR(e->f);
			goto put;
		}
		status = usb_add_function(c, e->f);
		if (status < 0) {
			usb_put_function(e->f);
			goto put;
		}
	}

	return 0;
put:
	list_for_each_entry(n, &hidg_func_list, node) {
		if (n == e)
			break;
		usb_remove_function(c, n->f);
		usb_put_function(n->f);
	}
	return status;
}

static struct usb_configuration config_driver = {
	.label			= "HID Gadget",
	.bConfigurationValue	= 1,
	/* .iConfiguration = DYNAMIC */
	.bmAttributes		= USB_CONFIG_ATT_SELFPOWER,
};

/****************************** Gadget Bind ******************************/

static int hid_bind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	struct list_head *tmp;
	struct hidg_func_node *n, *m;
	struct f_hid_opts *hid_opts;
	int status, funcs = 0;

	list_for_each(tmp, &hidg_func_list)
		funcs++;

	if (!funcs)
		return -ENODEV;

	list_for_each_entry(n, &hidg_func_list, node) {
		n->fi = usb_get_function_instance("hid");
		if (IS_ERR(n->fi)) {
			status = PTR_ERR(n->fi);
			goto put;
		}
		hid_opts = container_of(n->fi, struct f_hid_opts, func_inst);
		hid_opts->subclass = n->func->subclass;
		hid_opts->protocol = n->func->protocol;
		hid_opts->report_length = n->func->report_length;
		hid_opts->report_desc_length = n->func->report_desc_length;
		hid_opts->report_desc = n->func->report_desc;
	}


	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */

	status = usb_string_ids_tab(cdev, strings_dev);
	if (status < 0)
		goto put;
	device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;

	if (gadget_is_otg(gadget) && !otg_desc[0]) {
		struct usb_descriptor_header *usb_desc;

		usb_desc = usb_otg_descriptor_alloc(gadget);
		if (!usb_desc) {
			status = -ENOMEM;
			goto put;
		}
		usb_otg_descriptor_init(gadget, usb_desc);
		otg_desc[0] = usb_desc;
		otg_desc[1] = NULL;
	}

	/* register our configuration */
	status = usb_add_config(cdev, &config_driver, do_config);
	if (status < 0)
		goto free_otg_desc;

	usb_composite_overwrite_options(cdev, &coverwrite);
	dev_info(&gadget->dev, DRIVER_DESC ", version: " DRIVER_VERSION "\n");

	return 0;

free_otg_desc:
	kfree(otg_desc[0]);
	otg_desc[0] = NULL;
put:
	list_for_each_entry(m, &hidg_func_list, node) {
		if (m == n)
			break;
		usb_put_function_instance(m->fi);
	}
	return status;
}

static int hid_unbind(struct usb_composite_dev *cdev)
{
	struct hidg_func_node *n;

	list_for_each_entry(n, &hidg_func_list, node) {
		usb_put_function(n->f);
		usb_put_function_instance(n->fi);
	}

	kfree(otg_desc[0]);
	otg_desc[0] = NULL;

	return 0;
}

static int hidg_plat_driver_probe(struct platform_device *pdev)
{
	struct hidg_func_descriptor *func = dev_get_platdata(&pdev->dev);
	struct hidg_func_node *entry;
#ifdef CONFIG_ARCH_SSTAR
	int i;
#endif
	if (!func) {
		dev_err(&pdev->dev, "Platform data missing\n");
		return -ENODEV;
	}

#ifdef CONFIG_ARCH_SSTAR
	for (i = 0; i < intf_num; i++) {
		entry = kzalloc(sizeof(*entry), GFP_KERNEL);
		if (!entry)
			return -ENOMEM;
		entry->func = &func[i];
		list_add_tail(&entry->node, &hidg_func_list);
	}
#else
	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	entry->func = func;
	list_add_tail(&entry->node, &hidg_func_list);
#endif

	return 0;
}

static int hidg_plat_driver_remove(struct platform_device *pdev)
{
	struct hidg_func_node *e, *n;

	list_for_each_entry_safe(e, n, &hidg_func_list, node) {
		list_del(&e->node);
		kfree(e);
	}

	return 0;
}


/****************************** Some noise ******************************/


static struct usb_composite_driver hidg_driver = {
	.name		= "g_hid",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.max_speed	= USB_SPEED_HIGH,
	.bind		= hid_bind,
	.unbind		= hid_unbind,
};

static struct platform_driver hidg_plat_driver = {
	.remove		= hidg_plat_driver_remove,
	.driver		= {
		.name	= "hidg",
	},
};

#ifdef CONFIG_ARCH_SSTAR

static void hidg_device_release(struct device *device)
{
	return;
}

struct platform_device hidg_composite_device = {
	.name			= "hidg",
	.id			= 0,
	.num_resources		= 0,
	.resource		= 0,
	.dev.platform_data	= hidg_composite_pdata,
	.dev.release = hidg_device_release,
};
#endif


MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Fabien Chouteau, Peter Korsgaard");
MODULE_LICENSE("GPL");

static int __init hidg_init(void)
{
	int status;

#ifdef CONFIG_ARCH_SSTAR
	intf_num = 0;
	if (strstr(hid_mode, "key")) {
		memcpy(&hidg_composite_pdata[intf_num], &hidg_key_pdata, sizeof(struct hidg_func_descriptor));
		intf_num++;
	}

	if (strstr(hid_mode, "mouse")) {
		memcpy(&hidg_composite_pdata[intf_num], &hidg_mouse_pdata, sizeof(struct hidg_func_descriptor));
		intf_num++;
	}

	if (strstr(hid_mode, "bidi_com")) {
		memcpy(&hidg_composite_pdata[intf_num], &hidg_bidi_com_pdata, sizeof(struct hidg_func_descriptor));
		intf_num++;
	}

	if (strstr(hid_mode, "touch")) {
		memcpy(&hidg_composite_pdata[intf_num], &hidg_touch_pdata, sizeof(struct hidg_func_descriptor));
		intf_num++;
	}

	if (intf_num == 0) {
		return -EINVAL;
	}

	status = platform_device_register(&hidg_composite_device);
	if (status < 0) {
		return status;
	}
#endif

	status = platform_driver_probe(&hidg_plat_driver,
				hidg_plat_driver_probe);
	if (status < 0){
#ifdef CONFIG_ARCH_SSTAR
		platform_device_unregister(&hidg_composite_device);
#endif
		return status;
	}

	status = usb_composite_probe(&hidg_driver);
	if (status < 0){
#ifdef CONFIG_ARCH_SSTAR
		platform_device_unregister(&hidg_composite_device);
#endif
		platform_driver_unregister(&hidg_plat_driver);
	}

	return status;
}
module_init(hidg_init);

static void __exit hidg_cleanup(void)
{
	usb_composite_unregister(&hidg_driver);
	platform_driver_unregister(&hidg_plat_driver);
#ifdef CONFIG_ARCH_SSTAR
	platform_device_unregister(&hidg_composite_device);
#endif
}
module_exit(hidg_cleanup);
