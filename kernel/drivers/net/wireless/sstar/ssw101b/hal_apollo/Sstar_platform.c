#include <linux/version.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/spinlock.h>
//#include <asm/mach-types.h>
#include <net/Sstar_mac80211.h>
#include <linux/kthread.h>

#include "apollo.h"
#include "sbus.h"
#include "apollo_plat.h"

#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT31)

#define PLATFORMINF "ingenict31"
extern int jzmmc_manual_detect(int index, int on);
//extern	int	gpio_request(SDIO_WIFI_POWER, "sdio_wifi_power_on");
//extern	int	gpio_direction_output(WL_REG_EN, 0);
//extern	int	gpio_direction_output(WL_REG_EN, 1);
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT41)

#define PLATFORMINF "ingenict41"
extern int jzmmc_manual_detect(int index, int on);
static int WL_REG_EN = 32 + 25;
//extern	int	gpio_request(SDIO_WIFI_POWER, "sdio_wifi_power_on");
//extern	int	gpio_direction_output(WL_REG_EN, 0);
//extern	int	gpio_direction_output(WL_REG_EN, 1);
#endif

#ifdef SDIO_BUS
#ifndef CONFIG_SSTAR_SDIO_MMC_ID
#define CONFIG_SSTAR_SDIO_MMC_ID "mmc0"
#endif

#pragma message(CONFIG_SSTAR_SDIO_MMC_ID)

#if (SSTAR_WIFI_PLATFORM == PLATFORM_XUNWEI)
#define PLATFORMINF "xunwei"
#define CARD_INSERT_GPIO EXYNOS4_GPX3(2)

#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_S805)
#define PLATFORMINF "aml_s805"
#endif

#if (SSTAR_WIFI_PLATFORM == 12)
#define PLATFORMINF "cdlinux"
#endif

#if (SSTAR_WIFI_PLATFORM == 10)
#define PLATFORMINF "rockchip"
#endif
#if (SSTAR_WIFI_PLATFORM == 9)
#define PLATFORMINF "amlogic_905X"
#endif
#if (SSTAR_WIFI_PLATFORM == 8)
#define PLATFORMINF "amlogic_905"
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_SUN6I)
#define PLATFORMINF "sun6i"

#include <mach/sys_config.h>
#include <mach/gpio.h>
#include <linux/regulator/consumer.h>
#include <mach/sys_config.h>

#endif
#if (SSTAR_WIFI_PLATFORM == 10)
#include <linux/rfkill-wlan.h>
extern int rockchip_wifi_power(int on);
extern int rockchip_wifi_set_carddetect(int val);
#endif
#if ((SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_S805) ||                         \
     (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_905))

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
extern void wifi_teardown_dt(void);
extern int wifi_setup_dt(void);
#endif
#endif //#if (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_S805)

#if (SSTAR_WIFI_PLATFORM == PLATFORM_FRIENDLY)
#define PLATFORMINF "friendly"
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/ioport.h>
#include <plat/gpio-cfg.h>
#include <plat/sdhci.h>
#include <plat/devs.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/board-wlan.h>
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_SUN6I_64)
#define PLATFORMINF "sun6i_64bit"
extern void sunxi_mmc_rescan_card(unsigned ids);
extern void sunxi_wlan_set_power(int on);
extern int sunxi_wlan_get_bus_index(void);
//extern int sunxi_wlan_get_oob_irq(void);
//extern unsigned int oob_irq;
#endif
#ifndef PLATFORMINF
#define PLATFORMINF "no platform"
#endif

static const char *platform = PLATFORMINF;

#pragma message(PLATFORMINF)

#if (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_905)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
extern int wifi_irq_num(void);
#endif

u32 Sstar_wlan_get_oob_irq(void)
{
	u32 host_oob_irq = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
	host_oob_irq = INT_GPIO_4;
#else
	host_oob_irq = wifi_irq_num();
#endif
	Sstar_printk_platform("host_oob_irq: %d \r\n", host_oob_irq);

	return host_oob_irq;
}
#endif

