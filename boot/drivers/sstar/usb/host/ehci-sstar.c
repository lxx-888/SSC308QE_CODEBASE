/*
 * ehci-sstar.c - Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#include <common.h>
#include <clk.h>
#include <log.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/ofnode.h>
#include <generic-phy.h>
#include <reset.h>
//#include <asm/io.h>
#include <dm.h>
#include "../../../usb/host/ehci.h"
#include <power/regulator.h>

#include "inc/platform.h"
#include "inc/common.h"
#include "inc/ehci-sstar.h"
#include <linux/delay.h>

/*
 * Even though here we don't explicitly use "struct ehci_ctrl"
 * ehci_register() expects it to be the first thing that resides in
 * device's private data.
 */

struct sstar_ehci
{
    struct ehci_ctrl ctrl;
    struct phy       phy;
    struct udevice * dev;
#ifdef sstar_pm_vbus
    struct udevice *vbus_supply;
#endif
    uintptr_t utmi_base; //__iomem * or fdt_addr_t
    uintptr_t ehc_base;
    uintptr_t usbc_base;
    uintptr_t bc_base;
    uintptr_t init_flag;
    u32       port_index;
};

static void usb_bc_enable(unsigned long utmi_base, unsigned long bc_base, u8 enable)
{
    if (!bc_base)
        return;

    if (enable)
    {
        // printf("BC enable \n");
        writeb(readb((void *)(utmi_base + (0x1 * 2 - 1))) | 0x40,
               (void *)(utmi_base + (0x1 * 2 - 1))); // IREF_PDN=1��b1. (utmi+0x01[6] )
        writeb(readb((void *)(bc_base + (0x3 * 2 - 1))) | 0x40,
               (void *)(bc_base + (0x3 * 2 - 1))); // [6]= reg_host_bc_en
        writeb(readb((void *)(bc_base + (0xc * 2))) | 0x40,
               (void *)(bc_base + (0xc * 2)));           // [6]= reg_into_host_bc_sw_tri
        writeb(0x00, (void *)(bc_base));                 // [15:0] = bc_ctl_ov_en
        writeb(0x00, (void *)(bc_base + (0x1 * 2 - 1))); // [15:0] = bc_ctl_ov_en
        writeb(readb((void *)(bc_base + (0xa * 2))) | 0x80, (void *)(bc_base + (0xa * 2))); // [7]=reg_bc_switch_en
    }
    else
    {
        // disable BC
        // printf("BC disable \n");
        writeb(readb((void *)(bc_base + (0xc * 2))) & (~0x40),
               (void *)(bc_base + (0xc * 2))); // [6]= reg_into_host_bc_sw_tri
        writeb(readb((void *)(bc_base + (0x3 * 2 - 1))) & (~0x40),
               (void *)(bc_base + (0x3 * 2 - 1))); // [6]= reg_host_bc_en
        writeb(readb((void *)(utmi_base + (0x1 * 2 - 1))) & (~0x40),
               (void *)(utmi_base + (0x1 * 2 - 1))); // IREF_PDN=1��b1. (utmi+0x01[6] )
    }
}

