/*
 * ehci-sstar.h- Sigmastar
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

#ifndef _EHCI_SSTAR_H_
#define _EHCI_SSTAR_H_

#include <linux/io.h>
#include "registers.h"

#define EHCI_SSTAR_VERSION "20230309"

#define USB_MIU_SEL0 ((u8)0x70U)
#define USB_MIU_SEL1 ((u8)0xe8U)
#define USB_MIU_SEL2 ((u8)0xefU)
#define USB_MIU_SEL3 ((u8)0xefU)
#ifdef CONFIG_ARM64
/* MIU0 256M*/
#define MIU0_SIZE          ((unsigned long)0x80000000) // TBD
#define MIU1_PHY_BASE_ADDR ((unsigned long)0x80000000) // TBD
/* MIU1 512M*/
#define MIU1_SIZE ((unsigned long)0x20000000) // TBD
#endif

#if defined(CONFIG_ARM)
#define MIU0_PHY_BASE_ADDR ((unsigned long)0x00000000)
/* MIU0 256M*/
#define MIU0_SIZE          ((unsigned long)0x80000000) // TBD
#define MIU1_PHY_BASE_ADDR ((unsigned long)0x80000000) // TBD
/* MIU1 512M*/
#define MIU1_SIZE ((unsigned long)0x20000000) // TBD
#endif

//------ Hardware ECO enable switch ----------------------------------
//---- 1. cross point
#if 0 // every chip must enable it manually
#else
#define ENABLE_LS_CROSS_POINT_ECO
#define LS_CROSS_POINT_ECO_OFFSET (0x04 * 2)
#define LS_CROSS_POINT_ECO_BITSET BIT6
#endif

//---- 2. power noise
#if 0 // every chip must enable it manually
#else
#define ENABLE_PWR_NOISE_ECO
#endif

//---- 3. tx/rx reset clock gating cause XIU timeout
#if 0 // every chip must enable it manually
#else
#define ENABLE_TX_RX_RESET_CLK_GATING_ECO
#define TX_RX_RESET_CLK_GATING_ECO_OFFSET (0x04 * 2)
#define TX_RX_RESET_CLK_GATING_ECO_BITSET BIT5
#endif

//---- 4. short packet lose interrupt without IOC
#if 0
#define ENABLE_LOSS_SHORT_PACKET_INTR_ECO
#define LOSS_SHORT_PACKET_INTR_ECO_OPOR
#define LOSS_SHORT_PACKET_INTR_ECO_OFFSET (0x04 * 2)
#define LOSS_SHORT_PACKET_INTR_ECO_BITSET BIT7
#endif

//---- 5. babble avoidance
#if 0
#define ENABLE_BABBLE_ECO
#endif

//---- 6. lose packet in MDATA condition
#if 0
#define MDATA_ECO_OFFSET (0x0F * 2 - 1)
#define MDATA_ECO_BITSET BIT4
#endif

//---- 7. change override to hs_txser_en condition (DM always keep high issue)
#if 0
#define ENABLE_HS_DM_KEEP_HIGH_ECO
#endif

//---- 8. fix pv2mi bridge mis-behavior
#if 0 // every chip must enable it manually
#else
#define ENABLE_PV2MI_BRIDGE_ECO
#endif

//---- 9. change to 55 interface
#if 1
#define ENABLE_UTMI_55_INTERFACE
#endif

//---- 10. 240's phase as 120's clock
/* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for faraday 1 for etron */
#define ENABLE_UTMI_240_AS_120_PHASE_ECO

//---- 11. double date rate (480MHz)
//#define ENABLE_DOUBLE_DATARATE_SETTING

//---- 12. UPLL setting, normally it should be done in sboot
//#define ENABLE_UPLL_SETTING

//---- 13. chip top performance tuning
//#define ENABLE_CHIPTOP_PERFORMANCE_SETTING

//---- 14. HS connection fail problem (Gate into VFALL state)
#define ENABLE_HS_CONNECTION_FAIL_INTO_VFALL_ECO

//---- 15. Enable UHC Preamble ECO function
#define ENABLE_UHC_PREAMBLE_ECO

//---- 16. Don't close RUN bit when device disconnect
#define ENABLE_UHC_RUN_BIT_ALWAYS_ON_ECO

