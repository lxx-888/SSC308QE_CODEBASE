/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header declares the utility functions used by "Gadget Zero", plus
 * interfaces to its two single-configuration function drivers.
 */

#ifndef __G_ZERO_H
#define __G_ZERO_H

#define GZERO_BULK_BUFLEN	4096
#ifdef CONFIG_ARCH_SSTAR
#define GZERO_ISO_BUFLEN	3072
#define GZERO_INT_BUFLEN	512
#define GZERO_BULK_BUFLEN_VARY	0
#define GZERO_SS_INT_QLEN	1
#endif
#define GZERO_QLEN		32
#define GZERO_ISOC_INTERVAL	4
#define GZERO_ISOC_MAXPACKET	1024
#define GZERO_SS_BULK_QLEN	1

#define GZERO_SS_ISO_QLEN	8

struct usb_zero_options {
	unsigned pattern;
	unsigned isoc_interval;
	unsigned isoc_maxpacket;
	unsigned isoc_mult;
	unsigned isoc_maxburst;
	unsigned bulk_buflen;
#ifdef CONFIG_ARCH_SSTAR
    unsigned bulk_buflen_vary;
#endif
	unsigned qlen;
	unsigned ss_bulk_qlen;
	unsigned ss_iso_qlen;
};

struct f_ss_opts {
	struct usb_function_instance func_inst;
	unsigned pattern;
	unsigned isoc_interval;
	unsigned isoc_maxpacket;
	unsigned isoc_mult;
	unsigned isoc_maxburst;
	unsigned bulk_buflen;
#ifdef CONFIG_ARCH_SSTAR
	unsigned iso_buflen;
	unsigned int_buflen;
	unsigned int_qlen;
	unsigned buflen_vary;
#endif
	unsigned bulk_qlen;
	unsigned iso_qlen;

	/*
	 * Read/write access to configfs attributes is handled by configfs.
	 *
	 * This is to protect the data from concurrent access by read/write
	 * and create symlink/remove symlink.
	 */
	struct mutex			lock;
	int				refcnt;
};

struct f_lb_opts {
	struct usb_function_instance func_inst;
	unsigned bulk_buflen;
	unsigned qlen;

	/*
	 * Read/write access to configfs attributes is handled by configfs.
	 *
	 * This is to protect the data from concurrent access by read/write
	 * and create symlink/remove symlink.
	 */
	struct mutex			lock;
	int				refcnt;
};

void lb_modexit(void);
int lb_modinit(void);

/* common utilities */
#ifdef CONFIG_ARCH_SSTAR
void disable_endpoints(struct usb_composite_dev *cdev,
		struct usb_ep *in, struct usb_ep *out,
		struct usb_ep *iso_in, struct usb_ep *iso_out,
		struct usb_ep *int_in, struct usb_ep *int_out);
#else
void disable_endpoints(struct usb_composite_dev *cdev,
		struct usb_ep *in, struct usb_ep *out,
		struct usb_ep *iso_in, struct usb_ep *iso_out);
#endif

#endif /* __G_ZERO_H */
