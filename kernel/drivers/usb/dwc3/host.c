// SPDX-License-Identifier: GPL-2.0
/*
 * host.c - DesignWare USB3 DRD Controller Host Glue
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 */

#include <linux/acpi.h>
#include <linux/platform_device.h>

#include "core.h"

#ifdef CONFIG_ARCH_SSTAR
#include "xhci-plat.h"
#include <linux/of_gpio.h>
#include <linux/gpio.h>

static int sstar_xhci_power_gpio_get(struct platform_device *pdev, int *gpio_power)
{
	int                    ret    = 0;
	int                    gpio_vbus_power;
	struct device *        dev    = &pdev->dev;
	struct device_node *   np     = NULL;

	np = of_parse_phandle(dev->of_node, "gpio_vbus_power", 0);
	if (np && of_property_read_bool(np, "gpio_vbus_ctrl"))
	{
		gpio_vbus_power = of_get_named_gpio(np, "gpio_vbus_ctrl", 0);
		if (!gpio_is_valid(gpio_vbus_power))
		{
			dev_err(dev, "%s: get gpio_vbus_power %d error\n", __func__, gpio_vbus_power);
			ret = -EINVAL;
			goto err;
		}
		dev_info(dev, "%s: vbus contorl gpio=%d\n", __func__, gpio_vbus_power);

			ret = devm_gpio_request(dev, gpio_vbus_power, "gpio_vbus_power");
		if (ret < 0)
		{
			dev_err(dev, "%s: gpio_request error, ret=%d, gpio=%d\n", __func__, ret, gpio_vbus_power);
			goto err;
		}
		*gpio_power = gpio_vbus_power;
	}

err:
	if (np)
		of_node_put(np);
	return ret;
}

static int sstar_xhci_suspend_quirk(struct usb_hcd *hcd)
{
	int gpio_vbus_power;

	if (!device_property_read_u32(hcd->self.controller, "gpio_vbus_power", &gpio_vbus_power)) {
		dev_info(hcd->self.controller, "%s vbus power down\r\n", __func__);
		gpio_direction_output(gpio_vbus_power, 0);
		msleep(5);
	}
	return 0;
}

static int sstar_xhci_resume_quirk(struct usb_hcd *hcd)
{
	int gpio_vbus_power;

	if (!device_property_read_u32(hcd->self.controller, "gpio_vbus_power", &gpio_vbus_power)) {
		dev_info(hcd->self.controller, "%s vbus power up\r\n", __func__);
		gpio_direction_output(gpio_vbus_power, 1);
	}
	return 0;
}

static int sstar_xhci_plat_setup(struct usb_hcd *hcd)
{
	unsigned gpio_vbus_power;
	struct usb_hcd *shared_hcd = hcd->shared_hcd;
	shared_hcd->usb_phy = hcd->usb_phy;

	if (!device_property_read_u32(hcd->self.controller, "gpio_vbus_power", &gpio_vbus_power))
	{
		dev_info(hcd->self.controller, "enable vbus-power-gpio:%u\r\n", gpio_vbus_power);
		gpio_direction_output(gpio_vbus_power, 1);
	}

	dev_info(hcd->self.controller, "%s controller dma_mask: 0x%llx\r\n", __func__, *hcd->self.controller->dma_mask);

	return dma_set_mask_and_coherent(hcd->self.controller, *hcd->self.sysdev->dma_mask);
}

static const struct xhci_plat_priv sstar_xhci_plat = {
	.plat_setup = sstar_xhci_plat_setup,
	.quirks = XHCI_SKIP_PHY_INIT,
	.suspend_quirk = sstar_xhci_suspend_quirk,
	.resume_quirk = sstar_xhci_resume_quirk,
};

#endif

static int dwc3_host_get_irq(struct dwc3 *dwc)
{
	struct platform_device	*dwc3_pdev = to_platform_device(dwc->dev);
	int irq;

	irq = platform_get_irq_byname_optional(dwc3_pdev, "host");
	if (irq > 0)
		goto out;

	if (irq == -EPROBE_DEFER)
		goto out;

	irq = platform_get_irq_byname_optional(dwc3_pdev, "dwc_usb3");
	if (irq > 0)
		goto out;

	if (irq == -EPROBE_DEFER)
		goto out;

	irq = platform_get_irq(dwc3_pdev, 0);
	if (irq > 0)
		goto out;

	if (!irq)
		irq = -EINVAL;

out:
	return irq;
}