#if defined(ENABLE_USB_NEW_MIU_SEL)
void MIU_select_setting_ehc(unsigned long USBC_base)
{
    printf("[USB] config miu select [%x] [%x] [%x] ][%x]\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2, USB_MIU_SEL3);
    writeb(USB_MIU_SEL0, (void *)(USBC_base + 0x14 * 2));     // Setting MIU0 segment
    writeb(USB_MIU_SEL1, (void *)(USBC_base + 0x16 * 2));     // Setting MIU1 segment
    writeb(USB_MIU_SEL2, (void *)(USBC_base + 0x17 * 2 - 1)); // Setting MIU2 segment
    writeb(USB_MIU_SEL3, (void *)(USBC_base + 0x18 * 2));     // Setting MIU3 segment
    writeb(readb((void *)(USBC_base + 0x19 * 2 - 1)) | 0x01,
           (void *)(USBC_base + 0x19 * 2 - 1)); // Enable miu partition mechanism
#if !defined(DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO)
    writeb(readb((void *)(USBC_base + 0x0F * 2 - 1)) | 0x01, (void *)(USBC_base + 0x0F * 2 - 1));
#endif
}
#endif

int sstar_ehci_setup_phy(struct sstar_ehci *hcd)
{
    struct ehci_phy_priv_data *priv_data = (struct ehci_phy_priv_data *)dev_get_driver_data(hcd->dev);
    uintptr_t                  utmi2_base;

    printf("[USB] ehci_setup_phy ++\n");

    if (priv_data && priv_data->has_utmi2_bank)
    {
        utmi2_base = dev_read_addr_index(hcd->dev, 4);
        printf("[USB] has utmi2 bank[utmi2: %08lx]\n", utmi2_base);
        writew(0x7f05, (void *)utmi2_base);
    }

    writew(0x0001, (void *)hcd->utmi_base);
#if defined(ENABLE_USB_NEW_MIU_SEL)
    MIU_select_setting_ehc(hcd->usbc_base);
#endif
    writew(0x0C2F, (void *)(hcd->utmi_base + 0x8 * 2));
#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
    /*
     * patch for DM always keep high issue
     * init overwrite register
     */
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) & (u8)(~BIT3), (void *)(hcd->utmi_base + 0x0 * 2)); // DP_PUEN = 0
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) & (u8)(~BIT4), (void *)(hcd->utmi_base + 0x0 * 2)); // DM_PUEN = 0

    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) & (u8)(~BIT5), (void *)(hcd->utmi_base + 0x0 * 2)); // R_PUMODE = 0

    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) | BIT6, (void *)(hcd->utmi_base + 0x0 * 2)); // R_DP_PDEN = 1
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) | BIT7, (void *)(hcd->utmi_base + 0x0 * 2)); // R_DM_PDEN = 1

    writeb(readb((void *)(hcd->utmi_base + 0x10 * 2)) | BIT6,
           (void *)(hcd->utmi_base + 0x10 * 2)); // hs_txser_en_cb = 1
    writeb(readb((void *)(hcd->utmi_base + 0x10 * 2)) & (u8)(~BIT7),
           (void *)(hcd->utmi_base + 0x10 * 2)); // hs_se0_cb = 0

    /* turn on overwrite mode */
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) | BIT1, (void *)(hcd->utmi_base + 0x0 * 2)); // tern_ov = 1
    /* new HW term overwrite: on */
    writeb(readb((void *)(hcd->utmi_base + 0x52 * 2)) | (BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
           (void *)(hcd->utmi_base + 0x52 * 2));
#endif

    /* Turn on overwirte mode for D+/D- floating issue when UHC reset
     * Before UHC reset, R_DP_PDEN = 1, R_DM_PDEN = 1, tern_ov = 1 */
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) | (BIT7 | BIT6 | BIT1), (void *)(hcd->utmi_base + 0x0 * 2));
    /* new HW term overwrite: on */
    writeb(readb(hcd->utmi_base + 0x52 * 2) | (BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0), hcd->utmi_base + 0x52 * 2);

#ifdef ENABLE_DOUBLE_DATARATE_SETTING
    writeb(readb((void *)(hcd->utmi_base + 0x0D * 2 - 1)) | BIT0,
           (void *)(hcd->utmi_base + 0x0D * 2 - 1)); // set reg_double_data_rate, To get better jitter performance
#endif

#ifdef ENABLE_UPLL_SETTING
    // sync code from eCos
    {
        u16 reg_t;

        reg_t = readw((void *)(hcd->utmi_base + 0x22 * 2));
        if ((reg_t & 0x10e0) != 0x10e0)
            writew(0x10e0, (void *)(hcd->utmi_base + 0x22 * 2));
        reg_t = readw((void *)(hcd->utmi_base + 0x24 * 2));
        if (reg_t != 0x1)
            writew(0x1, (void *)(hcd->utmi_base + 0x24 * 2));
    }