static int Sstar_platform_power_ctrl(const struct Sstar_platform_data *pdata,
				     bool enabled)
{
	int ret = 0;
#ifndef USB_BUS
#if (SSTAR_WIFI_PLATFORM == PLATFORM_XUNWEI) ||                                \
	(SSTAR_WIFI_PLATFORM == PLATFORM_FRIENDLY)
	{
#if (PROJ_TYPE == ARES_A) //for ARESA chip hw reset pin bug
		enabled = 1;
#endif //#if (PROJ_TYPE>=ARES_A)
#ifndef WIFI_FW_DOWNLOAD
		enabled = 1;
#endif //#ifdef WIFI_FW_DOWNLOAD
		if (gpio_request(pdata->power_gpio, "WIFI_POWERON") != 0) {
			Sstar_printk_platform(
				"[sigmastar] ERROR:Cannot request WIFI_POWERON\n");
		} else {
			gpio_direction_output(pdata->power_gpio,
					      1); /* WLAN_CHIP_PWD */
			gpio_set_value(pdata->power_gpio, enabled);
			mdelay(1);
			gpio_free(pdata->power_gpio);
			Sstar_printk_platform(
				"[sigmastar] + %s :EXYNOS4_GPC1(0) wlan powerwew %s\n",
				__func__, enabled ? "on" : "off");
		}
	}
#endif //(SSTAR_WIFI_PLATFORM == PLATFORM_XUNWEI) ||(SSTAR_WIFI_PLATFORM == PLATFORM_FRIENDLY)

#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT31)
	{
		if (enabled) {
			Sstar_printk_platform("[%s] reset sigmastar wifi !\n",
					      __func__);

			gpio_request(WL_REG_EN, "sdio_wifi_power_on");

			Sstar_printk_platform(
				"PLATFORM_INGENICT31 SDIO WIFI_RESET 0 \n");
			gpio_direction_output(WL_REG_EN, 0);
			msleep(300);
			Sstar_printk_platform(
				"PLATFORM_INGENICT31 SDIO WIFI_RESET 1 \n");
			gpio_direction_output(WL_REG_EN, 1);
			msleep(100);
		}
	}
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT41)
	{
		if (enabled) {
			Sstar_printk_platform("[%s] reset sigmastar wifi !\n",
					      __func__);

			gpio_request(WL_REG_EN, "sdio_wifi_power_on");

			Sstar_printk_platform(
				"PLATFORM_INGENICT41 SDIO WIFI_RESET 0 \n");
			gpio_direction_output(WL_REG_EN, 0);
			msleep(300);
			Sstar_printk_platform(
				"PLATFORM_INGENICT41 SDIO WIFI_RESET 1 \n");
			gpio_direction_output(WL_REG_EN, 1);
			msleep(100);
		}
	}
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_SUN6I)
	{
		extern void wifi_pm_power(int on);
		pdata = pdata;
		wifi_pm_power(enabled);
	}
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_FRIENDLY)
	{
	}
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_SUN6I_64)
	//int wlan_bus_index = sunxi_wlan_get_bus_index();
#if 0
	if (gpio_request(354, "wlan_regon")!=0) {
		printk("[sigmastar] ERROR:Cannot request WIFI_POWERON\n");
	} else {
		gpio_direction_output(354, 0);/* WLAN_CHIP_PWD */
		//gpio_set_value(pdata->power_gpio, enabled);
		mdelay(1);
		printk("[sigmastar] + %s :EXYNOS4_GPC1(0) wlan powerwew %s\n",__func__, enabled?"on":"off");
	}
	if (gpio_request(355, "wlan_hostwake")!=0) {
		printk("[sigmastar] ERROR:Cannot request WIFI_POWERON\n");
	} else {
		gpio_direction_input(355);
		//gpio_set_value(pdata->power_gpio, enabled);
		mdelay(1);
		printk("[sigmastar] + %s :EXYNOS4_GPC1(0) wlan host wake %s\n",__func__, enabled?"on":"off");
	}
#endif
	mdelay(100);
	sunxi_wlan_set_power(enabled);
	//gpio_free(354);	gpio_free(355);
#endif
#if ((SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_S805) ||                         \
     (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_905))

	extern void sdio_reinit(void);
	extern void extern_wifi_set_enable(int is_on);
	if (enabled) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
		if (wifi_setup_dt()) {
			Sstar_printk_platform("%s : fail to setup dt\n",
					      __func__);
		}
#endif
		extern_wifi_set_enable(0);
		msleep(200);
		extern_wifi_set_enable(1);
		msleep(200);
		sdio_reinit();
		Sstar_printk_platform("Sstar sdio extern_wifi_set_enable 1\n");
	} else {
		extern_wifi_set_enable(0);
		msleep(200);
		Sstar_printk_platform("Sstar sdio extern_wifi_set_enable 0\n");
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
		wifi_teardown_dt();
#endif
	}

