/*
 * USB device controllers have lots of quirks.  Use these macros in
 * gadget drivers or other code that needs to deal with them, and which
 * autoconfigures instead of using early binding to the hardware.
 *
 * This SHOULD eventually work like the ARM mach_is_*() stuff, driven by
 * some config file that gets updated as new hardware is supported.
 * (And avoiding all runtime comparisons in typical one-choice configs!)
 *
 * NOTE:  some of these controller drivers may not be available yet.
 * Some are available on 2.4 kernels; several are available, but not
 * yet pushed in the 2.6 mainline tree.
 *
 * Ported to U-Boot by: Thomas Smits <ts.smits@gmail.com> and
 *                      Remy Bohmer <linux@bohmer.net>
 */
#ifdef CONFIG_USB_GADGET_NET2280
#define	gadget_is_net2280(g)	(!strcmp("net2280", (g)->name))
#else
#define	gadget_is_net2280(g)	0
#endif

#ifdef CONFIG_USB_GADGET_AMD5536UDC
#define	gadget_is_amd5536udc(g)	(!strcmp("amd5536udc", (g)->name))
#else
#define	gadget_is_amd5536udc(g)	0
#endif

#ifdef CONFIG_USB_GADGET_DUMMY_HCD
#define	gadget_is_dummy(g)	(!strcmp("dummy_udc", (g)->name))
#else
#define	gadget_is_dummy(g)	0
#endif

#ifdef CONFIG_USB_GADGET_PXA2XX
#define	gadget_is_pxa(g)	(!strcmp("pxa2xx_udc", (g)->name))
#else
#define	gadget_is_pxa(g)	0
#endif

#ifdef CONFIG_USB_GADGET_GOKU
#define	gadget_is_goku(g)	(!strcmp("goku_udc", (g)->name))
#else
#define	gadget_is_goku(g)	0
#endif

/* SH3 UDC -- not yet ported 2.4 --> 2.6 */
#ifdef CONFIG_USB_GADGET_SUPERH
#define	gadget_is_sh(g)		(!strcmp("sh_udc", (g)->name))
#else
#define	gadget_is_sh(g)		0
#endif

/* not yet stable on 2.6 (would help "original Zaurus") */
#ifdef CONFIG_USB_GADGET_SA1100
#define	gadget_is_sa1100(g)	(!strcmp("sa1100_udc", (g)->name))
#else
#define	gadget_is_sa1100(g)	0
#endif

/* handhelds.org tree (?) */
#ifdef CONFIG_USB_GADGET_MQ11XX
#define	gadget_is_mq11xx(g)	(!strcmp("mq11xx_udc", (g)->name))
#else
#define	gadget_is_mq11xx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_OMAP
#define	gadget_is_omap(g)	(!strcmp("omap_udc", (g)->name))
#else
#define	gadget_is_omap(g)	0
#endif

/* not yet ported 2.4 --> 2.6 */
#ifdef CONFIG_USB_GADGET_N9604
#define	gadget_is_n9604(g)	(!strcmp("n9604_udc", (g)->name))
#else
#define	gadget_is_n9604(g)	0
#endif

/* various unstable versions available */
#ifdef CONFIG_USB_GADGET_PXA27X
#define	gadget_is_pxa27x(g)	(!strcmp("pxa27x_udc", (g)->name))
#else
#define	gadget_is_pxa27x(g)	0
#endif

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
#define gadget_is_atmel_usba(g)	(!strcmp("atmel_usba_udc", (g)->name))
#else
#define gadget_is_atmel_usba(g)	0
#endif

#ifdef CONFIG_USB_GADGET_AT91
#define gadget_is_at91(g)	(!strcmp("at91_udc", (g)->name))
#else
#define gadget_is_at91(g)	0
#endif

/* status unclear */
#ifdef CONFIG_USB_GADGET_IMX
#define gadget_is_imx(g)	(!strcmp("imx_udc", (g)->name))
#else
#define gadget_is_imx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_FSL_USB2
#define gadget_is_fsl_usb2(g)	(!strcmp("fsl-usb2-udc", (g)->name))
#else
#define gadget_is_fsl_usb2(g)	0
#endif

/* Mentor high speed function controller */
/* from Montavista kernel (?) */
#ifdef CONFIG_USB_GADGET_MUSBHSFC
#define gadget_is_musbhsfc(g)	(!strcmp("musbhsfc_udc", (g)->name))
#else
#define gadget_is_musbhsfc(g)	0
#endif