//---- 18. Extra HS SOF after bus reset
#define ENABLE_UHC_EXTRA_HS_SOF_ECO

//---- 19. Not yet support MIU lower bound address subtraction ECO (for chips which use ENABLE_USB_NEW_MIU_SLE)
//#define DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO

//---- 20. UHC speed type report should be reset by device disconnection
#define ENABLE_DISCONNECT_SPEED_REPORT_RESET_ECO

//---- 21. Port Change Detect (PCD) is triggered by babble. Pulse trigger will not hang this condition.
/* 1'b0: level trigger
 * 1'b1: one-pulse trigger
 */
#define ENABLE_BABBLE_PCD_ONE_PULSE_TRIGGER_ECO

//---- 22. generation of hhc_reset_u
/* 1'b0: hhc_reset is_u double sync of hhc_reset
 * 1'b1: hhc_reset_u is one-pulse of hhc_reset
 */
#define ENABLE_HC_RESET_FAIL_ECO

//---- 23. EHCI keeps running when device is disconnected
//#define ENABLE_DISCONNECT_HC_KEEP_RUNNING_ECO

//---- 24. Chirp patch use software overwrite value
/* reg_sw_chirp_override_bit set to 0 */
#define DISABLE_NEW_HW_CHRIP_ECO

//--------------------------------------------------------------------

//------ Software patch enable switch --------------------------------
//---- 1. flush MIU pipe
#if 0 // every chip must apply it
#undef _USB_T3_WBTIMEOUT_PATCH
#else
#define _USB_T3_WBTIMEOUT_PATCH
#endif

//---- 2. data structure (qtd ,...) must be 128-byte aligment
#if 0 // every chip must apply it
#undef _USB_128_ALIGMENT
#else
#define _USB_128_ALIGMENT
#endif

//---- 3. tx/rx reset clock gating cause XIU timeout
#if 0
#define _USB_XIU_TIMEOUT_PATCH
#else
#undef _USB_XIU_TIMEOUT_PATCH
#endif

//---- 4. short packet lose interrupt without IOC
#if 0
#define _USB_SHORT_PACKET_LOSE_INT_PATCH
#else
#undef _USB_SHORT_PACKET_LOSE_INT_PATCH
#endif

//---- 5. QH blocking in MDATA condition, split zero-size data
#if 0 // every chip must apply it
#undef _USB_SPLIT_MDATA_BLOCKING_PATCH
#else
#define _USB_SPLIT_MDATA_BLOCKING_PATCH
#endif

//---- 6. DM always keep high issue
#if 1 // if without ECO solution, use SW patch.
#if 0
#define _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
#else
#undef _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
#endif
#else
#undef _USB_HS_CUR_DRIVE_DM_ALLWAYS_HIGH_PATCH
#endif

//---- 7. clear port eanble when device disconnect while bus reset
#if 0 // every chip must apply it, so far
#undef _USB_CLEAR_PORT_ENABLE_AFTER_FAIL_RESET_PATCH
#else
#define _USB_CLEAR_PORT_ENABLE_AFTER_FAIL_RESET_PATCH
#endif

//---- 8. sstar host only supports "Throttle Mode" in split translation
#if 0 // every chip must apply it, so far
#undef _USB_TURN_ON_TT_THROTTLE_MODE_PATCH
#else
#define _USB_TURN_ON_TT_THROTTLE_MODE_PATCH
#endif

//---- 9. lower squelch level to cover weak cable link
#if 0
#define _USB_ANALOG_RX_SQUELCH_PATCH
#else
#undef _USB_ANALOG_RX_SQUELCH_PATCH
#endif

//---- 10. high speed reset chirp patch
#define _USB_HS_CHIRP_PATCH

//---- 11. friendly customer patch
#define _USB_FRIENDLY_CUSTOMER_PATCH

//---- 12. enabe PVCI i_miwcplt wait for mi2uh_last_done_z
#define _USB_MIU_WRITE_WAIT_LAST_DONE_Z_PATCH

//---- 13. enabe VBUS reset when ERR_INT_SOF_RXACTIVE occur
//#define _USB_VBUS_RESET_PATCH

//--------------------------------------------------------------------