#endif //PLATFORM_AMLOGIC_S805

#if (SSTAR_WIFI_PLATFORM == 10)
	mdelay(100);
	rockchip_wifi_power(enabled);
#endif

#endif
	Sstar_printk_platform("[%s]:platform set power [%d]\n", platform,
			      enabled);
	return ret;
}
static int Sstar_platform_insert_crtl(const struct Sstar_platform_data *pdata,
				      bool enabled)
{
	int ret = 0;
#ifdef SDIO_BUS

#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT31)
	{
		mdelay(100);
		jzmmc_manual_detect(1, enabled);
		Sstar_printk_platform(
			"============platform insert crtl====== enable=%d\n",
			enabled);
	}
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT41)
	{
		mdelay(100);
		jzmmc_manual_detect(1, enabled);
		Sstar_printk_platform(
			"============platform insert crtl====== enable=%d\n",
			enabled);
	}
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_XUNWEI)
	{
		int outValue;

		if (enabled) {
			outValue = 0;
		} else {
			outValue = 1;
		}

		mdelay(10);
		if (gpio_request(CARD_INSERT_GPIO, "WIFI_GPIO2") != 0) {
			Sstar_printk_platform(
				"[sigmastar] ERROR:Cannot request WIFI_GPIO2\n");
		} else {
			gpio_direction_output(CARD_INSERT_GPIO,
					      1); /* SDIO_CARD_PWD */
			gpio_set_value(CARD_INSERT_GPIO, outValue);
			gpio_free(CARD_INSERT_GPIO);
			Sstar_printk_platform(
				"[sigmastar] + %s : wlan card %s\n", __func__,
				enabled ? "insert" : "remmove");
		}
	}
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_SUN6I)
	{
		extern void sw_mci_rescan_card(unsigned id, unsigned insert);
		script_item_u val;
		script_item_value_type_e type;
		static int sdc_id = -1;
		pdata = pdata;
		type = script_get_item("wifi_para", "wifi_sdc_id", &val);
		sdc_id = val.val;
		Sstar_printk_platform("scan scd_id(%d)\n", sdc_id);
		sw_mci_rescan_card(sdc_id, enabled);
	}
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_FRIENDLY)
	{
		Sstar_sdio_insert(enabled);
	}
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_SUN6I_64)
	{
		int wlan_bus_index = sunxi_wlan_get_bus_index();
		if (wlan_bus_index < 0)
			return wlan_bus_index;
		if (enabled) {
			sunxi_mmc_rescan_card(wlan_bus_index);
		} else {
		}

		//oob_irq = sunxi_wlan_get_oob_irq();
	}
#endif
#if (SSTAR_WIFI_PLATFORM == 10)
	mdelay(100);
	rockchip_wifi_set_carddetect(enabled);
#endif
#endif
	Sstar_printk_platform("[%s]:platform insert ctrl [%d]\n", platform,
			      enabled);
	return ret;
}

int Sstar_power_ctrl(const struct Sstar_platform_data *pdata, bool enabled)
{
	return Sstar_platform_power_ctrl(pdata, enabled);
}
int Sstar_insert_crtl(const struct Sstar_platform_data *pdata, bool enabled)
{
	return Sstar_platform_insert_crtl(pdata, enabled);
}
#ifdef SDIO_BUS
#ifdef CONFIG_SSTAR_APOLLO_USE_GPIO_IRQ
extern irqreturn_t Sstar_gpio_hardirq(int irq, void *dev_id);

extern irqreturn_t Sstar_gpio_irq(int irq, void *dev_id);

int Sstar_plat_request_gpio_irq(const struct Sstar_platform_data *pdata,
				struct sbus_priv *self, int *Sstar_bgf_irq)
{
	int bgf_irq = 0;
	int ret = 0;

#if (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_905)
	bgf_irq = Sstar_wlan_get_oob_irq();
	Sstar_printk_platform("Sstar_plat_request_gpio_irq \n");
	/* Request the IRQ */
	ret = request_threaded_irq(bgf_irq, Sstar_gpio_hardirq, Sstar_gpio_irq,
				   IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE |
					   IORESOURCE_IRQ_SHAREABLE,
				   "Sstar_wlan_irq", self);