#endif

    writeb(0x0A, hcd->usbc_base); /* Disable MAC initial suspend, Reset UHC */
    writeb(0x28, hcd->usbc_base); /* Release UHC reset, enable UHC XIU function */

    /* Init UTMI squelch level setting before CA */
    if (UTMI_DISCON_LEVEL_2A & (BIT3 | BIT2 | BIT1 | BIT0))
    {
        writeb((UTMI_DISCON_LEVEL_2A & (BIT3 | BIT2 | BIT1 | BIT0)), (void *)(hcd->utmi_base + 0x2a * 2));
        printf("[USB] init squelch level 0x%x\n", readb((void *)(hcd->utmi_base + 0x2a * 2)));
    }

    writeb(readb(hcd->utmi_base + 0x3C * 2) | 0x01, hcd->utmi_base + 0x3C * 2); /* set CA_START as 1 */

    mdelay(1); // 10->1

    writeb(readb(hcd->utmi_base + 0x3C * 2) & ~0x01, hcd->utmi_base + 0x3C * 2); /* release CA_START */
#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
    writeb(readb((void *)hcd->utmi_base + 0x10 * 2) | 0x40,
           (void *)(hcd->utmi_base + 0x10 * 2)); // bit<6> for monkey test and HS current
#endif
    while (((unsigned int)(readb(hcd->utmi_base + 0x3C * 2)) & 0x02) == 0)
        ; /* polling bit <1> (CA_END) */

    if ((0xFFF0 == (readw((void *)(hcd->utmi_base + 0x3C * 2)) & 0xFFF0))
        || (0x0000 == (readw((void *)(hcd->utmi_base + 0x3C * 2)) & 0xFFF0)))
        printf("WARNING: CA Fail !! \n");

    if (hcd->init_flag & EHCFLAG_DPDM_SWAP)
        writeb(readb(hcd->utmi_base + 0x0b * 2 - 1) | 0x20, hcd->utmi_base + 0x0b * 2 - 1); /* dp dm swap */

    writeb(readb(hcd->usbc_base + 0x02 * 2) & ~0x03, hcd->usbc_base + 0x02 * 2); /* UHC select enable */
    writeb(readb(hcd->usbc_base + 0x02 * 2) | 0x01, hcd->usbc_base + 0x02 * 2);  /* UHC select enable */

    writeb(readb(hcd->ehc_base + 0x40 * 2) & ~0x10, hcd->ehc_base + 0x40 * 2); /* 0: VBUS On. */
    udelay(1);                                                                 /* delay 1us */
    writeb(readb(hcd->ehc_base + 0x40 * 2) | 0x08, hcd->ehc_base + 0x40 * 2);  /* Active HIGH */

    /* Turn on overwirte mode for D+/D- floating issue when UHC reset
     * After UHC reset, disable overwrite bits */
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) & (u8)(~(BIT7 | BIT6 | BIT1)), (void *)(hcd->utmi_base + 0x0 * 2));
    /* new HW term overwrite: off */
    writeb(readb(hcd->utmi_base + 0x52 * 2) & ~(BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0), hcd->utmi_base + 0x52 * 2);

    /* improve the efficiency of USB access MIU when system is busy */
    writeb(readb((void *)(hcd->ehc_base + 0x81 * 2 - 1)) | (BIT0 | BIT1 | BIT2 | BIT3 | BIT7),
           (void *)(hcd->ehc_base + 0x81 * 2 - 1));

    writeb((readb(hcd->utmi_base + 0x06 * 2) & 0x9F) | 0x40,
           hcd->utmi_base + 0x06 * 2); /* reg_tx_force_hs_current_enable */

    writeb(readb(hcd->utmi_base + 0x03 * 2 - 1) | 0x28, hcd->utmi_base + 0x03 * 2 - 1); /* Disconnect window select */
    writeb(readb(hcd->utmi_base + 0x03 * 2 - 1) & 0xef, hcd->utmi_base + 0x03 * 2 - 1); /* Disconnect window select */
    writeb(readb(hcd->utmi_base + 0x07 * 2 - 1) & 0xfd, hcd->utmi_base + 0x07 * 2 - 1); /* Disable improved CDR */