//------ Reduce EOF1 to 12us for performance imporvement -------------
#if 1
/* Enlarge EOP1 from 12us to 16us for babble prvention under hub case.
 * However, we keep the "old config name". 20130121
 */
#define ENABLE_12US_EOF1
#endif
//--------------------------------------------------------------------

#if 0
#define _UTMI_PWR_SAV_MODE_ENABLE
#else
#undef _UTMI_PWR_SAV_MODE_ENABLE
#endif

//---- Setting PV2MI bridge read/write burst size to minimum
#define _USB_MINI_PV2MI_BURST_SIZE

//--------------------------------------------------------------------
#define _USB_UTMI_DPDM_SWAP_P0 0
#define _USB_UTMI_DPDM_SWAP_P1 0
#define _USB_UTMI_DPDM_SWAP_P2 0

//------ Titania3_series_start_ehc flag ------------------------------
// Use low word as flag
#define EHCFLAG_NONE            0x0
#define EHCFLAG_DPDM_SWAP       0x1
#define EHCFLAG_TESTPKG         0x2
#define EHCFLAG_DOUBLE_DATARATE 0x4
// Use high word as data
#define EHCFLAG_DDR_MASK 0xF0000000
#define EHCFLAG_DDR_x15  0x10000000 // 480MHz x1.5
#define EHCFLAG_DDR_x18  0x20000000 // 480MHz x1.8
//--------------------------------------------------------------------
// 0x00: 550mv, 0x20: 575, 0x40: 600, 0x60: 625
#define UTMI_DISCON_LEVEL_2A (0x62)
extern int vbus_gpio_of_parse_and_map(struct device_node *node);
//------ UTMI eye diagram parameters ---------------------------------
#if 0
    // for 40nm
#define UTMI_EYE_SETTING_2C (0x98)
#define UTMI_EYE_SETTING_2D (0x02)
#define UTMI_EYE_SETTING_2E (0x10)
#define UTMI_EYE_SETTING_2F (0x01)
#elif 0
// for 40nm after Agate, use 55nm setting7
#define UTMI_EYE_SETTING_2C (0x90)
#define UTMI_EYE_SETTING_2D (0x03)
#define UTMI_EYE_SETTING_2E (0x30)
#define UTMI_EYE_SETTING_2F (0x81)
#elif 0
// for 40nm after Agate, use 55nm setting6
#define UTMI_EYE_SETTING_2C (0x10)
#define UTMI_EYE_SETTING_2D (0x03)
#define UTMI_EYE_SETTING_2E (0x30)
#define UTMI_EYE_SETTING_2F (0x81)
#elif 0
// for 40nm after Agate, use 55nm setting5
#define UTMI_EYE_SETTING_2C (0x90)
#define UTMI_EYE_SETTING_2D (0x02)
#define UTMI_EYE_SETTING_2E (0x30)
#define UTMI_EYE_SETTING_2F (0x81)
#elif 0
// for 40nm after Agate, use 55nm setting4
#define UTMI_EYE_SETTING_2C (0x90)
#define UTMI_EYE_SETTING_2D (0x03)
#define UTMI_EYE_SETTING_2E (0x00)
#define UTMI_EYE_SETTING_2F (0x81)
#elif 0
// for 40nm after Agate, use 55nm setting3
#define UTMI_EYE_SETTING_2C (0x10)
#define UTMI_EYE_SETTING_2D (0x03)
#define UTMI_EYE_SETTING_2E (0x00)
#define UTMI_EYE_SETTING_2F (0x81)
#elif 0
// for 40nm after Agate, use 55nm setting2
#define UTMI_EYE_SETTING_2C (0x90)
#define UTMI_EYE_SETTING_2D (0x02)
#define UTMI_EYE_SETTING_2E (0x00)
#define UTMI_EYE_SETTING_2F (0x81)
#else
// for 40nm after Agate, use 55nm setting1, the default
#define UTMI_EYE_SETTING_2C (0x10)
#define UTMI_EYE_SETTING_2D (0x02)
#define UTMI_EYE_SETTING_2E (0x00)
#define UTMI_EYE_SETTING_2F (0x81)
#endif

#endif /* _EHCI_SSTAR_H_ */
