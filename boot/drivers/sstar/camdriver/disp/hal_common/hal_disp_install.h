/*
 * hal_disp_install.h- Sigmastar
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

#ifndef _HAL_DISP_INSTALL_H_
#define _HAL_DISP_INSTALL_H_

#ifdef DISP_SUPPORT_VGA
#define DISP_EXEC_VGA(func) (func)
#define DISP_OP_VGA_CNT     (DISP_SUPPORT_VGA)
#else
#define DISP_EXEC_VGA(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"VGA"))
#define DISP_OP_VGA_CNT     (0)
#endif
#ifdef DISP_SUPPORT_CVBS
#define DISP_EXEC_CVBS(func) (func)
#define DISP_OP_CVBS_CNT     (DISP_SUPPORT_CVBS)
#else
#define DISP_EXEC_CVBS(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"CVBS"))
#define DISP_OP_CVBS_CNT     (0)
#endif
#ifdef DISP_SUPPORT_HDMITX
#define DISP_EXEC_HDMITX(func) (func)
#define DISP_OP_HDMITX_CNT     (DISP_SUPPORT_HDMITX)
#else
#define DISP_EXEC_HDMITX(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"HDMITX"))
#define DISP_OP_HDMITX_CNT     (0)
#endif
#ifdef DISP_SUPPORT_LCD
#define DISP_EXEC_LCD(func) (func)
#define DISP_OP_LCD_CNT     (DISP_SUPPORT_LCD)
#else
#define DISP_EXEC_LCD(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"LCD"))
#define DISP_OP_LCD_CNT     (0)
#endif
#ifdef DISP_SUPPORT_MIPIDSI
#define DISP_EXEC_MIPIDSI(func) (func)
#define DISP_OP_MIPIDSI_CNT     (DISP_SUPPORT_MIPIDSI)
#else
#define DISP_EXEC_MIPIDSI(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"MIPIDSI"))
#define DISP_OP_MIPIDSI_CNT     (0)
#endif
#ifdef DISP_SUPPORT_LVDS
#define DISP_EXEC_LVDS(func) (func)
#define DISP_OP_LVDS_CNT     (DISP_SUPPORT_LVDS)
#else
#define DISP_EXEC_LVDS(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"LVDS"))
#define DISP_OP_LVDS_CNT     (0)
#endif
#ifdef DISP_SUPPORT_LCD
#define DISP_WAIT_NORMAL_CASE(u32Interface) \
    (!(u32Interface & HAL_DISP_INTF_MCU) && !(u32Interface & HAL_DISP_INTF_MCU_NOFLM))
#else
#define DISP_WAIT_NORMAL_CASE(u32Interface) 1
#endif

#ifdef DISP_SUPPORT_FPLL
#define DISP_EXEC_FPLL(func) (func)
#define DISP_OP_FPLL_CNT     (DISP_SUPPORT_FPLL)
#else
#define DISP_EXEC_FPLL(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"FPLL"))
#define DISP_OP_FPLL_CNT     (0)
#endif

#ifdef DISP_SUPPORT_WDMA
#define DISP_EXEC_WDMA(func) (func)
#define DISP_OP_WDMA_CNT     (DISP_SUPPORT_WDMA)
#else
#define DISP_EXEC_WDMA(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"WDMA"))
#define DISP_OP_WDMA_CNT     (0)
#endif

#ifdef DISP_SUPPORT_MOP
#define DISP_EXEC_MOP(func) (func)
#define DISP_IP_MOP_CNT     (DISP_SUPPORT_MOP)
#else
#define DISP_EXEC_MOP(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"MOP"))
#define DISP_IP_MOP_CNT     (0)
#endif
#ifdef DISP_SUPPORT_HVP
#define DISP_EXEC_HVP(func) (func)
#define DISP_IP_HVP_CNT     (DISP_SUPPORT_HVP)
#else
#define DISP_EXEC_HVP(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"HVP"))
#define DISP_IP_HVP_CNT     (0)
#endif
#ifdef DISP_SUPPORT_RDMA
#define DISP_EXEC_RDMA(func) (func)
#define DISP_IP_RDMA_CNT     (DISP_SUPPORT_RDMA)
#else
#define DISP_EXEC_RDMA(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"RDMA"))
#define DISP_IP_RDMA_CNT     (0)
#endif

#define DISP_IP_CNT (DISP_IP_RDMA_CNT + DISP_IP_MOP_CNT)

#ifdef DISP_SUPPORT_COLOR
#define DISP_EXEC_COLOR(func) (func)
#else
#define DISP_EXEC_COLOR(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"COLOR"))
#endif
#ifdef DISP_SUPPORT_GAMMA
#define DISP_EXEC_GAMMA(func) (func)
#else
#define DISP_EXEC_GAMMA(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"GAMMA"))
#endif
#ifdef DISP_SUPPORT_MACE
#define DISP_EXEC_MACE(func) (func)
#else
#define DISP_EXEC_MACE(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"MACE"))
#endif
#ifdef DISP_SUPPORT_HPQ
#define DISP_EXEC_HPQ(func) (func)
#else
#define DISP_EXEC_HPQ(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"HPQ"))
#endif
#ifdef DISP_SUPPORT_PQBIN
#define DISP_EXEC_PQBIN(func) (func)
#else
#define DISP_EXEC_PQBIN(func) (HAL_DISP_OpIfNotSupport(__LINE__, (u8 *)__FILE__, (u8 *)"PQBIN"))
#endif

#endif
