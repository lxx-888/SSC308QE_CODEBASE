// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Copyright 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 */
#include <command.h>
#include <config.h>
#include <common.h>
#include <env.h>
#include <errno.h>
#include <fastboot.h>
#include <log.h>
#include <malloc.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/compiler.h>
#include <g_dnl.h>

#define FASTBOOT_INTERFACE_CLASS	0xff
#define FASTBOOT_INTERFACE_SUB_CLASS	0x42
#define FASTBOOT_INTERFACE_PROTOCOL	0x03

#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0  (0x0200)
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1  (0x0040)
#define TX_ENDPOINT_MAXIMUM_PACKET_SIZE      (0x0040)

#define EP_BUFFER_SIZE			4096

#ifdef CONFIG_ARCH_SSTAR
struct usb_req {
	struct usb_request *in_req;
	struct usb_req *next;
};
#endif
/*
 * EP_BUFFER_SIZE must always be an integral multiple of maxpacket size
 * (64 or 512 or 1024), else we break on certain controllers like DWC3
 * that expect bulk OUT requests to be divisible by maxpacket size.
 */

struct f_fastboot {
	struct usb_function usb_function;

	/* IN/OUT EP's and corresponding requests */
	struct usb_ep *in_ep, *out_ep;
	struct usb_request *in_req, *out_req;

#ifdef CONFIG_ARCH_SSTAR
	struct usb_req *front, *rear;
#endif
};

static char fb_ext_prop_name[] = "DeviceInterfaceGUID";
static char fb_ext_prop_data[] = "{4866319A-F4D6-4374-93B9-DC2DEB361BA9}";

static struct usb_os_desc_ext_prop fb_ext_prop = {
	.type = 1,		/* NUL-terminated Unicode String (REG_SZ) */
	.name = fb_ext_prop_name,
	.data = fb_ext_prop_data,
};

/* 16 bytes of "Compatible ID" and "Subcompatible ID" */
static char fb_cid[16] = {'W', 'I', 'N', 'U', 'S', 'B'};
static struct usb_os_desc fb_os_desc = {
	.ext_compat_id = fb_cid,
};

static struct usb_os_desc_table fb_os_desc_table = {
	.os_desc = &fb_os_desc,
};

static inline struct f_fastboot *func_to_fastboot(struct usb_function *f)
{
	return container_of(f, struct f_fastboot, usb_function);
}

static struct f_fastboot *fastboot_func;

static struct usb_endpoint_descriptor fs_ep_in = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_IN,
	.bmAttributes       = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize     = cpu_to_le16(64),
};

static struct usb_endpoint_descriptor fs_ep_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(64),
};

static struct usb_endpoint_descriptor hs_ep_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_ep_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(512),
};

static struct usb_interface_descriptor interface_desc = {
	.bLength		= USB_DT_INTERFACE_SIZE,
	.bDescriptorType	= USB_DT_INTERFACE,
	.bInterfaceNumber	= 0x00,
	.bAlternateSetting	= 0x00,
	.bNumEndpoints		= 0x02,
	.bInterfaceClass	= FASTBOOT_INTERFACE_CLASS,
	.bInterfaceSubClass	= FASTBOOT_INTERFACE_SUB_CLASS,
	.bInterfaceProtocol	= FASTBOOT_INTERFACE_PROTOCOL,
};

static struct usb_descriptor_header *fb_fs_function[] = {
	(struct usb_descriptor_header *)&interface_desc,
	(struct usb_descriptor_header *)&fs_ep_in,
	(struct usb_descriptor_header *)&fs_ep_out,
};

static struct usb_descriptor_header *fb_hs_function[] = {
	(struct usb_descriptor_header *)&interface_desc,
	(struct usb_descriptor_header *)&hs_ep_in,
	(struct usb_descriptor_header *)&hs_ep_out,
	NULL,
};

