/*
 * hal_clk.h- Sigmastar
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

#ifndef __HAL_CLK_H__
#define __HAL_CLK_H__

#include "reg_clks.h"

/* fixed rate clock ID */
#define CFR_ID(n) (n)

#define SSTAR_CLK_DUMMY         CFR_ID(0)
#define SSTAR_CLK_XTAL_12M      CFR_ID(1)
#define SSTAR_CLK_XTAL_24M      CFR_ID(2)
#define SSTAR_CLK_MPLL_86M      CFR_ID(3)
#define SSTAR_CLK_GMACPLL       CFR_ID(4)
#define SSTAR_CLK_GMAC_25A      CFR_ID(5)
#define SSTAR_CLK_GMAC_25B      CFR_ID(6)
#define SSTAR_CLK_GMAC0_EXT100M CFR_ID(7)
#define SSTAR_CLK_GMAC1_EXT100M CFR_ID(8)
#define SSTAR_CLK_MPLL_432M     CFR_ID(9)
#define SSTAR_CLK_MPLL_288M     CFR_ID(10)
#define SSTAR_CLK_MPLL_216M     CFR_ID(11)
#define SSTAR_CLK_MPLL_172M     CFR_ID(12)
#define SSTAR_CLK_UPLL_384M     CFR_ID(13)
#define SSTAR_CLK_MPLL_345M     CFR_ID(14)
#define SSTAR_CLK_SPI_SYNTH_PLL CFR_ID(15)
#define SSTAR_CLK_12M_MUX       CFR_ID(16)
#define SSTAR_CLK_24M_MUX       CFR_ID(17)
#define SSTAR_CLK_RTCPLL_CLK    CFR_ID(18)
#define SSTAR_CFR_ID_END        SSTAR_CLK_RTCPLL_CLK

/* fixed factor clock ID */
#define SSTAR_CFF_ID_START (SSTAR_CFR_ID_END + 1)
#define CFF_ID(n)          (SSTAR_CFF_ID_START + n)

#define SSTAR_CLK_XTAL_12M_DIV2    CFF_ID(0)
#define SSTAR_CLK_XTAL_12M_DIV4    CFF_ID(1)
#define SSTAR_CLK_XTAL_12M_DIV8    CFF_ID(2)
#define SSTAR_CLK_XTAL_12M_DIV16   CFF_ID(3)
#define SSTAR_CLK_GMACPLL_DIV2     CFF_ID(4)
#define SSTAR_CLK_GMACPLL_DIV5     CFF_ID(5)
#define SSTAR_CLK_GMACPLL_DIV10    CFF_ID(6)
#define SSTAR_CLK_GMACPLL_DIV100   CFF_ID(7)
#define SSTAR_CLK_MPLL_432M_DIV4   CFF_ID(8)
#define SSTAR_CLK_MPLL_432M_DIV8   CFF_ID(9)
#define SSTAR_CLK_UPLL_384M_DIV8   CFF_ID(10)
#define SSTAR_CLK_MPLL_432M_DIV2   CFF_ID(11)
#define SSTAR_CLK_MPLL_432M_DIV16  CFF_ID(12)
#define SSTAR_CLK_MPLL_345M_DIV2   CFF_ID(13)
#define SSTAR_CLK_MPLL_288M_DIV2   CFF_ID(14)
#define SSTAR_CLK_MPLL_288M_DIV4   CFF_ID(15)
#define SSTAR_CLK_MPLL_288M_DIV8   CFF_ID(16)
#define SSTAR_CLK_12M_MUX_DIV2     CFF_ID(17)
#define SSTAR_CLK_12M_MUX_DIV4     CFF_ID(18)
#define SSTAR_CLK_12M_MUX_DIV8     CFF_ID(19)
#define SSTAR_CLK_RTCPLL_CLK_DIV16 CFF_ID(20)
#define SSTAR_CLK_RTCPLL_CLK_DIV96 CFF_ID(21)
#define SSTAR_CFF_ID_END           SSTAR_CLK_RTCPLL_CLK_DIV96

/* composite clock ID */
#define SSTAR_CCP_ID_START (SSTAR_CFF_ID_END + 1)
#define CCP_ID(n)          (SSTAR_CCP_ID_START + n)

#define SSTAR_CLK_PWM      CCP_ID(0)
#define SSTAR_CLK_MIIC0    CCP_ID(1)
#define SSTAR_CLK_MIIC1    CCP_ID(2)
#define SSTAR_CLK_MIIC2    CCP_ID(3)
#define SSTAR_CLK_MSPI0    CCP_ID(4)
#define SSTAR_CLK_PM_PWM   CCP_ID(5)
#define SSTAR_CLK_PM_MIIC0 CCP_ID(6)
#define SSTAR_CLK_SAR      CCP_ID(7)
#define SSTAR_CLK_WDT      CCP_ID(8)
#endif /* __HAL_CLK_H__ */