	if (WARN_ON(ret))
		goto err;

#elif (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_S805)
	bgf_irq = pdata->irq_gpio;
	Sstar_printk_platform("Sstar_plat_request_gpio_irq \n");
#define IRQ_THREAD_REQ
#ifdef IRQ_THREAD_REQ
	/* Request the IRQ */
	ret = request_threaded_irq(bgf_irq, Sstar_gpio_irq, NULL,
				   IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL |
					   IORESOURCE_IRQ_SHAREABLE,
				   "Sstar_wlan_irq", self);
	if (WARN_ON(ret))
		goto err;
#else //IRQ_THREAD_REQ

	ret = request_irq(bgf_irq, (void *)Sstar_gpio_irq,
			  IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE |
				  IORESOURCE_IRQ_SHAREABLE,
			  "Sstar_wlan_irq", self);
	if (WARN_ON(ret))
		goto err;

#endif //IRQ_THREAD_REQ

#else //(SSTAR_WIFI_PLATFORM == other)

	if (gpio_is_valid(pdata->irq_gpio)) {
		ret = gpio_request(pdata->irq_gpio, "apollo wifi BGF EINT");
		if (ret) {
			Sstar_printk_platform(
				"BGF_EINT gpio request fail, ret = %d\n", ret);
		}
	} else {
		Sstar_printk_platform("invalid BGF_EINT gpio: %d,self %p\n",
				      pdata->irq_gpio, self);
	}
	bgf_irq = gpio_to_irq(pdata->irq_gpio);
	ret = enable_irq_wake(bgf_irq);
	if (WARN_ON(ret))
		goto err;
	/* Request the IRQ */
	ret = request_threaded_irq(
		bgf_irq, Sstar_gpio_hardirq, Sstar_gpio_irq,
		IRQF_TRIGGER_RISING |
			IRQF_ONESHOT, /* IRQF_TRIGGER_HIGH | IRQF_ONESHOT,*/
		"Sstar_wlan_irq", self);
	if (WARN_ON(ret))
		goto err;
#endif

	*Sstar_bgf_irq = bgf_irq;

err:

	return ret;
}

void Sstar_plat_free_gpio_irq(const struct Sstar_platform_data *pdata,
			      struct sbus_priv *self, int Sstar_bgf_irq)
{
#if (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_905)
	disable_irq(Sstar_bgf_irq);
	free_irq(Sstar_bgf_irq, self);
#elif (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_S805)
	//do nothing
	disable_irq(Sstar_bgf_irq);
	free_irq(Sstar_bgf_irq, self);
#else
	disable_irq_wake(Sstar_bgf_irq);
	free_irq(Sstar_bgf_irq, self);
	gpio_free(pdata->irq_gpio);
#endif
	return;
}

#endif //CONFIG_SSTAR_APOLLO_USE_GPIO_IRQ
#endif
#endif
struct Sstar_platform_data platform_data = {
#ifdef SDIO_BUS
	.mmc_id = CONFIG_SSTAR_SDIO_MMC_ID,
	.clk_ctrl = NULL,
	.power_ctrl = Sstar_power_ctrl,
	.insert_ctrl = Sstar_insert_crtl,
#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT31)
	.power_ctrl = NULL,
	//.irq_gpio = EXYNOS4_GPX2(4),
	//.power_gpio	= EXYNOS4_GPC1(1),
	.irq_gpio = 60,
	.power_gpio = GPIO_PC(12),
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_INGENICT41)
	.power_ctrl = NULL,
	//.irq_gpio	= EXYNOS4_GPX2(4),
	//.power_gpio	= EXYNOS4_GPC1(1),
	.irq_gpio = 60,
	.power_gpio = GPIO_PC(12),
#endif

#if (SSTAR_WIFI_PLATFORM == PLATFORM_XUNWEI)
	.irq_gpio = EXYNOS4_GPX2(4),
	.power_gpio = EXYNOS4_GPC1(1),
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_S805)
	.irq_gpio = INT_GPIO_4,
	.power_gpio = 0,
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_AMLOGIC_905)
	.irq_gpio = 100,
	.power_gpio = 0,
#endif
#if (SSTAR_WIFI_PLATFORM == PLATFORM_FRIENDLY)
	.power_gpio = EXYNOS4_GPK3(2),
#endif
	.reset_gpio = 0,
#else
	.clk_ctrl = NULL,
	.power_ctrl = NULL,
	.insert_ctrl = NULL,
#endif
};

struct Sstar_platform_data *Sstar_get_platform_data(void)
{
	return &platform_data;
}