/* Super speed */
static struct usb_endpoint_descriptor ss_ep_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor ss_ep_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor fb_ss_bulk_comp_desc = {
	.bLength =		sizeof(fb_ss_bulk_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *fb_ss_function[] = {
	(struct usb_descriptor_header *)&interface_desc,
	(struct usb_descriptor_header *)&ss_ep_in,
	(struct usb_descriptor_header *)&fb_ss_bulk_comp_desc,
	(struct usb_descriptor_header *)&ss_ep_out,
	(struct usb_descriptor_header *)&fb_ss_bulk_comp_desc,
	NULL,
};

static struct usb_endpoint_descriptor *
fb_ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *fs,
	    struct usb_endpoint_descriptor *hs,
	    struct usb_endpoint_descriptor *ss)
{
	if (gadget_is_superspeed(g) && g->speed >= USB_SPEED_SUPER)
		return ss;

	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

/*
 * static strings, in UTF-8
 */
static const char fastboot_name[] = "Android Fastboot";

static struct usb_string fastboot_string_defs[] = {
	[0].s = fastboot_name,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_fastboot = {
	.language	= 0x0409,	/* en-us */
	.strings	= fastboot_string_defs,
};

static struct usb_gadget_strings *fastboot_strings[] = {
	&stringtab_fastboot,
	NULL,
};

static void rx_handler_command(struct usb_ep *ep, struct usb_request *req);
#ifdef CONFIG_ARCH_SSTAR
static void fastboot_tx_write_info(const char *msg);
#endif

static void fastboot_complete(struct usb_ep *ep, struct usb_request *req)
{
	int status = req->status;
	if (!status)
		return;
	printf("status: %d ep '%s' trans: %d\n", status, ep->name, req->actual);
}

static int fastboot_bind(struct usb_configuration *c, struct usb_function *f)
{
	int id;
	struct usb_gadget *gadget = c->cdev->gadget;
	struct f_fastboot *f_fb = func_to_fastboot(f);
	const char *s;

	/* DYNAMIC interface numbers assignments */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	interface_desc.bInterfaceNumber = id;

	/* Enable OS and Extended Properties Feature Descriptor */
	c->cdev->use_os_string = 1;
	f->os_desc_table = &fb_os_desc_table;
	f->os_desc_n = 1;
	f->os_desc_table->if_id = id;
	INIT_LIST_HEAD(&fb_os_desc.ext_prop);
	fb_ext_prop.name_len = strlen(fb_ext_prop.name) * 2 + 2;
	fb_os_desc.ext_prop_len = 10 + fb_ext_prop.name_len;
	fb_os_desc.ext_prop_count = 1;
	fb_ext_prop.data_len = strlen(fb_ext_prop.data) * 2 + 2;
	fb_os_desc.ext_prop_len += fb_ext_prop.data_len + 4;
	list_add_tail(&fb_ext_prop.entry, &fb_os_desc.ext_prop);

	id = usb_string_id(c->cdev);
	if (id < 0)
		return id;
	fastboot_string_defs[0].id = id;
	interface_desc.iInterface = id;

	f_fb->in_ep = usb_ep_autoconfig(gadget, &fs_ep_in);
	if (!f_fb->in_ep)
		return -ENODEV;
	f_fb->in_ep->driver_data = c->cdev;

	f_fb->out_ep = usb_ep_autoconfig(gadget, &fs_ep_out);
	if (!f_fb->out_ep)
		return -ENODEV;
	f_fb->out_ep->driver_data = c->cdev;

	f->descriptors = fb_fs_function;

	if (gadget_is_dualspeed(gadget)) {
		/* Assume endpoint addresses are the same for both speeds */
		hs_ep_in.bEndpointAddress = fs_ep_in.bEndpointAddress;
		hs_ep_out.bEndpointAddress = fs_ep_out.bEndpointAddress;
		/* copy HS descriptors */
		f->hs_descriptors = fb_hs_function;
	}

	if (gadget_is_superspeed(gadget)) {
		ss_ep_in.bEndpointAddress = fs_ep_in.bEndpointAddress;
		ss_ep_out.bEndpointAddress = fs_ep_out.bEndpointAddress;
		f->ss_descriptors = fb_ss_function;
#ifdef CONFIG_ARCH_SSTAR
		fb_ss_bulk_comp_desc.bMaxBurst = min_t(unsigned, EP_BUFFER_SIZE / 1024, 15);
#endif
	}

	s = env_get("serial#");
	if (s)
		g_dnl_set_serialnumber((char *)s);

#ifdef CONFIG_ARCH_SSTAR
	fastboot_set_sendinfo_callback(fastboot_tx_write_info);
#endif
	return 0;
}

static void fastboot_unbind(struct usb_configuration *c, struct usb_function *f)
{
	f->os_desc_table = NULL;
	list_del(&fb_os_desc.ext_prop);
	memset(fastboot_func, 0, sizeof(*fastboot_func));
}

static void fastboot_disable(struct usb_function *f)
{
	struct f_fastboot *f_fb = func_to_fastboot(f);

	usb_ep_disable(f_fb->out_ep);
	usb_ep_disable(f_fb->in_ep);

	if (f_fb->out_req) {
		free(f_fb->out_req->buf);
		usb_ep_free_request(f_fb->out_ep, f_fb->out_req);
		f_fb->out_req = NULL;
	}
	if (f_fb->in_req) {
		free(f_fb->in_req->buf);
		usb_ep_free_request(f_fb->in_ep, f_fb->in_req);
		f_fb->in_req = NULL;
	}
}

static struct usb_request *fastboot_start_ep(struct usb_ep *ep)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, 0);
	if (!req)
		return NULL;