#if defined(ENABLE_UTMI_240_AS_120_PHASE_ECO)
#if defined(UTMI_240_AS_120_PHASE_ECO_INV)
    writeb(readb((void *)hcd->utmi_base + 0x08 * 2) & ~0x08,
           (void *)(hcd->utmi_base + 0x08 * 2)); // bit<3> special for Eiffel analog LIB issue
#elif defined(UTMI_240_AS_120_PHASE_ECO_INV_IF_REV_0)
    if (MDrv_SYS_GetChipRev() == 0x0)
    {
        writeb(readb((void *)hcd->utmi_base + 0x08 * 2) & ~0x08,
               (void *)(hcd->utmi_base + 0x08 * 2)); // bit<3> special for Eiffel analog LIB issue
    }
#else
    writeb(readb(hcd->utmi_base + 0x08 * 2) | 0x08,
           hcd->utmi_base + 0x08 * 2); /* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for
                                          faraday 1 for etron */
#endif
#endif

    writeb(readb(hcd->utmi_base + 0x09 * 2 - 1) | 0x81,
           hcd->utmi_base + 0x09 * 2 - 1); /* UTMI RX anti-dead-loc, ISI effect improvement */
#if defined(UTMI_TX_TIMING_SEL_LATCH_PATH_INV_IF_REV_0)
    if (MDrv_SYS_GetChipRev() == 0x0)
    {
        writeb(readb(hcd->utmi_base + 0x0b * 2 - 1) & ~0x80,
               hcd->utmi_base + 0x0b * 2 - 1); /* TX timing select latch path */
    }
#else
    writeb(readb(hcd->utmi_base + 0x0b * 2 - 1) | 0x80,
           hcd->utmi_base + 0x0b * 2 - 1); /* TX timing select latch path */
#endif
    writeb(readb(hcd->utmi_base + 0x15 * 2 - 1) | 0x20, hcd->utmi_base + 0x15 * 2 - 1); /* Chirp signal source select */
#if defined(ENABLE_UTMI_55_INTERFACE)
    writeb(readb(hcd->utmi_base + 0x15 * 2 - 1) | 0x40, hcd->utmi_base + 0x15 * 2 - 1); /* change to 55 interface */
#endif

    /* new HW chrip design, defualt overwrite to reg_2A */
    writeb(readb((void *)(hcd->utmi_base + 0x40 * 2)) & (u8)(~BIT4), (void *)(hcd->utmi_base + 0x40 * 2));

    /* Init UTMI disconnect level setting */
    writeb(UTMI_DISCON_LEVEL_2A, (void *)(hcd->utmi_base + 0x2a * 2));

#if defined(ENABLE_NEW_HW_CHRIP_PATCH)
    /* Init chrip detect level setting */
    writeb(UTMI_CHIRP_DCT_LEVEL_42, (void *)(hcd->utmi_base + 0x42 * 2));
    /* enable HW control chrip/disconnect level */
    writeb(readb((void *)(hcd->utmi_base + 0x40 * 2)) & (u8)(~BIT3), (void *)(hcd->utmi_base + 0x40 * 2));
#endif

    /* Init UTMI eye diagram parameter setting */
    writeb(readb((void *)(hcd->utmi_base + 0x2c * 2)) | UTMI_EYE_SETTING_2C, (void *)(hcd->utmi_base + 0x2c * 2));
    writeb(readb((void *)(hcd->utmi_base + 0x2d * 2 - 1)) | UTMI_EYE_SETTING_2D,
           (void *)(hcd->utmi_base + 0x2d * 2 - 1));
    writeb(readb((void *)(hcd->utmi_base + 0x2e * 2)) | UTMI_EYE_SETTING_2E, (void *)(hcd->utmi_base + 0x2e * 2));
    writeb(readb((void *)(hcd->utmi_base + 0x2f * 2 - 1)) | UTMI_EYE_SETTING_2F,
           (void *)(hcd->utmi_base + 0x2f * 2 - 1));