/* Mentor high speed "dual role" controller, in peripheral role */
#ifdef CONFIG_USB_MUSB_GADGET
#define gadget_is_musbhdrc(g)	(!strcmp("musb-hdrc", (g)->name))
#else
#define gadget_is_musbhdrc(g)	0
#endif

#ifdef CONFIG_USB_GADGET_M66592
#define	gadget_is_m66592(g)	(!strcmp("m66592_udc", (g)->name))
#else
#define	gadget_is_m66592(g)	0
#endif

#ifdef CONFIG_CI_UDC
#define gadget_is_ci(g)        (!strcmp("ci_udc", (g)->name))
#else
#define gadget_is_ci(g)        0
#endif

#ifdef CONFIG_USB_GADGET_FOTG210
#define gadget_is_fotg210(g)        (!strcmp("fotg210_udc", (g)->name))
#else
#define gadget_is_fotg210(g)        0
#endif

#ifdef CONFIG_SSTAR_UDC_MSB250X
#define gadget_is_MSB250X(g)        (!strcmp("msb250x_udc", (g)->name))
#else
#define gadget_is_MSB250X(g)        0
#endif

#ifdef CONFIG_USB_DWC3_GADGET
#define gadget_is_dwc3(g)        (!strcmp("dwc3-gadget", (g)->name))
#else
#define gadget_is_dwc3(g)        0
#endif

#ifdef CONFIG_USB_CDNS3_GADGET
#define gadget_is_cdns3(g)        (!strcmp("cdns3-gadget", (g)->name))
#else
#define gadget_is_cdns3(g)        0
#endif

#ifdef CONFIG_USB_GADGET_MAX3420
#define gadget_is_max3420(g)        (!strcmp("max3420-udc", (g)->name))
#else
#define gadget_is_max3420(g)        0
#endif

#ifdef CONFIG_USB_MTU3_GADGET
#define gadget_is_mtu3(g)        (!strcmp("mtu3-gadget", (g)->name))
#else
#define gadget_is_mtu3(g)        0
#endif

/**
 * usb_gadget_controller_number - support bcdDevice id convention
 * @gadget: the controller being driven
 *
 * Return a 2-digit BCD value associated with the peripheral controller,
 * suitable for use as part of a bcdDevice value, or a negative error code.
 *
 * NOTE:  this convention is purely optional, and has no meaning in terms of
 * any USB specification.  If you want to use a different convention in your
 * gadget driver firmware -- maybe a more formal revision ID -- feel free.
 *
 * Hosts see these bcdDevice numbers, and are allowed (but not encouraged!)
 * to change their behavior accordingly.  For example it might help avoiding
 * some chip bug.
 */
static inline int usb_gadget_controller_number(struct usb_gadget *gadget)
{
	if (gadget_is_net2280(gadget))
		return 0x01;
	else if (gadget_is_dummy(gadget))
		return 0x02;
	else if (gadget_is_pxa(gadget))
		return 0x03;
	else if (gadget_is_sh(gadget))
		return 0x04;
	else if (gadget_is_sa1100(gadget))
		return 0x05;
	else if (gadget_is_goku(gadget))
		return 0x06;
	else if (gadget_is_mq11xx(gadget))
		return 0x07;
	else if (gadget_is_omap(gadget))
		return 0x08;
	else if (gadget_is_n9604(gadget))
		return 0x09;
	else if (gadget_is_pxa27x(gadget))
		return 0x10;
	else if (gadget_is_at91(gadget))
		return 0x12;
	else if (gadget_is_imx(gadget))
		return 0x13;
	else if (gadget_is_musbhsfc(gadget))
		return 0x14;
	else if (gadget_is_musbhdrc(gadget))
		return 0x15;
	else if (gadget_is_atmel_usba(gadget))
		return 0x17;
	else if (gadget_is_fsl_usb2(gadget))
		return 0x18;
	else if (gadget_is_amd5536udc(gadget))
		return 0x19;
	else if (gadget_is_m66592(gadget))
		return 0x20;
	else if (gadget_is_ci(gadget))
		return 0x21;
	else if (gadget_is_fotg210(gadget))
		return 0x22;
	else if (gadget_is_dwc3(gadget))
		return 0x23;
	else if (gadget_is_cdns3(gadget))
		return 0x24;
	else if (gadget_is_max3420(gadget))
		return 0x25;
	else if (gadget_is_mtu3(gadget))
		return 0x26;
	else if (gadget_is_MSB250X(gadget))
		return 0x27;
	return -ENOENT;
}