	req->length = EP_BUFFER_SIZE;
	req->buf = memalign(CONFIG_SYS_CACHELINE_SIZE, EP_BUFFER_SIZE);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	memset(req->buf, 0, req->length);
	return req;
}

static int fastboot_set_alt(struct usb_function *f,
			    unsigned interface, unsigned alt)
{
	int ret;
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_gadget *gadget = cdev->gadget;
	struct f_fastboot *f_fb = func_to_fastboot(f);
	const struct usb_endpoint_descriptor *d;

	debug("%s: func: %s intf: %d alt: %d\n",
	      __func__, f->name, interface, alt);

	d = fb_ep_desc(gadget, &fs_ep_out, &hs_ep_out, &ss_ep_out);
#ifdef CONFIG_ARCH_SSTAR
	if (gadget_is_superspeed(gadget))
		f_fb->out_ep->maxburst = fb_ss_bulk_comp_desc.bMaxBurst + 1;
#endif
	ret = usb_ep_enable(f_fb->out_ep, d);
	if (ret) {
		puts("failed to enable out ep\n");
		return ret;
	}

	f_fb->out_req = fastboot_start_ep(f_fb->out_ep);
	if (!f_fb->out_req) {
		puts("failed to alloc out req\n");
		ret = -EINVAL;
		goto err;
	}
	f_fb->out_req->complete = rx_handler_command;

	d = fb_ep_desc(gadget, &fs_ep_in, &hs_ep_in, &ss_ep_in);
#ifdef CONFIG_ARCH_SSTAR
	if (gadget_is_superspeed(gadget))
		f_fb->in_ep->maxburst = fb_ss_bulk_comp_desc.bMaxBurst + 1;
#endif
	ret = usb_ep_enable(f_fb->in_ep, d);
	if (ret) {
		puts("failed to enable in ep\n");
		goto err;
	}

	f_fb->in_req = fastboot_start_ep(f_fb->in_ep);
	if (!f_fb->in_req) {
		puts("failed alloc req in\n");
		ret = -EINVAL;
		goto err;
	}
	f_fb->in_req->complete = fastboot_complete;

	ret = usb_ep_queue(f_fb->out_ep, f_fb->out_req, 0);
	if (ret)
		goto err;

	return 0;
err:
	fastboot_disable(f);
	return ret;
}

static int fastboot_add(struct usb_configuration *c)
{
	struct f_fastboot *f_fb = fastboot_func;
	int status;

	debug("%s: cdev: 0x%p\n", __func__, c->cdev);

	if (!f_fb) {
		f_fb = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*f_fb));
		if (!f_fb)
			return -ENOMEM;