int dwc3_host_init(struct dwc3 *dwc)
{
	struct property_entry	props[4];
	struct platform_device	*xhci;
	int			ret, irq;
	struct resource		*res;
	struct platform_device	*dwc3_pdev = to_platform_device(dwc->dev);
	int			prop_idx = 0;
#ifdef CONFIG_ARCH_SSTAR
	int 		gpio_vbus_power;
#endif

	irq = dwc3_host_get_irq(dwc);
	if (irq < 0)
		return irq;

	res = platform_get_resource_byname(dwc3_pdev, IORESOURCE_IRQ, "host");
	if (!res)
		res = platform_get_resource_byname(dwc3_pdev, IORESOURCE_IRQ,
				"dwc_usb3");
	if (!res)
		res = platform_get_resource(dwc3_pdev, IORESOURCE_IRQ, 0);
	if (!res)
		return -ENOMEM;

	dwc->xhci_resources[1].start = irq;
	dwc->xhci_resources[1].end = irq;
	dwc->xhci_resources[1].flags = res->flags;
	dwc->xhci_resources[1].name = res->name;

	xhci = platform_device_alloc("xhci-hcd", PLATFORM_DEVID_AUTO);
	if (!xhci) {
		dev_err(dwc->dev, "couldn't allocate xHCI device\n");
		return -ENOMEM;
	}

#ifdef CONFIG_ARCH_SSTAR
	xhci->dev.parent	= dwc->sysdev;
#else
	xhci->dev.parent	= dwc->dev;
#endif
	ACPI_COMPANION_SET(&xhci->dev, ACPI_COMPANION(dwc->dev));

	dwc->xhci = xhci;

	ret = platform_device_add_resources(xhci, dwc->xhci_resources,
						DWC3_XHCI_RESOURCES_NUM);
	if (ret) {
		dev_err(dwc->dev, "couldn't add resources to xHCI device\n");
		goto err;
	}

	memset(props, 0, sizeof(struct property_entry) * ARRAY_SIZE(props));

	if (dwc->usb3_lpm_capable)
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("usb3-lpm-capable");

	if (dwc->usb2_lpm_disable)
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("usb2-lpm-disable");

	/**
	 * WORKAROUND: dwc3 revisions <=3.00a have a limitation
	 * where Port Disable command doesn't work.
	 *
	 * The suggested workaround is that we avoid Port Disable
	 * completely.
	 *
	 * This following flag tells XHCI to do just that.
	 */
	if (DWC3_VER_IS_WITHIN(DWC3, ANY, 300A))
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("quirk-broken-port-ped");

#ifdef CONFIG_ARCH_SSTAR
	if (device_property_read_bool(&dwc3_pdev->dev, "gpio_vbus_power"))
	{
		if (sstar_xhci_power_gpio_get(dwc3_pdev, &gpio_vbus_power) == 0)
			props[prop_idx++] = PROPERTY_ENTRY_U32("gpio_vbus_power", gpio_vbus_power);
	}

	platform_device_add_data(xhci, &sstar_xhci_plat, sizeof sstar_xhci_plat);
#endif

	if (prop_idx) {
		ret = platform_device_add_properties(xhci, props);
		if (ret) {
			dev_err(dwc->dev, "failed to add properties to xHCI\n");
			goto err;
		}
	}

	ret = platform_device_add(xhci);
	if (ret) {
		dev_err(dwc->dev, "failed to register xHCI device\n");
		goto err;
	}
	device_link_add(&xhci->dev, dwc->dev, DL_FLAG_STATELESS);

	return 0;
err:
	platform_device_put(xhci);
	return ret;
}

void dwc3_host_exit(struct dwc3 *dwc)
{
	platform_device_unregister(dwc->xhci);
}