#if defined(ENABLE_LS_CROSS_POINT_ECO)
    /* Enable deglitch SE0 (low-speed cross point) */
    writeb(readb((void *)(hcd->utmi_base + LS_CROSS_POINT_ECO_OFFSET)) | LS_CROSS_POINT_ECO_BITSET,
           (void *)(hcd->utmi_base + LS_CROSS_POINT_ECO_OFFSET));
#endif

#if defined(ENABLE_PWR_NOISE_ECO)
    /* Enable use eof2 to reset state machine (power noise) */
    writeb(readb((void *)(hcd->usbc_base + 0x02 * 2)) | BIT6, (void *)(hcd->usbc_base + 0x02 * 2));
#endif

#if defined(ENABLE_TX_RX_RESET_CLK_GATING_ECO)
    /* Enable hw auto deassert sw reset(tx/rx reset) */
    writeb(readb((void *)(hcd->utmi_base + TX_RX_RESET_CLK_GATING_ECO_OFFSET)) | TX_RX_RESET_CLK_GATING_ECO_BITSET,
           (void *)(hcd->utmi_base + TX_RX_RESET_CLK_GATING_ECO_OFFSET));
#endif

#if defined(ENABLE_LOSS_SHORT_PACKET_INTR_ECO)
    /* Enable patch for the assertion of interrupt(Lose short packet interrupt) */
#if defined(LOSS_SHORT_PACKET_INTR_ECO_OPOR)
    writeb(readb((void *)(hcd->usbc_base + LOSS_SHORT_PACKET_INTR_ECO_OFFSET)) | LOSS_SHORT_PACKET_INTR_ECO_BITSET,
           (void *)(hcd->usbc_base + LOSS_SHORT_PACKET_INTR_ECO_OFFSET));
#else
    writeb(readb((void *)(hcd->usbc_base + 0x04 * 2)) & (u8)(~BIT7), (void *)(hcd->usbc_base + 0x04 * 2));
#endif
#endif

#if defined(ENABLE_BABBLE_ECO)
    /* Enable add patch to Period_EOF1(babble problem) */
    writeb(readb((void *)(hcd->usbc_base + 0x04 * 2)) | BIT6, (void *)(hcd->usbc_base + 0x04 * 2));
#endif

#if defined(ENABLE_MDATA_ECO)
    /* Enable short packet MDATA in Split transaction clears ACT bit (LS dev under a HS hub) */
    writeb(readb((void *)(hcd->usbc_base + MDATA_ECO_OFFSET)) | MDATA_ECO_BITSET,
           (void *)(hcd->usbc_base + MDATA_ECO_OFFSET));
#endif

#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
    /* Change override to hs_txser_en.  Dm always keep high issue */
    writeb(readb((void *)(hcd->utmi_base + 0x10 * 2)) | BIT6, (void *)(hcd->utmi_base + 0x10 * 2));
#endif

#if defined(ENABLE_HS_CONNECTION_FAIL_INTO_VFALL_ECO)
    /* HS connection fail problem (Gate into VFALL state) */
    writeb(readb((void *)(hcd->usbc_base + 0x11 * 2 - 1)) | BIT1, (void *)(hcd->usbc_base + 0x11 * 2 - 1));
#endif