		fastboot_func = f_fb;
		memset(f_fb, 0, sizeof(*f_fb));
	}

	f_fb->usb_function.name = "f_fastboot";
	f_fb->usb_function.bind = fastboot_bind;
	f_fb->usb_function.unbind = fastboot_unbind;
	f_fb->usb_function.set_alt = fastboot_set_alt;
	f_fb->usb_function.disable = fastboot_disable;
	f_fb->usb_function.strings = fastboot_strings;

	status = usb_add_function(c, &f_fb->usb_function);
	if (status) {
		free(f_fb);
		fastboot_func = NULL;
	}

	return status;
}
DECLARE_GADGET_BIND_CALLBACK(usb_dnl_fastboot, fastboot_add);

static int fastboot_tx_write(const char *buffer, unsigned int buffer_size)
{
	struct usb_request *in_req = fastboot_func->in_req;
	int ret;

	memcpy(in_req->buf, buffer, buffer_size);
	in_req->length = buffer_size;

	usb_ep_dequeue(fastboot_func->in_ep, in_req);

	ret = usb_ep_queue(fastboot_func->in_ep, in_req, 0);
	if (ret)
		printf("Error %d on queue\n", ret);
	return 0;
}

#ifdef CONFIG_ARCH_SSTAR
static void fastboot_fifo_complete(struct usb_ep *ep, struct usb_request *req)
{
	int status = req->status;
	struct usb_req *request;

	if (!status) {
		if (fastboot_func->front != NULL) {
			request = fastboot_func->front;
			fastboot_func->front = fastboot_func->front->next;
			usb_ep_free_request(ep, request->in_req);
			free(request);
		}
	}

	if (!fastboot_func->front)
		fastboot_func->rear = NULL;
}

static void fastboot_tx_write_info(const char *msg)
{
	int ret = 0;

	/* alloc usb request FIFO node */
	struct usb_req *req = (struct usb_req *)malloc(ALIGN(sizeof(struct usb_req), 64));
	if (!req) {
		printf("failed alloc usb req!\n");
		return;
	}

	/* usb request node FIFO enquene */
	if ((fastboot_func->front == NULL) && (fastboot_func->rear == NULL)) {
		fastboot_func->front = fastboot_func->rear = req;
		req->next = NULL;
	} else {
		fastboot_func->rear->next = req;
		fastboot_func->rear = req;
		req->next = NULL;
	}

	/* alloc in request for current node */
	req->in_req = fastboot_start_ep(fastboot_func->in_ep);
	if (!req->in_req) {
		printf("failed alloc req in\n");
		fastboot_disable(&(fastboot_func->usb_function));
		return;
	}
	req->in_req->complete = fastboot_fifo_complete;

	fastboot_response("INFO", req->in_req->buf, "%s", msg);
	req->in_req->length = strlen(req->in_req->buf);

	ret = usb_ep_queue(fastboot_func->in_ep, req->in_req, 0);
	if (ret) {
		printf("Error %d on queue\n", ret);
	}

	return;
}

#endif

int fastboot_tx_write_str(const char *buffer)
{
	return fastboot_tx_write(buffer, strlen(buffer));
}

static void compl_do_reset(struct usb_ep *ep, struct usb_request *req)
{
	do_reset(NULL, 0, 0, NULL);
}

