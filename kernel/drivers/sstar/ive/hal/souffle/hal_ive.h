/*
 * hal_ive.h- Sigmastar
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
#ifndef _HAL_IVE_H_
#define _HAL_IVE_H_

//---------------------------------------------------------------------------
// INCLUDE
//---------------------------------------------------------------------------
#include "hal_ive_common.h"
#include "hal_ive_reg.h"
#include "drv_ive_io_st.h"

//---------------------------------------------------------------------------
// MACRO
//---------------------------------------------------------------------------
#define DRV_IVE_NAME            "mstar_ive"
#define IVE_BANK_NUM            (4)
#define IVE_DEFAULT_CLK_RATE    (480000000)
#define IVE_MAX_NON_OD_CLK_FREQ (480000000)

#define IVE_ISR_IRQ_CHECK_MASK     (IVE_HAL_IRQ_MASK_FRAME_DONE | IVE_HAL_IRQ_MASK_CCL_OVERFLOW)
#define IVE_BGB_ISR_IRQ_CHECK_MASK (IVE_HAL_BGB_IRQ_MASK_FRAME_DONE)

#define MAX_W_CCL    (1280)
#define MAX_H_CCL    (720)
#define MAX_W_GMM    (1280)
#define MAX_H_GMM    (720)
#define MAX_W_RESIZE (1920)
#define MAX_H_RESIZE (1080)
#define MAX_W_BGBLUR (7680)
#define MAX_H_BGBLUR (4320)

#define MIN_W_CCL    (16)
#define MIN_H_CCL    (4)
#define MIN_W_GMM    (64)
#define MIN_H_GMM    (64)
#define MIN_W_RESIZE (16)
#define MIN_H_RESIZE (4)
#define MIN_W_BGBLUR (64)
#define MIN_H_BGBLUR (64)

#define GMM_COMPLEXITY_REDUCTION_THR   (3277)
#define GMM_VAR_THR_BG                 (1048576)
#define GMM_INIT_VAR_OF_MODEL_U8C1     (983040)
#define GMM_INIT_VAR_OF_MODEL_U8C3PACK (2949120)

// RegVal=0/1/2/3 means cmd data length is 4/8/16/32x16 bytes(128 bits), so here use shift left to get cmd data length.
#define AXI_CMD_DATA_LENGTH(RegVal)    (1 << (RegVal + 6))
#define AXI_ARLEN_DEFAULT              (0x2)
#define AXI_AWLEN_DEFAULT              (0x2)
#define AXI_AR_MAX_OUTSTANDING_DEFAULT (0x7)
#define AXI_AW_MAX_OUTSTANDING_DEFAULT (0x7)

//---------------------------------------------------------------------------
// STRUCT
//---------------------------------------------------------------------------

typedef enum
{
    IVE_HAL_IRQ_MASK_FRAME_DONE   = 0x00000001, // bit 0, frame done
    IVE_HAL_IRQ_MASK_Y_CNT_HIT_0  = 0x00000002, // bit 1, Y line counter 0 hit
    IVE_HAL_IRQ_MASK_Y_CNT_HIT_1  = 0x00000004, // bit 2, Y line counter 1 hit
    IVE_HAL_IRQ_MASK_Y_CNT_HIT_2  = 0x00000008, // bit 3, Y line counter 2 hit
    IVE_HAL_IRQ_MASK_CCL_OVERFLOW = 0x00000010, // bit 4, CCL label overflow
    IVE_HAL_IRQ_MASK_ALL          = 0xF000001F  // ALL bits
} IVE_HAL_IRQ_MASK;

typedef enum
{
    IVE_HAL_BGB_IRQ_MASK_FRAME_DONE = 0x00000001, // bit 0, frame done
    IVE_HAL_BGB_IRQ_MASK_ALL        = 0xF0000001  // ALL bits
} IVE_HAL_BGB_IRQ_MASK;

typedef struct
{
    ss_phys_addr_t    base_addr0;
    ive_hal_reg_bank0 reg_bank0;

    ss_phys_addr_t    base_addr1;
    ive_hal_reg_bank1 reg_bank1;

    ss_phys_addr_t    base_addr2; // for IVE200.
    ive_hal_reg_bank2 reg_bank2;

    ss_phys_addr_t    base_addr3; // for BGBlur.
    ive_hal_reg_bank3 reg_bank3;
} ive_hal_handle;

//---------------------------------------------------------------------------
// FUNCTION
//---------------------------------------------------------------------------

void ive_hal_init(ive_hal_handle *handle, ss_phys_addr_t *u64BankBase);

void             ive_hal_set_irq_mask(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
void             ive_hal_set_bgblur_irq_mask(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
void             ive_hal_clear_irq(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
void             ive_hal_clear_bgblur_irq(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
IVE_HAL_IRQ_MASK ive_hal_get_irq_check(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
IVE_HAL_IRQ_MASK ive_hal_get_bgblur_irq_check(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
u16              ive_hal_get_irq(ive_hal_handle *handle);
u16              ive_hal_get_bgblur_irq(ive_hal_handle *handle);

void ive_hal_set_operation(ive_hal_handle *handle, IVE_IOC_OP_TYPE op_type);
void ive_hal_set_images(ive_hal_handle *handle, ive_ioc_config *config);
void ive_hal_set_gmm_images(ive_hal_handle *handle, ive_ioc_coeff_gmm *coeff_gmm);
void ive_hal_set_bgblur_images(ive_hal_handle *handle, ive_ioc_config *config);

void ive_hal_set_op_params(ive_hal_handle *pstHandle, ive_ioc_config *pstConfig);
void ive_hal_set_coeff_bgblur(ive_hal_handle *handle, ive_ioc_coeff_bgblur *coeff);
void ive_hal_set_burst(ive_hal_handle *handle);

void ive_hal_start(ive_hal_handle *handle);
void ive_hal_start_bgblur(ive_hal_handle *handle);
void ive_hal_sw_reset(ive_hal_handle *handle);

void ive_hal_reg_dump(ss_phys_addr_t u64BankBase);

void ive_hal_mcm_busy_on_off(ive_hal_handle *handle, u8 u8Enable);
void ive_hal_bgblur_mcm_busy_on_off(ive_hal_handle *handle, u8 u8Enable);
void ive_hal_padding_mode_set(ive_hal_handle *handle, int padding_mode);

IVE_HAL_RET_E ive_hal_get_check_params(IVE_IOC_OP_TYPE eOpType, IVE_CONFIG_WH_CHECK_E *peCheckFlag, u16 *pu16MaxW,
                                       u16 *pu16MaxH, u16 *pu16MinW, u16 *pu16MinH);
#endif // _HAL_IVE_H_