#if _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
    /*
     * patch for DM always keep high issue
     * init overwrite register
     */
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) | BIT6, (void *)(hcd->utmi_base + 0x0 * 2)); // R_DP_PDEN = 1
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) | BIT7, (void *)(hcd->utmi_base + 0x0 * 2)); // R_DM_PDEN = 1

    /* turn on overwrite mode */
    writeb(readb((void *)(hcd->utmi_base + 0x0 * 2)) | BIT1, (void *)(hcd->utmi_base + 0x0 * 2)); // tern_ov = 1
    /* new HW term overwrite: on */
    writeb(readb((void *)(hcd->utmi_base + 0x52 * 2)) | (BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
           (void *)(hcd->utmi_base + 0x52 * 2));
#endif

#if defined(ENABLE_PV2MI_BRIDGE_ECO)
    writeb(readb((hcd->usbc_base + 0x0a * 2)) | 0x40, hcd->usbc_base + 0x0a * 2);
#endif

#if _USB_ANALOG_RX_SQUELCH_PATCH
    /* squelch level adjust by calibration value */
    {
        unsigned int ca_da_ov, ca_db_ov, ca_tmp;

        ca_tmp   = readw((void *)(hcd->utmi_base + 0x3c * 2));
        ca_da_ov = (((ca_tmp >> 4) & 0x3f) - 5) + 0x40;
        ca_db_ov = (((ca_tmp >> 10) & 0x3f) - 5) + 0x40;
        printf("[%x]-5 -> (ca_da_ov, ca_db_ov)=(%x,%x)\n", ca_tmp, ca_da_ov, ca_db_ov);
        writeb(ca_da_ov, (void *)(hcd->utmi_base + 0x3B * 2 - 1));
        writeb(ca_db_ov, (void *)(hcd->utmi_base + 0x24 * 2));
    }
#endif

#if _USB_MINI_PV2MI_BURST_SIZE
    writeb(readb((void *)(hcd->usbc_base + 0x0b * 2 - 1)) & ~(BIT1 | BIT2 | BIT3 | BIT4),
           (void *)(hcd->usbc_base + 0x0b * 2 - 1));
#endif

#if defined(ENABLE_UHC_PREAMBLE_ECO)
    /* [7]: reg_etron_en, to enable utmi Preamble function */
    writeb(readb((void *)(hcd->utmi_base + 0x3f * 2 - 1)) | BIT7, (void *)(hcd->utmi_base + 0x3f * 2 - 1));

    /* [3:]: reg_preamble_en, to enable Faraday Preamble */
    writeb(readb((void *)(hcd->usbc_base + 0x0f * 2 - 1)) | BIT3, (void *)(hcd->usbc_base + 0x0f * 2 - 1));

    /* [0]: reg_preamble_babble_fix, to patch Babble occurs in Preamble */
    writeb(readb((void *)(hcd->usbc_base + 0x10 * 2)) | BIT0, (void *)(hcd->usbc_base + 0x10 * 2));

    /* [1]: reg_preamble_fs_within_pre_en, to patch FS crash problem */
    writeb(readb((void *)(hcd->usbc_base + 0x10 * 2)) | BIT1, (void *)(hcd->usbc_base + 0x10 * 2));

    /* [2]: reg_fl_sel_override, to override utmi to have FS drive strength */
    writeb(readb((void *)(hcd->utmi_base + 0x03 * 2 - 1)) | BIT2, (void *)(hcd->utmi_base + 0x03 * 2 - 1));
#endif

#if defined(ENABLE_UHC_RUN_BIT_ALWAYS_ON_ECO)
    /* Don't close RUN bit when device disconnect */
    writeb(readb((void *)(hcd->ehc_base + 0x34 * 2)) | BIT7, (void *)(hcd->ehc_base + 0x34 * 2));
#endif

#if _USB_MIU_WRITE_WAIT_LAST_DONE_Z_PATCH
    /* Enabe PVCI i_miwcplt wait for mi2uh_last_done_z */
    writeb(readb((void *)(hcd->ehc_base + 0x83 * 2 - 1)) | BIT4, (void *)(hcd->ehc_base + 0x83 * 2 - 1));
#endif

#if defined(ENABLE_UHC_EXTRA_HS_SOF_ECO)
    /* Extra HS SOF after bus reset */
    writeb(readb((void *)(hcd->ehc_base + 0x8C * 2)) | BIT0, (void *)(hcd->ehc_base + 0x8C * 2));
#endif

    /* Enable HS ISO IN Camera Cornor case ECO function */
#if defined(HS_ISO_IN_ECO_OFFSET)
    writeb(readb((void *)(hcd->usbc_base + HS_ISO_IN_ECO_OFFSET)) | HS_ISO_IN_ECO_BITSET,
           (void *)(hcd->usbc_base + HS_ISO_IN_ECO_OFFSET));
#else
    writeb(readb((void *)(hcd->usbc_base + 0x13 * 2 - 1)) | BIT0, (void *)(hcd->usbc_base + 0x13 * 2 - 1));
#endif

#if defined(ENABLE_DISCONNECT_SPEED_REPORT_RESET_ECO)
    /* UHC speed type report should be reset by device disconnection */
    writeb(readb((void *)(hcd->usbc_base + 0x20 * 2)) | BIT0, (void *)(hcd->usbc_base + 0x20 * 2));
#endif

#if defined(ENABLE_BABBLE_PCD_ONE_PULSE_TRIGGER_ECO)
    /* Port Change Detect (PCD) is triggered by babble.
     * Pulse trigger will not hang this condition.
     */
    writeb(readb((void *)(hcd->usbc_base + 0x20 * 2)) | BIT1, (void *)(hcd->usbc_base + 0x20 * 2));
#endif

#if defined(ENABLE_HC_RESET_FAIL_ECO)
    /* generation of hhc_reset_u */
    writeb(readb((void *)(hcd->usbc_base + 0x20 * 2)) | BIT2, (void *)(hcd->usbc_base + 0x20 * 2));
#endif

#if defined(ENABLE_INT_AFTER_WRITE_DMA_ECO)
    /* DMA interrupt after the write back of qTD */
    writeb(readb((void *)(hcd->usbc_base + 0x20 * 2)) | BIT3, (void *)(hcd->usbc_base + 0x20 * 2));
#endif

#if defined(ENABLE_DISCONNECT_HC_KEEP_RUNNING_ECO)
    /* EHCI keeps running when device is disconnected */
    writeb(readb((void *)(hcd->usbc_base + 0x19 * 2 - 1)) | BIT3, (void *)(hcd->usbc_base + 0x19 * 2 - 1));
#endif

#if !defined(_EHC_SINGLE_SOF_TO_CHK_DISCONN)
    writeb(0x05, (void *)(hcd->usbc_base + 0x03 * 2 - 1)); // Use 2 SOFs to check disconnection
#endif

#if defined(ENABLE_SRAM_CLK_GATING_ECO)
    /* do SRAM clock gating automatically to save power */
    writeb(readb((void *)(hcd->usbc_base + 0x20 * 2)) & (u8)(~BIT4), (void *)(hcd->usbc_base + 0x20 * 2));
#endif

#if defined(ENABLE_INTR_SITD_CS_IN_ZERO_ECO)
    /* Enable interrupt in sitd cs in zero packet */
    writeb(readb((void *)(hcd->usbc_base + 0x11 * 2 - 1)) | BIT7, (void *)(hcd->usbc_base + 0x11 * 2 - 1));
#endif

    usb_bc_enable(hcd->utmi_base, hcd->bc_base, FALSE);

    printf("[USB] ehci_setup_phy --\n");
    return 0;
}