static unsigned int rx_bytes_expected(struct usb_ep *ep)
{
	int rx_remain = fastboot_data_remaining();
	unsigned int rem;
	unsigned int maxpacket = usb_endpoint_maxp(ep->desc);

	if (rx_remain <= 0)
		return 0;
	else if (rx_remain > EP_BUFFER_SIZE)
		return EP_BUFFER_SIZE;

	/*
	 * Some controllers e.g. DWC3 don't like OUT transfers to be
	 * not ending in maxpacket boundary. So just make them happy by
	 * always requesting for integral multiple of maxpackets.
	 * This shouldn't bother controllers that don't care about it.
	 */
	rem = rx_remain % maxpacket;
	if (rem > 0)
		rx_remain = rx_remain + (maxpacket - rem);

	return rx_remain;
}

static void rx_handler_dl_image(struct usb_ep *ep, struct usb_request *req)
{
	char response[FASTBOOT_RESPONSE_LEN] = {0};
	unsigned int transfer_size = fastboot_data_remaining();
	const unsigned char *buffer = req->buf;
	unsigned int buffer_size = req->actual;

	if (req->status != 0) {
		printf("Bad status: %d\n", req->status);
		return;
	}

	if (buffer_size < transfer_size)
		transfer_size = buffer_size;

	fastboot_data_download(buffer, transfer_size, response);
	if (response[0]) {
		fastboot_tx_write_str(response);
	} else if (!fastboot_data_remaining()) {
		fastboot_data_complete(response);

		/*
		 * Reset global transfer variable
		 */
		req->complete = rx_handler_command;
		req->length = EP_BUFFER_SIZE;

		fastboot_tx_write_str(response);
	} else {
		req->length = rx_bytes_expected(ep);
	}

	req->actual = 0;
	usb_ep_queue(ep, req, 0);
}

static void do_exit_on_complete(struct usb_ep *ep, struct usb_request *req)
{
	g_dnl_trigger_detach();
}

static void do_bootm_on_complete(struct usb_ep *ep, struct usb_request *req)
{
	fastboot_boot();
	do_exit_on_complete(ep, req);
}

#if CONFIG_IS_ENABLED(FASTBOOT_UUU_SUPPORT)
static void do_acmd_complete(struct usb_ep *ep, struct usb_request *req)
{
	/* When usb dequeue complete will be called
	 *  Need status value before call run_command.
	 * otherwise, host can't get last message.
	 */
	if (req->status == 0)
		fastboot_acmd_complete();
}
#endif

static void rx_handler_command(struct usb_ep *ep, struct usb_request *req)
{
	char *cmdbuf = req->buf;
	char response[FASTBOOT_RESPONSE_LEN] = {0};
	int cmd = -1;

	if (req->status != 0 || req->length == 0)
		return;

	if (req->actual < req->length) {
		cmdbuf[req->actual] = '\0';
		cmd = fastboot_handle_command(cmdbuf, response);
	} else {
		pr_err("buffer overflow");
		fastboot_fail("buffer overflow", response);
	}

	if (!strncmp("DATA", response, 4)) {
		req->complete = rx_handler_dl_image;
		req->length = rx_bytes_expected(ep);
	}

	if (!strncmp("OKAY", response, 4)) {
		switch (cmd) {
		case FASTBOOT_COMMAND_BOOT:
			fastboot_func->in_req->complete = do_bootm_on_complete;
			break;

		case FASTBOOT_COMMAND_CONTINUE:
			fastboot_func->in_req->complete = do_exit_on_complete;
			break;

		case FASTBOOT_COMMAND_REBOOT:
		case FASTBOOT_COMMAND_REBOOT_BOOTLOADER:
		case FASTBOOT_COMMAND_REBOOT_FASTBOOTD:
		case FASTBOOT_COMMAND_REBOOT_RECOVERY:
			fastboot_func->in_req->complete = compl_do_reset;
			break;
#if CONFIG_IS_ENABLED(FASTBOOT_UUU_SUPPORT)
		case FASTBOOT_COMMAND_ACMD:
			fastboot_func->in_req->complete = do_acmd_complete;
			break;
#endif
		}
	}

	fastboot_tx_write_str(response);

	*cmdbuf = '\0';
	req->actual = 0;
	usb_ep_queue(ep, req, 0);
}