#ifdef sstar_pm_vbus
static int ehci_enable_vbus_supply(struct udevice *dev)
{
    struct generic_ehci *priv = dev_get_priv(dev);
    int                  ret;

    ret = device_get_supply_regulator(dev, "vbus-supply", &priv->vbus_supply);
    if (ret && ret != -ENOENT)
        return ret;

    if (priv->vbus_supply)
    {
        ret = regulator_set_enable(priv->vbus_supply, true);
        if (ret)
        {
            dev_err(dev, "Error enabling VBUS supply\n");
            return ret;
        }
    }
    else
    {
        dev_dbg(dev, "No vbus supply\n");
    }

    return 0;
}

static int ehci_disable_vbus_supply(struct sstar_ehci *priv)
{
    if (priv->vbus_supply)
        return regulator_set_enable(priv->vbus_supply, false);
    else
        return 0;
}
#else
static int ehci_enable_vbus_supply(struct udevice *dev)
{
    return 0;
}

static int ehci_disable_vbus_supply(struct sstar_ehci *priv)
{
    return 0;
}
#endif

int ehci_deinit_utmi(struct sstar_ehci *hcd)
{
    writeb((readb((void *)GET_REG16_ADDR(hcd->utmi_base, 0x02)) & 0x7f), (void *)GET_REG16_ADDR(hcd->utmi_base, 0x02));

    writew(readw((void *)GET_REG16_ADDR(hcd->utmi_base, 0x03)) | BIT0 | BIT1 | BIT8,
           (void *)GET_REG16_ADDR(hcd->utmi_base, 0x03));
    writew(readw((void *)GET_REG16_ADDR(hcd->utmi_base, 0x08)) | BIT12, (void *)GET_REG16_ADDR(hcd->utmi_base, 0x08));
    mdelay(1);
    // clear reset
    writew(readw((void *)GET_REG16_ADDR(hcd->utmi_base, 0x03)) & (~(BIT0 | BIT1 | BIT8)),
           (void *)GET_REG16_ADDR(hcd->utmi_base, 0x03));
    writew(readw((void *)GET_REG16_ADDR(hcd->utmi_base, 0x08)) & (~BIT12),
           (void *)GET_REG16_ADDR(hcd->utmi_base, 0x08));

    printk(" Deinit utmi\n");
    return 0;
}

static int sstar_ehci_usb_probe(struct udevice *dev)
{
    struct sstar_ehci *    priv = dev_get_priv(dev);
    struct ehci_hccr *     hccr;
    struct ehci_hcor *     hcor;
    int                    err = 0;
    struct reset_ctl_bulk *reset;

    reset = devm_reset_bulk_get_optional(dev);
    if (reset)
        reset_deassert_bulk(reset);

    priv->ehc_base  = dev_read_addr_index(dev, 3);
    priv->usbc_base = dev_read_addr_index(dev, 2);
    priv->bc_base   = dev_read_addr_index(dev, 1);
    priv->utmi_base = dev_read_addr_index(dev, 0);
    priv->dev       = dev;

    /*printf("Bank[utmi: %08lx][bc: %08lx][usbc: %08lx][ehc: %08lx]\n", priv->utmi_base, priv->bc_base, priv->usbc_base,
           priv->ehc_base);*/

    err = ehci_enable_vbus_supply(dev);
    if (err)
        goto regulator_err;

    err = sstar_ehci_setup_phy(priv);
    if (err)
        goto regulator_err;

    hccr = (struct ehci_hccr *)(priv->ehc_base);
    hcor = (struct ehci_hcor *)((u32)hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

    err = ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);
    if (err)
        goto phy_err;

    return 0;

phy_err:
    err = ehci_deinit_utmi(priv);
    if (err)
        dev_err(dev, "failed to deinit utmi\n");

regulator_err:
    err = ehci_disable_vbus_supply(priv);
    if (err)
        dev_err(dev, "failed to disable VBUS supply\n");

    return err;
}

static int sstar_ehci_usb_remove(struct udevice *dev)
{
    struct sstar_ehci *priv = dev_get_priv(dev);
    int                ret;

    ret = ehci_deregister(dev);
    if (ret)
        return ret;

    ret = ehci_deinit_utmi(priv);
    if (ret)
        return ret;

    ret = ehci_disable_vbus_supply(priv);
    if (ret)
        return ret;
    return 0;
}

static struct ehci_phy_priv_data m6p_priv_data = {
    .has_utmi2_bank = 1,
};

static struct ehci_phy_priv_data i6f_priv_data = {
    .has_utmi2_bank = 1,
};

static const struct udevice_id sstar_ehci_ids[] = {{.compatible = "sstar,ehci"},
                                                   {
                                                       .compatible = "sstar,mercury6p-ehci",
                                                       .data       = (ulong)&m6p_priv_data,
                                                   },
                                                   {
                                                       .compatible = "sstar,infinity6f-ehci",
                                                       .data       = (ulong)&i6f_priv_data,
                                                   },
                                                   {}};

U_BOOT_DRIVER(sstar_ehci1) = {
    .name      = "sstar_ehci",
    .id        = UCLASS_USB,
    .of_match  = sstar_ehci_ids,
    .probe     = sstar_ehci_usb_probe,
    .remove    = sstar_ehci_usb_remove,
    .ops       = &ehci_usb_ops,
    .priv_auto = sizeof(struct sstar_ehci),
    .flags     = DM_FLAG_ALLOC_PRIV_DMA,
};
