/*
 * hal_pinmux.c- Sigmastar
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
#include <gpio_os.h>
#include <hal_gpio.h>
#include <padmux.h>
#include <gpio.h>

#define BASE_RIU_PA GPIO_IO_ADDRESS(0x1F000000)

#define PMSLEEP_BANK   0x000E00
#define PM_SAR_BANK    0x001400
#define ALBANY1_BANK   0x151500
#define ALBANY2_BANK   0x151600
#define CHIPTOP_BANK   0x101E00
#define PADTOP_BANK    0x103C00
#define PADGPIO_BANK   0x103E00
#define PADGPIO2_BANK  0x110400
#define PM_PADTOP_BANK 0x003F00
#define UTMI0_BANK     0x142100
#define UTMI1_BANK     0x142900
#define TZMISC_BANK    0x11FE00
#define MIPI0_BANK     0x154800
#define MIPI1_BANK     0x154900
#define MIPI2_BANK     0x154A00

#define _GPIO_W_WORD(addr, val)                           \
    {                                                     \
        (*(volatile u16*)(uintptr_t)(addr)) = (u16)(val); \
    }
#define _GPIO_W_WORD_MASK(addr, val, mask)                                                                             \
    {                                                                                                                  \
        (*(volatile u16*)(uintptr_t)(addr)) = ((*(volatile u16*)(uintptr_t)(addr)) & ~(mask)) | ((u16)(val) & (mask)); \
    }
#define _GPIO_R_BYTE(addr)            (*(volatile u8*)(uintptr_t)(addr))
#define _GPIO_R_WORD_MASK(addr, mask) ((*(volatile u16*)(uintptr_t)(addr)) & (mask))

#define GET_BASE_ADDR_BY_BANK(x, y) ((x) + ((y) << 1))
#define _RIUA_8BIT(bank, offset)    GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + (((offset) & ~1) << 1) + ((offset)&1)
#define _RIUA_16BIT(bank, offset)   GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + ((offset) << 2)

// PADMUX MASK MARCO START
#define REG_OUT_RX0_IE_MODE0         0x70
#define REG_OUT_RX0_IE_MODE0_MASK    BIT0
#define REG_OUT_RX0_IE_MODE1         0x70
#define REG_OUT_RX0_IE_MODE1_MASK    BIT1
#define REG_OUT_RX0_IE_MODE2         0x70
#define REG_OUT_RX0_IE_MODE2_MASK    BIT2
#define REG_OUT_RX0_IE_MODE3         0x70
#define REG_OUT_RX0_IE_MODE3_MASK    BIT3
#define REG_OUT_RX0_IE_MODE4         0x70
#define REG_OUT_RX0_IE_MODE4_MASK    BIT4
#define REG_OUT_RX0_IE_MODE5         0x70
#define REG_OUT_RX0_IE_MODE5_MASK    BIT5
#define REG_OUT_RX0_IE_MODE6         0x70
#define REG_OUT_RX0_IE_MODE6_MASK    BIT6
#define REG_OUT_RX0_IE_MODE7         0x70
#define REG_OUT_RX0_IE_MODE7_MASK    BIT7
#define REG_OUT_RX0_IE_MODE8         0x70
#define REG_OUT_RX0_IE_MODE8_MASK    BIT8
#define REG_OUT_RX0_IE_MODE9         0x70
#define REG_OUT_RX0_IE_MODE9_MASK    BIT9
#define REG_OUT_RX0_IE_MODE10        0x70
#define REG_OUT_RX0_IE_MODE10_MASK   BIT10
#define REG_OUT_RX0_IE_MODE11        0x70
#define REG_OUT_RX0_IE_MODE11_MASK   BIT11
#define REG_OUT_RX1_IE_MODE0         0x70
#define REG_OUT_RX1_IE_MODE0_MASK    BIT0
#define REG_OUT_RX1_IE_MODE1         0x70
#define REG_OUT_RX1_IE_MODE1_MASK    BIT1
#define REG_OUT_RX1_IE_MODE2         0x70
#define REG_OUT_RX1_IE_MODE2_MASK    BIT2
#define REG_OUT_RX1_IE_MODE3         0x70
#define REG_OUT_RX1_IE_MODE3_MASK    BIT3
#define REG_OUT_RX1_IE_MODE4         0x70
#define REG_OUT_RX1_IE_MODE4_MASK    BIT4
#define REG_OUT_RX1_IE_MODE5         0x70
#define REG_OUT_RX1_IE_MODE5_MASK    BIT5
#define REG_OUT_RX1_IE_MODE6         0x70
#define REG_OUT_RX1_IE_MODE6_MASK    BIT6
#define REG_OUT_RX1_IE_MODE7         0x70
#define REG_OUT_RX1_IE_MODE7_MASK    BIT7
#define REG_OUT_RX1_IE_MODE8         0x70
#define REG_OUT_RX1_IE_MODE8_MASK    BIT8
#define REG_OUT_RX1_IE_MODE9         0x70
#define REG_OUT_RX1_IE_MODE9_MASK    BIT9
#define REG_OUT_RX2_IE_MODE0         0x70
#define REG_OUT_RX2_IE_MODE0_MASK    BIT0
#define REG_OUT_RX2_IE_MODE1         0x70
#define REG_OUT_RX2_IE_MODE1_MASK    BIT1
#define REG_OUT_RX2_IE_MODE2         0x70
#define REG_OUT_RX2_IE_MODE2_MASK    BIT2
#define REG_OUT_RX2_IE_MODE3         0x70
#define REG_OUT_RX2_IE_MODE3_MASK    BIT3
#define REG_OUT_RX2_IE_MODE4         0x70
#define REG_OUT_RX2_IE_MODE4_MASK    BIT4
#define REG_OUT_RX2_IE_MODE5         0x70
#define REG_OUT_RX2_IE_MODE5_MASK    BIT5
#define REG_EJ_MODE                  0x60
#define REG_EJ_MODE_MASK             BIT7 | BIT8 | BIT9
#define REG_CM4_EJ_MODE              0x58
#define REG_CM4_EJ_MODE_MASK         BIT0 | BIT1
#define REG_PM_TEST_MODE             0x5f
#define REG_PM_TEST_MODE_MASK        BIT0 | BIT1
#define REG_IR_IN_MODE               0x62
#define REG_IR_IN_MODE_MASK          BIT0 | BIT1
#define REG_VBUS_DET_MODE            0x53
#define REG_VBUS_DET_MODE_MASK       BIT4 | BIT5
#define REG_RADAR_20M_MODE           0x5b
#define REG_RADAR_20M_MODE_MASK      BIT1
#define REG_I2CM0_MODE               0x50
#define REG_I2CM0_MODE_MASK          BIT0 | BIT1
#define REG_I2CM1_MODE               0x50
#define REG_I2CM1_MODE_MASK          BIT2 | BIT3
#define REG_PM_SPI0_MODE             0x58
#define REG_PM_SPI0_MODE_MASK        BIT10 | BIT11
#define REG_PM_SPI0_CZ1_MODE         0x58
#define REG_PM_SPI0_CZ1_MODE_MASK    BIT8 | BIT9
#define REG_MSPI1_MODE               0x58
#define REG_MSPI1_MODE_MASK          BIT6 | BIT7
#define REG_MSPI1_CZ1_MODE           0x58
#define REG_MSPI1_CZ1_MODE_MASK      BIT4 | BIT5
#define REG_SSPI0_MODE               0x5a
#define REG_SSPI0_MODE_MASK          BIT8 | BIT9
#define REG_PSPI0_SLAVE_MODE         0x5a
#define REG_PSPI0_SLAVE_MODE_MASK    BIT1
#define REG_PSPI0_VSYNC_MODE         0x5a
#define REG_PSPI0_VSYNC_MODE_MASK    BIT4
#define REG_PM_FUART_MODE            0x59
#define REG_PM_FUART_MODE_MASK       BIT0 | BIT1
#define REG_PM_FUART_2W_MODE         0x59
#define REG_PM_FUART_2W_MODE_MASK    BIT4 | BIT5 | BIT6
#define REG_PM_UART0_MODE            0x59
#define REG_PM_UART0_MODE_MASK       BIT8
#define REG_PM_PWM0_MODE             0x51
#define REG_PM_PWM0_MODE_MASK        BIT0 | BIT1 | BIT2
#define REG_PM_PWM1_MODE             0x51
#define REG_PM_PWM1_MODE_MASK        BIT4 | BIT5 | BIT6
#define REG_PM_DMIC_MODE             0x54
#define REG_PM_DMIC_MODE_MASK        BIT8 | BIT9
#define REG_PIR_DIRLK0_MODE          0x5c
#define REG_PIR_DIRLK0_MODE_MASK     BIT0 | BIT1
#define REG_PIR_DIRLK1_MODE          0x5c
#define REG_PIR_DIRLK1_MODE_MASK     BIT4 | BIT5
#define REG_PIR_DIRLK2_MODE          0x5c
#define REG_PIR_DIRLK2_MODE_MASK     BIT8 | BIT9
#define REG_PIR_DIRLK3_MODE          0x5c
#define REG_PIR_DIRLK3_MODE_MASK     BIT12 | BIT13
#define REG_PIR_SERIAL0_MODE         0x5b
#define REG_PIR_SERIAL0_MODE_MASK    BIT8 | BIT9
#define REG_I2S_MCLK_MODE            0x54
#define REG_I2S_MCLK_MODE_MASK       BIT4 | BIT5
#define REG_I2S_RX_MODE              0x54
#define REG_I2S_RX_MODE_MASK         BIT0 | BIT1
#define REG_INT_O_MODE               0x50
#define REG_INT_O_MODE_MASK          BIT8
#define REG_DVP_MODE                 0x53
#define REG_DVP_MODE_MASK            BIT8
#define REG_SSI_MODE                 0x5e
#define REG_SSI_MODE_MASK            BIT0 | BIT1 | BIT2
#define REG_SSI_CS_MODE              0x5e
#define REG_SSI_CS_MODE_MASK         BIT4 | BIT5
#define REG_SR_RST_MODE              0x5d
#define REG_SR_RST_MODE_MASK         BIT4
#define REG_SR_PD_MODE               0x5d
#define REG_SR_PD_MODE_MASK          BIT8 | BIT9
#define REG_SR_MCLK_MODE             0x5d
#define REG_SR_MCLK_MODE_MASK        BIT0
#define REG_PM_PAD_EXT_MODE0         0x55
#define REG_PM_PAD_EXT_MODE0_MASK    BIT0
#define REG_PM_PAD_EXT_MODE1         0x55
#define REG_PM_PAD_EXT_MODE1_MASK    BIT1
#define REG_PM_PAD_EXT_MODE2         0x55
#define REG_PM_PAD_EXT_MODE2_MASK    BIT2
#define REG_PM_PAD_EXT_MODE3         0x55
#define REG_PM_PAD_EXT_MODE3_MASK    BIT3
#define REG_PM_PAD_EXT_MODE4         0x55
#define REG_PM_PAD_EXT_MODE4_MASK    BIT4
#define REG_PM_PAD_EXT_MODE5         0x55
#define REG_PM_PAD_EXT_MODE5_MASK    BIT5
#define REG_PM_PAD_EXT_MODE6         0x55
#define REG_PM_PAD_EXT_MODE6_MASK    BIT6
#define REG_PM_PAD_EXT_MODE7         0x55
#define REG_PM_PAD_EXT_MODE7_MASK    BIT7
#define REG_PM_PAD_EXT_MODE8         0x55
#define REG_PM_PAD_EXT_MODE8_MASK    BIT8
#define REG_PM_PAD_EXT_MODE9         0x55
#define REG_PM_PAD_EXT_MODE9_MASK    BIT9
#define REG_PM_PAD_EXT_MODE10        0x55
#define REG_PM_PAD_EXT_MODE10_MASK   BIT10
#define REG_PM_PAD_EXT_MODE11        0x55
#define REG_PM_PAD_EXT_MODE11_MASK   BIT11
#define REG_PM_PAD_EXT_MODE12        0x55
#define REG_PM_PAD_EXT_MODE12_MASK   BIT12
#define REG_PM_PAD_EXT_MODE13        0x55
#define REG_PM_PAD_EXT_MODE13_MASK   BIT13
#define REG_PM_PAD_EXT_MODE14        0x55
#define REG_PM_PAD_EXT_MODE14_MASK   BIT14
#define REG_PM_PAD_EXT_MODE15        0x55
#define REG_PM_PAD_EXT_MODE15_MASK   BIT15
#define REG_PM_PAD_EXT_MODE16        0x56
#define REG_PM_PAD_EXT_MODE16_MASK   BIT0
#define REG_PM_PAD_EXT_MODE17        0x56
#define REG_PM_PAD_EXT_MODE17_MASK   BIT1
#define REG_PM_PAD_EXT_MODE18        0x56
#define REG_PM_PAD_EXT_MODE18_MASK   BIT2
#define REG_PM_PAD_EXT_MODE19        0x56
#define REG_PM_PAD_EXT_MODE19_MASK   BIT3
#define REG_PM_PAD_EXT_MODE20        0x56
#define REG_PM_PAD_EXT_MODE20_MASK   BIT4
#define REG_PM_PAD_EXT_MODE21        0x56
#define REG_PM_PAD_EXT_MODE21_MASK   BIT5
#define REG_PM_PAD_EXT_MODE22        0x56
#define REG_PM_PAD_EXT_MODE22_MASK   BIT6
#define REG_PM_PAD_EXT_MODE23        0x56
#define REG_PM_PAD_EXT_MODE23_MASK   BIT7
#define REG_PM_PAD_EXT_MODE24        0x56
#define REG_PM_PAD_EXT_MODE24_MASK   BIT8
#define REG_PM_PAD_EXT_MODE25        0x56
#define REG_PM_PAD_EXT_MODE25_MASK   BIT9
#define REG_PM_PAD_EXT_MODE26        0x56
#define REG_PM_PAD_EXT_MODE26_MASK   BIT10
#define REG_PM_PAD_EXT_MODE27        0x56
#define REG_PM_PAD_EXT_MODE27_MASK   BIT11
#define REG_PM_PAD_EXT_MODE28        0x56
#define REG_PM_PAD_EXT_MODE28_MASK   BIT12
#define REG_PM_PAD_EXT_MODE29        0x56
#define REG_PM_PAD_EXT_MODE29_MASK   BIT13
#define REG_PM_PAD_EXT_MODE30        0x56
#define REG_PM_PAD_EXT_MODE30_MASK   BIT14
#define REG_PM_PAD_EXT_MODE31        0x56
#define REG_PM_PAD_EXT_MODE31_MASK   BIT15
#define REG_PM_PAD_EXT_MODE32        0x57
#define REG_PM_PAD_EXT_MODE32_MASK   BIT0
#define REG_PM_PAD_EXT_MODE33        0x57
#define REG_PM_PAD_EXT_MODE33_MASK   BIT1
#define REG_PM_PAD_EXT_MODE34        0x57
#define REG_PM_PAD_EXT_MODE34_MASK   BIT2
#define REG_PM_PAD_EXT_MODE35        0x57
#define REG_PM_PAD_EXT_MODE35_MASK   BIT3
#define REG_PM_PAD_EXT_MODE36        0x57
#define REG_PM_PAD_EXT_MODE36_MASK   BIT4
#define REG_PM_PAD_EXT_MODE37        0x57
#define REG_PM_PAD_EXT_MODE37_MASK   BIT5
#define REG_PM_PAD_EXT_MODE38        0x57
#define REG_PM_PAD_EXT_MODE38_MASK   BIT6
#define REG_PM_PAD_EXT_MODE39        0x57
#define REG_PM_PAD_EXT_MODE39_MASK   BIT7
#define REG_PM_PAD_EXT_MODE40        0x57
#define REG_PM_PAD_EXT_MODE40_MASK   BIT8
#define REG_PM_PAD_EXT_MODE41        0x57
#define REG_PM_PAD_EXT_MODE41_MASK   BIT9
#define REG_PM_PAD_EXT_MODE42        0x57
#define REG_PM_PAD_EXT_MODE42_MASK   BIT10
#define REG_PM_PAD_EXT_MODE43        0x57
#define REG_PM_PAD_EXT_MODE43_MASK   BIT11
#define REG_PM_PAD_EXT_MODE44        0x57
#define REG_PM_PAD_EXT_MODE44_MASK   BIT12
#define REG_SPI_GPIO                 0x35
#define REG_SPI_GPIO_MASK            BIT0
#define REG_SPIWPN_GPIO              0x35
#define REG_SPIWPN_GPIO_MASK         BIT4
#define REG_SPICSZ1_GPIO             0x35
#define REG_SPICSZ1_GPIO_MASK        BIT2
#define REG_SPICSZ2_MODE             0x66
#define REG_SPICSZ2_MODE_MASK        BIT0 | BIT1
#define REG_SPIHOLDN_MODE            0x52
#define REG_SPIHOLDN_MODE_MASK       BIT0
#define REG_UART_GPIO_EN             0x65
#define REG_UART_GPIO_EN_MASK        BIT4
#define REG_TEST_IN_MODE             0x12
#define REG_TEST_IN_MODE_MASK        BIT0 | BIT1
#define REG_TEST_OUT_MODE            0x12
#define REG_TEST_OUT_MODE_MASK       BIT4 | BIT5
#define REG_I2C0_MODE                0x6f
#define REG_I2C0_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_I2C1_MODE                0x53
#define REG_I2C1_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_I2C2_MODE                0x6f
#define REG_I2C2_MODE_MASK           BIT8 | BIT9 | BIT10
#define REG_I2C3_MODE                0x73
#define REG_I2C3_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_I2C4_MODE                0x10
#define REG_I2C4_MODE_MASK           BIT4 | BIT5 | BIT6 | BIT7
#define REG_I2C5_MODE                0x73
#define REG_I2C5_MODE_MASK           BIT8 | BIT9
#define REG_SPI0_MODE                0x68
#define REG_SPI0_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_SPI0_CZ1_MODE            0x70
#define REG_SPI0_CZ1_MODE_MASK       BIT4 | BIT5
#define REG_SPI1_MODE                0x68
#define REG_SPI1_MODE_MASK           BIT3 | BIT4 | BIT5
#define REG_SPI2_MODE                0x68
#define REG_SPI2_MODE_MASK           BIT8 | BIT9 | BIT10
#define REG_SPI3_MODE                0x47
#define REG_SPI3_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_FUART_MODE               0x6e
#define REG_FUART_MODE_MASK          BIT8 | BIT9 | BIT10
#define REG_FUART_2W_MODE            0x6e
#define REG_FUART_2W_MODE_MASK       BIT12 | BIT13 | BIT14
#define REG_UART0_MODE               0x6d
#define REG_UART0_MODE_MASK          BIT0 | BIT1
#define REG_UART1_MODE               0x6d
#define REG_UART1_MODE_MASK          BIT4 | BIT5 | BIT6
#define REG_UART2_MODE               0x6d
#define REG_UART2_MODE_MASK          BIT8 | BIT9 | BIT10
#define REG_UART3_MODE               0x6d
#define REG_UART3_MODE_MASK          BIT12 | BIT13 | BIT14
#define REG_UART4_MODE               0x49
#define REG_UART4_MODE_MASK          BIT0 | BIT1 | BIT2
#define REG_UART5_MODE               0x49
#define REG_UART5_MODE_MASK          BIT4 | BIT5 | BIT6
#define REG_UART6_MODE               0x49
#define REG_UART6_MODE_MASK          BIT8 | BIT9 | BIT10
#define REG_SD0_BOOT_MODE            0x8
#define REG_SD0_BOOT_MODE_MASK       BIT0
#define REG_SD0_MODE                 0x67
#define REG_SD0_MODE_MASK            BIT8 | BIT9
#define REG_SD0_CDZ_MODE             0x67
#define REG_SD0_CDZ_MODE_MASK        BIT10 | BIT11
#define REG_SD0_RSTN_MODE            0x7a
#define REG_SD0_RSTN_MODE_MASK       BIT0 | BIT1
#define REG_EMMC8B_BOOT_MODE         0x9
#define REG_EMMC8B_BOOT_MODE_MASK    BIT0
#define REG_EMMC4B_BOOT_MODE         0x9
#define REG_EMMC4B_BOOT_MODE_MASK    BIT8
#define REG_EMMC_8B_MODE             0x61
#define REG_EMMC_8B_MODE_MASK        BIT2
#define REG_EMMC_4B_MODE             0x61
#define REG_EMMC_4B_MODE_MASK        BIT0
#define REG_EMMC_RST_MODE            0x61
#define REG_EMMC_RST_MODE_MASK       BIT4
#define REG_EMMC_AS_SD_CDZ_MODE      0x61
#define REG_EMMC_AS_SD_CDZ_MODE_MASK BIT8
#define REG_SDIO_MODE                0x67
#define REG_SDIO_MODE_MASK           BIT12 | BIT13 | BIT14
#define REG_SDIO_CDZ_MODE            0x67
#define REG_SDIO_CDZ_MODE_MASK       BIT0 | BIT1 | BIT2
#define REG_SDIO_RST_MODE            0x67
#define REG_SDIO_RST_MODE_MASK       BIT4 | BIT5 | BIT6
#define REG_PWM0_MODE                0x65
#define REG_PWM0_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_PWM1_MODE                0x65
#define REG_PWM1_MODE_MASK           BIT4 | BIT5 | BIT6
#define REG_PWM2_MODE                0x65
#define REG_PWM2_MODE_MASK           BIT8 | BIT9 | BIT10
#define REG_PWM3_MODE                0x65
#define REG_PWM3_MODE_MASK           BIT12 | BIT13 | BIT14
#define REG_PWM4_MODE                0x7b
#define REG_PWM4_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_PWM5_MODE                0x7b
#define REG_PWM5_MODE_MASK           BIT4 | BIT5 | BIT6
#define REG_PWM6_MODE                0x7b
#define REG_PWM6_MODE_MASK           BIT8 | BIT9 | BIT10
#define REG_PWM7_MODE                0x7b
#define REG_PWM7_MODE_MASK           BIT12 | BIT13 | BIT14
#define REG_PWM8_MODE                0x7c
#define REG_PWM8_MODE_MASK           BIT0 | BIT1 | BIT2
#define REG_PWM9_MODE                0x7c
#define REG_PWM9_MODE_MASK           BIT4 | BIT5 | BIT6
#define REG_PWM10_MODE               0x7c
#define REG_PWM10_MODE_MASK          BIT8 | BIT9 | BIT10
#define REG_PWM11_MODE               0x7c
#define REG_PWM11_MODE_MASK          BIT12 | BIT13 | BIT14
#define REG_LED0_MODE                0x63
#define REG_LED0_MODE_MASK           BIT0 | BIT1
#define REG_LED1_MODE                0x63
#define REG_LED1_MODE_MASK           BIT4 | BIT5
#define REG_VD_MODE                  0x6b
#define REG_VD_MODE_MASK             BIT8 | BIT9 | BIT10
#define REG_I2S0_MCK_MODE            0x62
#define REG_I2S0_MCK_MODE_MASK       BIT0 | BIT1 | BIT2
#define REG_I2S0_RX_MODE             0x62
#define REG_I2S0_RX_MODE_MASK        BIT8 | BIT9 | BIT10
#define REG_I2S0_TX_MODE             0x62
#define REG_I2S0_TX_MODE_MASK        BIT12 | BIT13 | BIT14
#define REG_I2S0_RXTX_MODE           0x4f
#define REG_I2S0_RXTX_MODE_MASK      BIT0 | BIT1 | BIT2
#define REG_DMIC_2CH_MODE            0x60
#define REG_DMIC_2CH_MODE_MASK       BIT0 | BIT1 | BIT2
#define REG_DMIC_4CH_MODE            0x79
#define REG_DMIC_4CH_MODE_MASK       BIT0 | BIT1 | BIT2
#define REG_DMIC_6CH_MODE            0x79
#define REG_DMIC_6CH_MODE_MASK       BIT4 | BIT5 | BIT6
#define REG_SR0_MIPI_MODE            0x69
#define REG_SR0_MIPI_MODE_MASK       BIT0 | BIT1 | BIT2
#define REG_SR1_MIPI_MODE            0x69
#define REG_SR1_MIPI_MODE_MASK       BIT4 | BIT5
#define REG_SR2_MIPI_MODE            0x69
#define REG_SR2_MIPI_MODE_MASK       BIT8
#define REG_SR0_SUB_LVDS_MODE        0x69
#define REG_SR0_SUB_LVDS_MODE_MASK   BIT14 | BIT15
#define REG_SR0_MODE                 0x6b
#define REG_SR0_MODE_MASK            BIT0 | BIT1 | BIT2 | BIT3
#define REG_SR0_INFRARED_MODE        0x48
#define REG_SR0_INFRARED_MODE_MASK   BIT8 | BIT9
#define REG_ISR_MCLK_MODE            0x48
#define REG_ISR_MCLK_MODE_MASK       BIT12 | BIT13
#define REG_SR00_MCLK_MODE           0x6a
#define REG_SR00_MCLK_MODE_MASK      BIT0 | BIT1
#define REG_SR01_MCLK_MODE           0x6a
#define REG_SR01_MCLK_MODE_MASK      BIT2 | BIT3
#define REG_SR1_MCLK_MODE            0x6a
#define REG_SR1_MCLK_MODE_MASK       BIT4 | BIT5
#define REG_SR2_MCLK_MODE            0x6a
#define REG_SR2_MCLK_MODE_MASK       BIT6
#define REG_SR00_RST_MODE            0x54
#define REG_SR00_RST_MODE_MASK       BIT0 | BIT1
#define REG_SR01_RST_MODE            0x54
#define REG_SR01_RST_MODE_MASK       BIT2 | BIT3
#define REG_SR1_RST_MODE             0x54
#define REG_SR1_RST_MODE_MASK        BIT4 | BIT5
#define REG_SR2_RST_MODE             0x54
#define REG_SR2_RST_MODE_MASK        BIT6
#define REG_SR0_PDN_MODE             0x51
#define REG_SR0_PDN_MODE_MASK        BIT8 | BIT9 | BIT10
#define REG_SR1_PDN_MODE             0x51
#define REG_SR1_PDN_MODE_MASK        BIT2 | BIT3
#define REG_SR2_PDN_MODE             0x51
#define REG_SR2_PDN_MODE_MASK        BIT4
#define REG_SR0_HSYNC_MODE           0x52
#define REG_SR0_HSYNC_MODE_MASK      BIT8 | BIT9
#define REG_SR0_VSYNC_MODE           0x52
#define REG_SR0_VSYNC_MODE_MASK      BIT12 | BIT13
#define REG_SR0_PCLK_MODE            0x52
#define REG_SR0_PCLK_MODE_MASK       BIT0 | BIT1 | BIT2
#define REG_SR0_BT656_MODE           0x4a
#define REG_SR0_BT656_MODE_MASK      BIT0
#define REG_SR1_BT656_MODE           0x4a
#define REG_SR1_BT656_MODE_MASK      BIT2 | BIT3
#define REG_SR0_BT1120_MODE          0x48
#define REG_SR0_BT1120_MODE_MASK     BIT0 | BIT1
#define REG_SR_SLAVE_XLK_MODE        0x14
#define REG_SR_SLAVE_XLK_MODE_MASK   BIT0 | BIT1
#define REG_SR0_SLAVE_MODE           0x12
#define REG_SR0_SLAVE_MODE_MASK      BIT12 | BIT13
#define REG_MIPITX0_OUT_MODE         0x44
#define REG_MIPITX0_OUT_MODE_MASK    BIT0 | BIT1 | BIT2
#define REG_BT656_OUT_MODE           0x60
#define REG_BT656_OUT_MODE_MASK      BIT4 | BIT5
#define REG_BT1120_OUT_MODE          0x72
#define REG_BT1120_OUT_MODE_MASK     BIT0
#define REG_TTL24_MODE               0x6c
#define REG_TTL24_MODE_MASK          BIT8
#define REG_TTL18_MODE               0x6c
#define REG_TTL18_MODE_MASK          BIT4
#define REG_TTL16_MODE               0x6c
#define REG_TTL16_MODE_MASK          BIT0 | BIT1
#define REG_RGMII0_MODE              0x41
#define REG_RGMII0_MODE_MASK         BIT0
#define REG_GPHY0_REF_MODE           0x38
#define REG_GPHY0_REF_MODE_MASK      BIT0
#define REG_RMII_MODE                0x41
#define REG_RMII_MODE_MASK           BIT8
#define REG_OTP_TEST                 0x64
#define REG_OTP_TEST_MASK            BIT8
#define REG_QSPI_MODE                0x10
#define REG_QSPI_MODE_MASK           BIT0
#define REG_QSPICSZ2_MODE            0xd
#define REG_QSPICSZ2_MODE_MASK       BIT4
#define REG_SPI_EXT_EN_MODE0         0x39
#define REG_SPI_EXT_EN_MODE0_MASK    BIT0
#define REG_SPI_EXT_EN_MODE1         0x39
#define REG_SPI_EXT_EN_MODE1_MASK    BIT1
#define REG_SPI_EXT_EN_MODE2         0x39
#define REG_SPI_EXT_EN_MODE2_MASK    BIT2
#define REG_SPI_EXT_EN_MODE3         0x39
#define REG_SPI_EXT_EN_MODE3_MASK    BIT3
#define REG_SPI_EXT_EN_MODE4         0x39
#define REG_SPI_EXT_EN_MODE4_MASK    BIT4
#define REG_SPI_EXT_EN_MODE5         0x39
#define REG_SPI_EXT_EN_MODE5_MASK    BIT5
// PADMUX MASK MARCO END

#define REG_PWM_OUT0_GPIO_MODE                0x00
#define REG_PWM_OUT0_GPIO_MODE_MASK           BIT3
#define REG_PWM_OUT1_GPIO_MODE                0x01
#define REG_PWM_OUT1_GPIO_MODE_MASK           BIT3
#define REG_PWM_OUT2_GPIO_MODE                0x02
#define REG_PWM_OUT2_GPIO_MODE_MASK           BIT3
#define REG_PWM_OUT3_GPIO_MODE                0x03
#define REG_PWM_OUT3_GPIO_MODE_MASK           BIT3
#define REG_PWM_OUT4_GPIO_MODE                0x04
#define REG_PWM_OUT4_GPIO_MODE_MASK           BIT3
#define REG_PWM_OUT5_GPIO_MODE                0x05
#define REG_PWM_OUT5_GPIO_MODE_MASK           BIT3
#define REG_PWM_OUT6_GPIO_MODE                0x06
#define REG_PWM_OUT6_GPIO_MODE_MASK           BIT3
#define REG_PWM_OUT7_GPIO_MODE                0x07
#define REG_PWM_OUT7_GPIO_MODE_MASK           BIT3
#define REG_SD0_GPIO0_GPIO_MODE               0x08
#define REG_SD0_GPIO0_GPIO_MODE_MASK          BIT3
#define REG_SD0_VCTRL_GPIO_MODE               0x09
#define REG_SD0_VCTRL_GPIO_MODE_MASK          BIT3
#define REG_SD0_CDZ_GPIO_MODE                 0x0A
#define REG_SD0_CDZ_GPIO_MODE_MASK            BIT3
#define REG_SD0_D1_GPIO_MODE                  0x0B
#define REG_SD0_D1_GPIO_MODE_MASK             BIT3
#define REG_SD0_D0_GPIO_MODE                  0x0C
#define REG_SD0_D0_GPIO_MODE_MASK             BIT3
#define REG_SD0_CLK_GPIO_MODE                 0x0D
#define REG_SD0_CLK_GPIO_MODE_MASK            BIT3
#define REG_SD0_CMD_GPIO_MODE                 0x0E
#define REG_SD0_CMD_GPIO_MODE_MASK            BIT3
#define REG_SD0_D3_GPIO_MODE                  0x0F
#define REG_SD0_D3_GPIO_MODE_MASK             BIT3
#define REG_SD0_D2_GPIO_MODE                  0x10
#define REG_SD0_D2_GPIO_MODE_MASK             BIT3
#define REG_I2C0_SCL_GPIO_MODE                0x11
#define REG_I2C0_SCL_GPIO_MODE_MASK           BIT3
#define REG_I2C0_SDA_GPIO_MODE                0x12
#define REG_I2C0_SDA_GPIO_MODE_MASK           BIT3
#define REG_SR_RST0_GPIO_MODE                 0x13
#define REG_SR_RST0_GPIO_MODE_MASK            BIT3
#define REG_SR_MCLK0_GPIO_MODE                0x14
#define REG_SR_MCLK0_GPIO_MODE_MASK           BIT3
#define REG_I2C1_SCL_GPIO_MODE                0x15
#define REG_I2C1_SCL_GPIO_MODE_MASK           BIT3
#define REG_I2C1_SDA_GPIO_MODE                0x16
#define REG_I2C1_SDA_GPIO_MODE_MASK           BIT3
#define REG_SR_RST1_GPIO_MODE                 0x17
#define REG_SR_RST1_GPIO_MODE_MASK            BIT3
#define REG_SR_MCLK1_GPIO_MODE                0x18
#define REG_SR_MCLK1_GPIO_MODE_MASK           BIT3
#define REG_OUTP_RX0_CH0_GPIO_MODE            0x19
#define REG_OUTP_RX0_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX0_CH0_GPIO_MODE            0x1A
#define REG_OUTN_RX0_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX0_CH1_GPIO_MODE            0x1B
#define REG_OUTP_RX0_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX0_CH1_GPIO_MODE            0x1C
#define REG_OUTN_RX0_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX0_CH2_GPIO_MODE            0x1D
#define REG_OUTP_RX0_CH2_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX0_CH2_GPIO_MODE            0x1E
#define REG_OUTN_RX0_CH2_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX0_CH3_GPIO_MODE            0x1F
#define REG_OUTP_RX0_CH3_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX0_CH3_GPIO_MODE            0x20
#define REG_OUTN_RX0_CH3_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX0_CH4_GPIO_MODE            0x21
#define REG_OUTP_RX0_CH4_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX0_CH4_GPIO_MODE            0x22
#define REG_OUTN_RX0_CH4_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX0_CH5_GPIO_MODE            0x23
#define REG_OUTP_RX0_CH5_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX0_CH5_GPIO_MODE            0x24
#define REG_OUTN_RX0_CH5_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX1_CH0_GPIO_MODE            0x25
#define REG_OUTP_RX1_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX1_CH0_GPIO_MODE            0x26
#define REG_OUTN_RX1_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX1_CH1_GPIO_MODE            0x27
#define REG_OUTP_RX1_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX1_CH1_GPIO_MODE            0x28
#define REG_OUTN_RX1_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX1_CH2_GPIO_MODE            0x29
#define REG_OUTP_RX1_CH2_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX1_CH2_GPIO_MODE            0x2A
#define REG_OUTN_RX1_CH2_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX1_CH3_GPIO_MODE            0x2B
#define REG_OUTP_RX1_CH3_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX1_CH3_GPIO_MODE            0x2C
#define REG_OUTN_RX1_CH3_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX1_CH4_GPIO_MODE            0x2D
#define REG_OUTP_RX1_CH4_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX1_CH4_GPIO_MODE            0x2E
#define REG_OUTN_RX1_CH4_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX2_CH0_GPIO_MODE            0x2F
#define REG_OUTP_RX2_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX2_CH0_GPIO_MODE            0x30
#define REG_OUTN_RX2_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX2_CH1_GPIO_MODE            0x31
#define REG_OUTP_RX2_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX2_CH1_GPIO_MODE            0x32
#define REG_OUTN_RX2_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTP_RX2_CH2_GPIO_MODE            0x33
#define REG_OUTP_RX2_CH2_GPIO_MODE_MASK       BIT3
#define REG_OUTN_RX2_CH2_GPIO_MODE            0x34
#define REG_OUTN_RX2_CH2_GPIO_MODE_MASK       BIT3
#define REG_I2C2_SCL_GPIO_MODE                0x35
#define REG_I2C2_SCL_GPIO_MODE_MASK           BIT3
#define REG_I2C2_SDA_GPIO_MODE                0x36
#define REG_I2C2_SDA_GPIO_MODE_MASK           BIT3
#define REG_SR_RST2_GPIO_MODE                 0x37
#define REG_SR_RST2_GPIO_MODE_MASK            BIT3
#define REG_SR_MCLK2_GPIO_MODE                0x38
#define REG_SR_MCLK2_GPIO_MODE_MASK           BIT3
#define REG_I2C3_SCL_GPIO_MODE                0x39
#define REG_I2C3_SCL_GPIO_MODE_MASK           BIT3
#define REG_I2C3_SDA_GPIO_MODE                0x3A
#define REG_I2C3_SDA_GPIO_MODE_MASK           BIT3
#define REG_SR_RST3_GPIO_MODE                 0x3B
#define REG_SR_RST3_GPIO_MODE_MASK            BIT3
#define REG_SR_MCLK3_GPIO_MODE                0x3C
#define REG_SR_MCLK3_GPIO_MODE_MASK           BIT3
#define REG_ISP0_XVS_GPIO_MODE                0x3D
#define REG_ISP0_XVS_GPIO_MODE_MASK           BIT3
#define REG_ISP0_XHS_GPIO_MODE                0x3E
#define REG_ISP0_XHS_GPIO_MODE_MASK           BIT3
#define REG_ISP0_XTRIG_GPIO_MODE              0x3F
#define REG_ISP0_XTRIG_GPIO_MODE_MASK         BIT3
#define REG_PM_SR1_D0_GPIO_MODE               0x00
#define REG_PM_SR1_D0_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_D4_GPIO_MODE               0x01
#define REG_PM_SR1_D4_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_PCLK_GPIO_MODE             0x02
#define REG_PM_SR1_PCLK_GPIO_MODE_MASK        BIT3
#define REG_PM_SR1_RST_GPIO_MODE              0x03
#define REG_PM_SR1_RST_GPIO_MODE_MASK         BIT3
#define REG_PM_SR1_INT_GPIO_MODE              0x04
#define REG_PM_SR1_INT_GPIO_MODE_MASK         BIT3
#define REG_PM_MI2C1_SCL_GPIO_MODE            0x05
#define REG_PM_MI2C1_SCL_GPIO_MODE_MASK       BIT3
#define REG_PM_MI2C1_SDA_GPIO_MODE            0x06
#define REG_PM_MI2C1_SDA_GPIO_MODE_MASK       BIT3
#define REG_PM_SR1_D1_GPIO_MODE               0x07
#define REG_PM_SR1_D1_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_D2_GPIO_MODE               0x08
#define REG_PM_SR1_D2_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_D3_GPIO_MODE               0x09
#define REG_PM_SR1_D3_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_D5_GPIO_MODE               0x0A
#define REG_PM_SR1_D5_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_D6_GPIO_MODE               0x0B
#define REG_PM_SR1_D6_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_D7_GPIO_MODE               0x0C
#define REG_PM_SR1_D7_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_VS_GPIO_MODE               0x0D
#define REG_PM_SR1_VS_GPIO_MODE_MASK          BIT3
#define REG_PM_SR1_HS_GPIO_MODE               0x0E
#define REG_PM_SR1_HS_GPIO_MODE_MASK          BIT3
#define REG_PM_DMIC0_CLK_GPIO_MODE            0x0F
#define REG_PM_DMIC0_CLK_GPIO_MODE_MASK       BIT3
#define REG_PM_DMIC0_D0_GPIO_MODE             0x10
#define REG_PM_DMIC0_D0_GPIO_MODE_MASK        BIT3
#define REG_PM_SPI_CK_GPIO_MODE               0x11
#define REG_PM_SPI_CK_GPIO_MODE_MASK          BIT3
#define REG_PM_SPI_DI_GPIO_MODE               0x12
#define REG_PM_SPI_DI_GPIO_MODE_MASK          BIT3
#define REG_PM_SPI_HLD_GPIO_MODE              0x13
#define REG_PM_SPI_HLD_GPIO_MODE_MASK         BIT3
#define REG_PM_SPI_DO_GPIO_MODE               0x14
#define REG_PM_SPI_DO_GPIO_MODE_MASK          BIT3
#define REG_PM_SPI_WPZ_GPIO_MODE              0x15
#define REG_PM_SPI_WPZ_GPIO_MODE_MASK         BIT3
#define REG_PM_SPI_CZ_GPIO_MODE               0x16
#define REG_PM_SPI_CZ_GPIO_MODE_MASK          BIT3
#define REG_PM_RADAR_SAR_GPIO0_GPIO_MODE      0x17
#define REG_PM_RADAR_SAR_GPIO0_GPIO_MODE_MASK BIT3
#define REG_PM_RADAR_SAR_GPIO1_GPIO_MODE      0x18
#define REG_PM_RADAR_SAR_GPIO1_GPIO_MODE_MASK BIT3
#define REG_PM_RADAR_SAR_GPIO2_GPIO_MODE      0x19
#define REG_PM_RADAR_SAR_GPIO2_GPIO_MODE_MASK BIT3
#define REG_PM_RADAR_SAR_GPIO3_GPIO_MODE      0x1A
#define REG_PM_RADAR_SAR_GPIO3_GPIO_MODE_MASK BIT3
#define REG_PM_UART_RX_GPIO_MODE              0x1B
#define REG_PM_UART_RX_GPIO_MODE_MASK         BIT3
#define REG_PM_UART_TX_GPIO_MODE              0x1C
#define REG_PM_UART_TX_GPIO_MODE_MASK         BIT3
#define REG_PM_INTOUT_GPIO_MODE               0x1D
#define REG_PM_INTOUT_GPIO_MODE_MASK          BIT3
#define REG_PM_GPIO0_GPIO_MODE                0x1E
#define REG_PM_GPIO0_GPIO_MODE_MASK           BIT3
#define REG_PM_GPIO1_GPIO_MODE                0x1F
#define REG_PM_GPIO1_GPIO_MODE_MASK           BIT3
#define REG_PM_GPIO2_GPIO_MODE                0x20
#define REG_PM_GPIO2_GPIO_MODE_MASK           BIT3
#define REG_PM_GPIO3_GPIO_MODE                0x21
#define REG_PM_GPIO3_GPIO_MODE_MASK           BIT3
#define REG_PM_GPIO4_GPIO_MODE                0x22
#define REG_PM_GPIO4_GPIO_MODE_MASK           BIT3
#define REG_PM_GPIO5_GPIO_MODE                0x23
#define REG_PM_GPIO5_GPIO_MODE_MASK           BIT3
#define REG_PM_FUART_RX_GPIO_MODE             0x24
#define REG_PM_FUART_RX_GPIO_MODE_MASK        BIT3
#define REG_PM_FUART_TX_GPIO_MODE             0x25
#define REG_PM_FUART_TX_GPIO_MODE_MASK        BIT3
#define REG_PM_IRIN_GPIO_MODE                 0x26
#define REG_PM_IRIN_GPIO_MODE_MASK            BIT3
#define REG_PM_MSPI0_DO_GPIO_MODE             0x27
#define REG_PM_MSPI0_DO_GPIO_MODE_MASK        BIT3
#define REG_PM_MSPI0_DI_GPIO_MODE             0x28
#define REG_PM_MSPI0_DI_GPIO_MODE_MASK        BIT3
#define REG_PM_MSPI0_CK_GPIO_MODE             0x29
#define REG_PM_MSPI0_CK_GPIO_MODE_MASK        BIT3
#define REG_PM_MSPI0_CZ_GPIO_MODE             0x2A
#define REG_PM_MSPI0_CZ_GPIO_MODE_MASK        BIT3
#define REG_PM_USB3_INT_GPIO_MODE             0x2B
#define REG_PM_USB3_INT_GPIO_MODE_MASK        BIT3
#define REG_PM_USB3_ID_GPIO_MODE              0x2C
#define REG_PM_USB3_ID_GPIO_MODE_MASK         BIT3
#define REG_SAR_GPIO0_GPIO_MODE               0x11
#define REG_SAR_GPIO0_GPIO_MODE_MASK          BIT0
#define REG_SAR_GPIO1_GPIO_MODE               0x11
#define REG_SAR_GPIO1_GPIO_MODE_MASK          BIT1
#define REG_RGMII0_MCLK_GPIO_MODE             0x6D
#define REG_RGMII0_MCLK_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_RSTN_GPIO_MODE             0x6E
#define REG_RGMII0_RSTN_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_RXCLK_GPIO_MODE            0x6F
#define REG_RGMII0_RXCLK_GPIO_MODE_MASK       BIT3
#define REG_RGMII0_RXCTL_GPIO_MODE            0x70
#define REG_RGMII0_RXCTL_GPIO_MODE_MASK       BIT3
#define REG_RGMII0_RXD0_GPIO_MODE             0x71
#define REG_RGMII0_RXD0_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_RXD1_GPIO_MODE             0x72
#define REG_RGMII0_RXD1_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_RXD2_GPIO_MODE             0x73
#define REG_RGMII0_RXD2_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_RXD3_GPIO_MODE             0x74
#define REG_RGMII0_RXD3_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_TXCLK_GPIO_MODE            0x75
#define REG_RGMII0_TXCLK_GPIO_MODE_MASK       BIT3
#define REG_RGMII0_TXCTL_GPIO_MODE            0x76
#define REG_RGMII0_TXCTL_GPIO_MODE_MASK       BIT3
#define REG_RGMII0_TXD0_GPIO_MODE             0x77
#define REG_RGMII0_TXD0_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_TXD1_GPIO_MODE             0x78
#define REG_RGMII0_TXD1_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_TXD2_GPIO_MODE             0x79
#define REG_RGMII0_TXD2_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_TXD3_GPIO_MODE             0x7A
#define REG_RGMII0_TXD3_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_MDIO_GPIO_MODE             0x7B
#define REG_RGMII0_MDIO_GPIO_MODE_MASK        BIT3
#define REG_RGMII0_MDC_GPIO_MODE              0x7C
#define REG_RGMII0_MDC_GPIO_MODE_MASK         BIT3
#define REG_SD1_GPIO0_GPIO_MODE               0x7D
#define REG_SD1_GPIO0_GPIO_MODE_MASK          BIT3
#define REG_SD1_GPIO1_GPIO_MODE               0x7E
#define REG_SD1_GPIO1_GPIO_MODE_MASK          BIT3
#define REG_SD1_CDZ_GPIO_MODE                 0x7F
#define REG_SD1_CDZ_GPIO_MODE_MASK            BIT3
#define REG_SD1_D1_GPIO_MODE                  0x00
#define REG_SD1_D1_GPIO_MODE_MASK             BIT3
#define REG_SD1_D0_GPIO_MODE                  0x01
#define REG_SD1_D0_GPIO_MODE_MASK             BIT3
#define REG_SD1_CLK_GPIO_MODE                 0x02
#define REG_SD1_CLK_GPIO_MODE_MASK            BIT3
#define REG_SD1_CMD_GPIO_MODE                 0x03
#define REG_SD1_CMD_GPIO_MODE_MASK            BIT3
#define REG_SD1_D3_GPIO_MODE                  0x04
#define REG_SD1_D3_GPIO_MODE_MASK             BIT3
#define REG_SD1_D2_GPIO_MODE                  0x05
#define REG_SD1_D2_GPIO_MODE_MASK             BIT3
#define REG_OUTP_TX0_CH0_GPIO_MODE            0x06
#define REG_OUTP_TX0_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTN_TX0_CH0_GPIO_MODE            0x07
#define REG_OUTN_TX0_CH0_GPIO_MODE_MASK       BIT3
#define REG_OUTP_TX0_CH1_GPIO_MODE            0x08
#define REG_OUTP_TX0_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTN_TX0_CH1_GPIO_MODE            0x09
#define REG_OUTN_TX0_CH1_GPIO_MODE_MASK       BIT3
#define REG_OUTP_TX0_CH2_GPIO_MODE            0x0A
#define REG_OUTP_TX0_CH2_GPIO_MODE_MASK       BIT3
#define REG_OUTN_TX0_CH2_GPIO_MODE            0x0B
#define REG_OUTN_TX0_CH2_GPIO_MODE_MASK       BIT3
#define REG_OUTP_TX0_CH3_GPIO_MODE            0x0C
#define REG_OUTP_TX0_CH3_GPIO_MODE_MASK       BIT3
#define REG_OUTN_TX0_CH3_GPIO_MODE            0x0D
#define REG_OUTN_TX0_CH3_GPIO_MODE_MASK       BIT3
#define REG_OUTP_TX0_CH4_GPIO_MODE            0x0E
#define REG_OUTP_TX0_CH4_GPIO_MODE_MASK       BIT3
#define REG_OUTN_TX0_CH4_GPIO_MODE            0x0F
#define REG_OUTN_TX0_CH4_GPIO_MODE_MASK       BIT3
#define REG_EMMC_RSTN_GPIO_MODE               0x10
#define REG_EMMC_RSTN_GPIO_MODE_MASK          BIT3
#define REG_EMMC_CLK_GPIO_MODE                0x11
#define REG_EMMC_CLK_GPIO_MODE_MASK           BIT3
#define REG_EMMC_CMD_GPIO_MODE                0x12
#define REG_EMMC_CMD_GPIO_MODE_MASK           BIT3
#define REG_EMMC_DS_GPIO_MODE                 0x13
#define REG_EMMC_DS_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D3_GPIO_MODE                 0x14
#define REG_EMMC_D3_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D4_GPIO_MODE                 0x15
#define REG_EMMC_D4_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D0_GPIO_MODE                 0x16
#define REG_EMMC_D0_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D5_GPIO_MODE                 0x17
#define REG_EMMC_D5_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D1_GPIO_MODE                 0x18
#define REG_EMMC_D1_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D6_GPIO_MODE                 0x19
#define REG_EMMC_D6_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D2_GPIO_MODE                 0x1A
#define REG_EMMC_D2_GPIO_MODE_MASK            BIT3
#define REG_EMMC_D7_GPIO_MODE                 0x1B
#define REG_EMMC_D7_GPIO_MODE_MASK            BIT3
#define REG_ETH_LED0_GPIO_MODE                0x1C
#define REG_ETH_LED0_GPIO_MODE_MASK           BIT3
#define REG_ETH_LED1_GPIO_MODE                0x1D
#define REG_ETH_LED1_GPIO_MODE_MASK           BIT3
#define REG_I2C4_SCL_GPIO_MODE                0x1E
#define REG_I2C4_SCL_GPIO_MODE_MASK           BIT3
#define REG_I2C4_SDA_GPIO_MODE                0x1F
#define REG_I2C4_SDA_GPIO_MODE_MASK           BIT3
#define REG_SPI_CK_GPIO_MODE                  0x20
#define REG_SPI_CK_GPIO_MODE_MASK             BIT3
#define REG_SPI_DI_GPIO_MODE                  0x21
#define REG_SPI_DI_GPIO_MODE_MASK             BIT3
#define REG_SPI_HLD_GPIO_MODE                 0x22
#define REG_SPI_HLD_GPIO_MODE_MASK            BIT3
#define REG_SPI_DO_GPIO_MODE                  0x23
#define REG_SPI_DO_GPIO_MODE_MASK             BIT3
#define REG_SPI_WPZ_GPIO_MODE                 0x24
#define REG_SPI_WPZ_GPIO_MODE_MASK            BIT3
#define REG_SPI_CZ_GPIO_MODE                  0x25
#define REG_SPI_CZ_GPIO_MODE_MASK             BIT3
#define REG_UART_RX_GPIO_MODE                 0x26
#define REG_UART_RX_GPIO_MODE_MASK            BIT3
#define REG_UART_TX_GPIO_MODE                 0x27
#define REG_UART_TX_GPIO_MODE_MASK            BIT3
/* please put GPIO_GEN marco to here end */

/* for misc pad which must be added by hand start */
#define REG_GMII_SEL_MODE 0x0c
#define REG_GMII_SEL_MASK BIT0
/* for misc pad which must be added by hand end */

#define ENABLE_CHECK_ALL_PAD_CONFLICT 1

typedef struct hal_gpio_st_padmux
{
    u16 pad_id;
    u32 base;
    u16 offset;
    u16 mask;
    u16 val;
    u16 mode;
} hal_gpio_st_padmux_info;

typedef struct hal_gpio_st_padmode
{
    u8  pad_name[32];
    u64 mode_riu;
    u16 mode_mask;
    u16 mode_val;
} hal_gpio_st_padmode_info;

typedef struct hal_gpio_st_pad_check
{
    u16 base;
    u16 offset;
    u16 mask;
    u16 val;
    u16 regval;
} hal_gpio_st_pad_check_info;

typedef struct hal_gpio_st_pad_checkVal
{
    u8                           infocount;
    struct hal_gpio_st_pad_check infos[64];
} hal_gpio_st_pad_check_v;

typedef struct hal_gpio_st_padmux_entry
{
    u32                            size;
    const hal_gpio_st_padmux_info* padmux;
} hal_gpio_st_padmux_en;

static s32 pad_mode_recoder[GPIO_NR] = {[0 ... GPIO_NR - 1] = PINMUX_FOR_UNKNOWN_MODE};

hal_gpio_st_pad_check_v m_hal_gpio_st_pad_checkVal;

const hal_gpio_st_padmux_info pwm_out0_tbl[] = {
    {PAD_PWM_OUT0, PADGPIO_BANK, REG_PWM_OUT0_GPIO_MODE, REG_PWM_OUT0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_I2C5_MODE, REG_I2C5_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_I2C5_MODE_3},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT2, PINMUX_FOR_SPI0_MODE_4},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT9, PINMUX_FOR_SD0_MODE_2},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_SDIO_MODE_5},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_PWM0_MODE, REG_PWM0_MODE_MASK, BIT0, PINMUX_FOR_PWM0_MODE_1},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT8, PINMUX_FOR_I2S0_RX_MODE_1},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT0, PINMUX_FOR_I2S0_RXTX_MODE_1},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_PWM_OUT0, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info pwm_out1_tbl[] = {
    {PAD_PWM_OUT1, PADGPIO_BANK, REG_PWM_OUT1_GPIO_MODE, REG_PWM_OUT1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_I2C5_MODE, REG_I2C5_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_I2C5_MODE_3},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT2, PINMUX_FOR_SPI0_MODE_4},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT9, PINMUX_FOR_SD0_MODE_2},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_SDIO_MODE_5},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_PWM1_MODE, REG_PWM1_MODE_MASK, BIT4, PINMUX_FOR_PWM1_MODE_1},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT8, PINMUX_FOR_I2S0_RX_MODE_1},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT0, PINMUX_FOR_I2S0_RXTX_MODE_1},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_PWM_OUT1, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info pwm_out2_tbl[] = {
    {PAD_PWM_OUT2, PADGPIO_BANK, REG_PWM_OUT2_GPIO_MODE, REG_PWM_OUT2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT2, PINMUX_FOR_SPI0_MODE_4},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT9, PINMUX_FOR_SD0_MODE_2},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_SDIO_MODE_5},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_PWM2_MODE, REG_PWM2_MODE_MASK, BIT8, PINMUX_FOR_PWM2_MODE_1},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT8, PINMUX_FOR_I2S0_RX_MODE_1},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT0, PINMUX_FOR_I2S0_RXTX_MODE_1},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_PWM_OUT2, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info pwm_out3_tbl[] = {
    {PAD_PWM_OUT3, PADGPIO_BANK, REG_PWM_OUT3_GPIO_MODE, REG_PWM_OUT3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT2, PINMUX_FOR_SPI0_MODE_4},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT2, PINMUX_FOR_UART4_MODE_4},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT9, PINMUX_FOR_SD0_MODE_2},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_SDIO_MODE_5},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_PWM3_MODE, REG_PWM3_MODE_MASK, BIT12, PINMUX_FOR_PWM3_MODE_1},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT12, PINMUX_FOR_I2S0_TX_MODE_1},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT0, PINMUX_FOR_I2S0_RXTX_MODE_1},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_PWM_OUT3, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
};
const hal_gpio_st_padmux_info pwm_out4_tbl[] = {
    {PAD_PWM_OUT4, PADGPIO_BANK, REG_PWM_OUT4_GPIO_MODE, REG_PWM_OUT4_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT4, PADTOP_BANK, REG_SPI0_CZ1_MODE, REG_SPI0_CZ1_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SPI0_CZ1_MODE_3},
    {PAD_PWM_OUT4, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT2, PINMUX_FOR_UART4_MODE_4},
    {PAD_PWM_OUT4, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT9, PINMUX_FOR_SD0_MODE_2},
    {PAD_PWM_OUT4, PADTOP_BANK, REG_PWM4_MODE, REG_PWM4_MODE_MASK, BIT0, PINMUX_FOR_PWM4_MODE_1},
    {PAD_PWM_OUT4, PADTOP_BANK, REG_I2S0_MCK_MODE, REG_I2S0_MCK_MODE_MASK, BIT0, PINMUX_FOR_I2S0_MCK_MODE_1},
    {PAD_PWM_OUT4, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
};
const hal_gpio_st_padmux_info pwm_out5_tbl[] = {
    {PAD_PWM_OUT5, PADGPIO_BANK, REG_PWM_OUT5_GPIO_MODE, REG_PWM_OUT5_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT5, PADTOP_BANK, REG_PWM5_MODE, REG_PWM5_MODE_MASK, BIT4, PINMUX_FOR_PWM5_MODE_1},
    {PAD_PWM_OUT5, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT10, PINMUX_FOR_VD_MODE_4},
    {PAD_PWM_OUT5, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT2, PINMUX_FOR_DMIC_2CH_MODE_4},
    {PAD_PWM_OUT5, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT2, PINMUX_FOR_DMIC_4CH_MODE_4},
    {PAD_PWM_OUT5, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_DMIC_4CH_MODE_5},
    {PAD_PWM_OUT5, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6, PINMUX_FOR_DMIC_6CH_MODE_4},
    {PAD_PWM_OUT5, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_5},
};
const hal_gpio_st_padmux_info pwm_out6_tbl[] = {
    {PAD_PWM_OUT6, PADGPIO_BANK, REG_PWM_OUT6_GPIO_MODE, REG_PWM_OUT6_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2C0_MODE_3},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT6, PINMUX_FOR_UART5_MODE_4},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_PWM6_MODE, REG_PWM6_MODE_MASK, BIT8, PINMUX_FOR_PWM6_MODE_1},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_LED0_MODE, REG_LED0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_LED0_MODE_3},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT10, PINMUX_FOR_VD_MODE_4},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT2, PINMUX_FOR_DMIC_2CH_MODE_4},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT2, PINMUX_FOR_DMIC_4CH_MODE_4},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_DMIC_4CH_MODE_5},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6, PINMUX_FOR_DMIC_6CH_MODE_4},
    {PAD_PWM_OUT6, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_5},
};
const hal_gpio_st_padmux_info pwm_out7_tbl[] = {
    {PAD_PWM_OUT7, PADGPIO_BANK, REG_PWM_OUT7_GPIO_MODE, REG_PWM_OUT7_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PWM_OUT7, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2C0_MODE_3},
    {PAD_PWM_OUT7, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT6, PINMUX_FOR_UART5_MODE_4},
    {PAD_PWM_OUT7, PADTOP_BANK, REG_PWM7_MODE, REG_PWM7_MODE_MASK, BIT12, PINMUX_FOR_PWM7_MODE_1},
    {PAD_PWM_OUT7, PADTOP_BANK, REG_LED1_MODE, REG_LED1_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_LED1_MODE_3},
    {PAD_PWM_OUT7, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT2, PINMUX_FOR_DMIC_4CH_MODE_4},
    {PAD_PWM_OUT7, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6, PINMUX_FOR_DMIC_6CH_MODE_4},
    {PAD_PWM_OUT7, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_5},
};
const hal_gpio_st_padmux_info sd0_gpio0_tbl[] = {
    {PAD_SD0_GPIO0, PADGPIO_BANK, REG_SD0_GPIO0_GPIO_MODE, REG_SD0_GPIO0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_GPIO0, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_GPIO0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_GPIO0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_GPIO0, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_GPIO0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd0_vctrl_tbl[] = {
    {PAD_SD0_VCTRL, PADGPIO_BANK, REG_SD0_VCTRL_GPIO_MODE, REG_SD0_VCTRL_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_VCTRL, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_VCTRL, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_VCTRL, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_VCTRL, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_VCTRL, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd0_cdz_tbl[] = {
    {PAD_SD0_CDZ, PADGPIO_BANK, REG_SD0_CDZ_GPIO_MODE, REG_SD0_CDZ_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_CDZ, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_CDZ, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_CDZ, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_CDZ, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_CDZ, PADTOP_BANK, REG_SD0_CDZ_MODE, REG_SD0_CDZ_MODE_MASK, BIT10, PINMUX_FOR_SD0_CDZ_MODE_1},
    {PAD_SD0_CDZ, PADTOP_BANK, REG_SD0_RSTN_MODE, REG_SD0_RSTN_MODE_MASK, BIT0, PINMUX_FOR_SD0_RSTN_MODE_1},
    {PAD_SD0_CDZ, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd0_d1_tbl[] = {
    {PAD_SD0_D1, PADGPIO_BANK, REG_SD0_D1_GPIO_MODE, REG_SD0_D1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D1, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_D1, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_D1, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_D1, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_D1, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT8, PINMUX_FOR_SD0_MODE_1},
    {PAD_SD0_D1, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd0_d0_tbl[] = {
    {PAD_SD0_D0, PADGPIO_BANK, REG_SD0_D0_GPIO_MODE, REG_SD0_D0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D0, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_D0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_D0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_D0, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT3, PINMUX_FOR_SPI1_MODE_1},
    {PAD_SD0_D0, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_D0, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT8, PINMUX_FOR_SD0_MODE_1},
    {PAD_SD0_D0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd0_clk_tbl[] = {
    {PAD_SD0_CLK, PADGPIO_BANK, REG_SD0_CLK_GPIO_MODE, REG_SD0_CLK_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_CLK, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_CLK, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_CLK, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_CLK, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_I2C2_MODE_3},
    {PAD_SD0_CLK, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT3, PINMUX_FOR_SPI1_MODE_1},
    {PAD_SD0_CLK, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_CLK, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT8, PINMUX_FOR_SD0_MODE_1},
    {PAD_SD0_CLK, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd0_cmd_tbl[] = {
    {PAD_SD0_CMD, PADGPIO_BANK, REG_SD0_CMD_GPIO_MODE, REG_SD0_CMD_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_CMD, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_CMD, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_CMD, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_CMD, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_I2C2_MODE_3},
    {PAD_SD0_CMD, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT3, PINMUX_FOR_SPI1_MODE_1},
    {PAD_SD0_CMD, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_CMD, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT8, PINMUX_FOR_SD0_MODE_1},
};
const hal_gpio_st_padmux_info sd0_d3_tbl[] = {
    {PAD_SD0_D3, PADGPIO_BANK, REG_SD0_D3_GPIO_MODE, REG_SD0_D3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D3, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_D3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_D3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_D3, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT3, PINMUX_FOR_SPI1_MODE_1},
    {PAD_SD0_D3, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_D3, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT8, PINMUX_FOR_SD0_MODE_1},
};
const hal_gpio_st_padmux_info sd0_d2_tbl[] = {
    {PAD_SD0_D2, PADGPIO_BANK, REG_SD0_D2_GPIO_MODE, REG_SD0_D2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D2, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SD0_D2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SD0_D2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SD0_D2, PADTOP_BANK, REG_SD0_BOOT_MODE, REG_SD0_BOOT_MODE_MASK, BIT0, PINMUX_FOR_SD0_BOOT_MODE_1},
    {PAD_SD0_D2, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT8, PINMUX_FOR_SD0_MODE_1},
};
const hal_gpio_st_padmux_info i2c0_scl_tbl[] = {
    {PAD_I2C0_SCL, PADGPIO_BANK, REG_I2C0_SCL_GPIO_MODE, REG_I2C0_SCL_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT0, PINMUX_FOR_I2C0_MODE_1},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2, PINMUX_FOR_I2C1_MODE_4},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT10 | BIT9, PINMUX_FOR_I2C2_MODE_6},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT8, PINMUX_FOR_SPI2_MODE_1},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info i2c0_sda_tbl[] = {
    {PAD_I2C0_SDA, PADGPIO_BANK, REG_I2C0_SDA_GPIO_MODE, REG_I2C0_SDA_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT0, PINMUX_FOR_I2C0_MODE_1},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2, PINMUX_FOR_I2C1_MODE_4},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT10 | BIT9, PINMUX_FOR_I2C2_MODE_6},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT8, PINMUX_FOR_SPI2_MODE_1},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sr_rst0_tbl[] = {
    {PAD_SR_RST0, PADGPIO_BANK, REG_SR_RST0_GPIO_MODE, REG_SR_RST0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_RST0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_SR_RST0, PADTOP_BANK, REG_SR00_RST_MODE, REG_SR00_RST_MODE_MASK, BIT0, PINMUX_FOR_SR00_RST_MODE_1},
    {PAD_SR_RST0, PADTOP_BANK, REG_SR1_RST_MODE, REG_SR1_RST_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_RST_MODE_3},
    {PAD_SR_RST0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sr_mclk0_tbl[] = {
    {PAD_SR_MCLK0, PADGPIO_BANK, REG_SR_MCLK0_GPIO_MODE, REG_SR_MCLK0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_MCLK0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_SR_MCLK0, PADTOP_BANK, REG_ISR_MCLK_MODE, REG_ISR_MCLK_MODE_MASK, BIT12, PINMUX_FOR_ISR_MCLK_MODE_1},
    {PAD_SR_MCLK0, PADTOP_BANK, REG_SR00_MCLK_MODE, REG_SR00_MCLK_MODE_MASK, BIT0, PINMUX_FOR_SR00_MCLK_MODE_1},
    {PAD_SR_MCLK0, PADTOP_BANK, REG_SR1_MCLK_MODE, REG_SR1_MCLK_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_MCLK_MODE_3},
    {PAD_SR_MCLK0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info i2c1_scl_tbl[] = {
    {PAD_I2C1_SCL, PADGPIO_BANK, REG_I2C1_SCL_GPIO_MODE, REG_I2C1_SCL_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT0, PINMUX_FOR_I2C1_MODE_1},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2, PINMUX_FOR_I2C1_MODE_4},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT0, PINMUX_FOR_DMIC_2CH_MODE_1},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT0, PINMUX_FOR_DMIC_4CH_MODE_1},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT4, PINMUX_FOR_DMIC_6CH_MODE_1},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_SR0_VSYNC_MODE, REG_SR0_VSYNC_MODE_MASK, BIT12, PINMUX_FOR_SR0_VSYNC_MODE_1},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_SR0_SLAVE_MODE, REG_SR0_SLAVE_MODE_MASK, BIT13, PINMUX_FOR_SR0_SLAVE_MODE_2},
    {PAD_I2C1_SCL, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info i2c1_sda_tbl[] = {
    {PAD_I2C1_SDA, PADGPIO_BANK, REG_I2C1_SDA_GPIO_MODE, REG_I2C1_SDA_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT0, PINMUX_FOR_I2C1_MODE_1},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2, PINMUX_FOR_I2C1_MODE_4},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT0, PINMUX_FOR_DMIC_2CH_MODE_1},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT0, PINMUX_FOR_DMIC_4CH_MODE_1},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT4, PINMUX_FOR_DMIC_6CH_MODE_1},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_SR0_HSYNC_MODE, REG_SR0_HSYNC_MODE_MASK, BIT8, PINMUX_FOR_SR0_HSYNC_MODE_1},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_SR0_SLAVE_MODE, REG_SR0_SLAVE_MODE_MASK, BIT13, PINMUX_FOR_SR0_SLAVE_MODE_2},
    {PAD_I2C1_SDA, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sr_rst1_tbl[] = {
    {PAD_SR_RST1, PADGPIO_BANK, REG_SR_RST1_GPIO_MODE, REG_SR_RST1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_RST1, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT8, PINMUX_FOR_SPI2_MODE_1},
    {PAD_SR_RST1, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT0, PINMUX_FOR_DMIC_4CH_MODE_1},
    {PAD_SR_RST1, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT4, PINMUX_FOR_DMIC_6CH_MODE_1},
    {PAD_SR_RST1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_SR_RST1, PADTOP_BANK, REG_SR01_RST_MODE, REG_SR01_RST_MODE_MASK, BIT2, PINMUX_FOR_SR01_RST_MODE_1},
    {PAD_SR_RST1, PADTOP_BANK, REG_SR0_PDN_MODE, REG_SR0_PDN_MODE_MASK, BIT8, PINMUX_FOR_SR0_PDN_MODE_1},
    {PAD_SR_RST1, PADTOP_BANK, REG_SR0_SLAVE_MODE, REG_SR0_SLAVE_MODE_MASK, BIT13, PINMUX_FOR_SR0_SLAVE_MODE_2},
};
const hal_gpio_st_padmux_info sr_mclk1_tbl[] = {
    {PAD_SR_MCLK1, PADGPIO_BANK, REG_SR_MCLK1_GPIO_MODE, REG_SR_MCLK1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_MCLK1, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT8, PINMUX_FOR_SPI2_MODE_1},
    {PAD_SR_MCLK1, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT4, PINMUX_FOR_DMIC_6CH_MODE_1},
    {PAD_SR_MCLK1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_SR_MCLK1, PADTOP_BANK, REG_SR01_MCLK_MODE, REG_SR01_MCLK_MODE_MASK, BIT2, PINMUX_FOR_SR01_MCLK_MODE_1},
    {PAD_SR_MCLK1, PADTOP_BANK, REG_SR0_PCLK_MODE, REG_SR0_PCLK_MODE_MASK, BIT0, PINMUX_FOR_SR0_PCLK_MODE_1},
    {PAD_SR_MCLK1, PADTOP_BANK, REG_SR_SLAVE_XLK_MODE, REG_SR_SLAVE_XLK_MODE_MASK, BIT1,
     PINMUX_FOR_SR_SLAVE_XLK_MODE_2},
};
const hal_gpio_st_padmux_info outp_rx0_ch0_tbl[] = {
    {PAD_OUTP_RX0_CH0, PADGPIO_BANK, REG_OUTP_RX0_CH0_GPIO_MODE, REG_OUTP_RX0_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH0, MIPI0_BANK, REG_OUT_RX0_IE_MODE0, REG_OUT_RX0_IE_MODE0_MASK, BIT0, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH0, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTP_RX0_CH0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2, PINMUX_FOR_SR0_MIPI_MODE_4},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15 | BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_3},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTP_RX0_CH0, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT9,
     PINMUX_FOR_SR0_INFRARED_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx0_ch0_tbl[] = {
    {PAD_OUTN_RX0_CH0, PADGPIO_BANK, REG_OUTN_RX0_CH0_GPIO_MODE, REG_OUTN_RX0_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH0, MIPI0_BANK, REG_OUT_RX0_IE_MODE1, REG_OUT_RX0_IE_MODE1_MASK, BIT1, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH0, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTN_RX0_CH0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2, PINMUX_FOR_SR0_MIPI_MODE_4},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15 | BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_3},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTN_RX0_CH0, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT9,
     PINMUX_FOR_SR0_INFRARED_MODE_2},
};
const hal_gpio_st_padmux_info outp_rx0_ch1_tbl[] = {
    {PAD_OUTP_RX0_CH1, PADGPIO_BANK, REG_OUTP_RX0_CH1_GPIO_MODE, REG_OUTP_RX0_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH1, MIPI0_BANK, REG_OUT_RX0_IE_MODE2, REG_OUT_RX0_IE_MODE2_MASK, BIT2, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH1, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTP_RX0_CH1, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1, PINMUX_FOR_SR0_MIPI_MODE_2},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2, PINMUX_FOR_SR0_MIPI_MODE_4},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15 | BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_3},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTP_RX0_CH1, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT9,
     PINMUX_FOR_SR0_INFRARED_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx0_ch1_tbl[] = {
    {PAD_OUTN_RX0_CH1, PADGPIO_BANK, REG_OUTN_RX0_CH1_GPIO_MODE, REG_OUTN_RX0_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH1, MIPI0_BANK, REG_OUT_RX0_IE_MODE3, REG_OUT_RX0_IE_MODE3_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH1, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTN_RX0_CH1, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1, PINMUX_FOR_SR0_MIPI_MODE_2},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2, PINMUX_FOR_SR0_MIPI_MODE_4},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15 | BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_3},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_PCLK_MODE, REG_SR0_PCLK_MODE_MASK, BIT2, PINMUX_FOR_SR0_PCLK_MODE_4},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTN_RX0_CH1, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
};
const hal_gpio_st_padmux_info outp_rx0_ch2_tbl[] = {
    {PAD_OUTP_RX0_CH2, PADGPIO_BANK, REG_OUTP_RX0_CH2_GPIO_MODE, REG_OUTP_RX0_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH2, MIPI0_BANK, REG_OUT_RX0_IE_MODE4, REG_OUT_RX0_IE_MODE4_MASK, BIT4, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH2, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTP_RX0_CH2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1, PINMUX_FOR_SR0_MIPI_MODE_2},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2, PINMUX_FOR_SR0_MIPI_MODE_4},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15 | BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_3},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX0_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx0_ch2_tbl[] = {
    {PAD_OUTN_RX0_CH2, PADGPIO_BANK, REG_OUTN_RX0_CH2_GPIO_MODE, REG_OUTN_RX0_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH2, MIPI0_BANK, REG_OUT_RX0_IE_MODE5, REG_OUT_RX0_IE_MODE5_MASK, BIT5, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH2, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTN_RX0_CH2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1, PINMUX_FOR_SR0_MIPI_MODE_2},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2, PINMUX_FOR_SR0_MIPI_MODE_4},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15 | BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_3},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX0_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outp_rx0_ch3_tbl[] = {
    {PAD_OUTP_RX0_CH3, PADGPIO_BANK, REG_OUTP_RX0_CH3_GPIO_MODE, REG_OUTP_RX0_CH3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH3, MIPI0_BANK, REG_OUT_RX0_IE_MODE6, REG_OUT_RX0_IE_MODE6_MASK, BIT6, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH3, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTP_RX0_CH3, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_OUTP_RX0_CH3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1, PINMUX_FOR_SR0_MIPI_MODE_2},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_5},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX0_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx0_ch3_tbl[] = {
    {PAD_OUTN_RX0_CH3, PADGPIO_BANK, REG_OUTN_RX0_CH3_GPIO_MODE, REG_OUTN_RX0_CH3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH3, MIPI0_BANK, REG_OUT_RX0_IE_MODE7, REG_OUT_RX0_IE_MODE7_MASK, BIT7, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH3, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTN_RX0_CH3, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_OUTN_RX0_CH3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1, PINMUX_FOR_SR0_MIPI_MODE_2},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_5},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX0_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outp_rx0_ch4_tbl[] = {
    {PAD_OUTP_RX0_CH4, PADGPIO_BANK, REG_OUTP_RX0_CH4_GPIO_MODE, REG_OUTP_RX0_CH4_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH4, MIPI0_BANK, REG_OUT_RX0_IE_MODE8, REG_OUT_RX0_IE_MODE8_MASK, BIT8, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH4, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTP_RX0_CH4, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_OUTP_RX0_CH4, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_5},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX0_CH4, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx0_ch4_tbl[] = {
    {PAD_OUTN_RX0_CH4, PADGPIO_BANK, REG_OUTN_RX0_CH4_GPIO_MODE, REG_OUTN_RX0_CH4_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH4, MIPI0_BANK, REG_OUT_RX0_IE_MODE9, REG_OUT_RX0_IE_MODE9_MASK, BIT9, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH4, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTN_RX0_CH4, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_OUTN_RX0_CH4, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT0, PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_5},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT15,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_2},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX0_CH4, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outp_rx0_ch5_tbl[] = {
    {PAD_OUTP_RX0_CH5, PADGPIO_BANK, REG_OUTP_RX0_CH5_GPIO_MODE, REG_OUTP_RX0_CH5_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH5, MIPI0_BANK, REG_OUT_RX0_IE_MODE10, REG_OUT_RX0_IE_MODE10_MASK, BIT10, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX0_CH5, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTP_RX0_CH5, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_OUTP_RX0_CH5, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_5},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTP_RX0_CH5, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outn_rx0_ch5_tbl[] = {
    {PAD_OUTN_RX0_CH5, PADGPIO_BANK, REG_OUTN_RX0_CH5_GPIO_MODE, REG_OUTN_RX0_CH5_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH5, MIPI0_BANK, REG_OUT_RX0_IE_MODE11, REG_OUT_RX0_IE_MODE11_MASK, BIT11, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX0_CH5, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_OUTN_RX0_CH5, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_OUTN_RX0_CH5, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_3},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_MIPI_MODE, REG_SR0_MIPI_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MIPI_MODE_5},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT0, PINMUX_FOR_SR0_MODE_1},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3 | BIT0, PINMUX_FOR_SR0_MODE_9},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2, PINMUX_FOR_SR0_MODE_4},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SR0_MODE_5},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_BT656_MODE, REG_SR0_BT656_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTN_RX0_CH5, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outp_rx1_ch0_tbl[] = {
    {PAD_OUTP_RX1_CH0, PADGPIO_BANK, REG_OUTP_RX1_CH0_GPIO_MODE, REG_OUTP_RX1_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH0, MIPI1_BANK, REG_OUT_RX1_IE_MODE0, REG_OUT_RX1_IE_MODE0_MASK, BIT0, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT9, PINMUX_FOR_I2C2_MODE_2},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1, PINMUX_FOR_SPI0_MODE_2},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT8, PINMUX_FOR_FUART_MODE_1},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT12, PINMUX_FOR_FUART_2W_MODE_1},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTP_RX1_CH0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outn_rx1_ch0_tbl[] = {
    {PAD_OUTN_RX1_CH0, PADGPIO_BANK, REG_OUTN_RX1_CH0_GPIO_MODE, REG_OUTN_RX1_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH0, MIPI1_BANK, REG_OUT_RX1_IE_MODE1, REG_OUT_RX1_IE_MODE1_MASK, BIT1, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT9, PINMUX_FOR_I2C2_MODE_2},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1, PINMUX_FOR_SPI0_MODE_2},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT8, PINMUX_FOR_FUART_MODE_1},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT12, PINMUX_FOR_FUART_2W_MODE_1},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTN_RX1_CH0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outp_rx1_ch1_tbl[] = {
    {PAD_OUTP_RX1_CH1, PADGPIO_BANK, REG_OUTP_RX1_CH1_GPIO_MODE, REG_OUTP_RX1_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH1, MIPI1_BANK, REG_OUT_RX1_IE_MODE2, REG_OUT_RX1_IE_MODE2_MASK, BIT2, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2C3_MODE_3},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1, PINMUX_FOR_SPI0_MODE_2},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT8, PINMUX_FOR_FUART_MODE_1},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT4, PINMUX_FOR_UART1_MODE_1},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5, PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTP_RX1_CH1, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outn_rx1_ch1_tbl[] = {
    {PAD_OUTN_RX1_CH1, PADGPIO_BANK, REG_OUTN_RX1_CH1_GPIO_MODE, REG_OUTN_RX1_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH1, MIPI1_BANK, REG_OUT_RX1_IE_MODE3, REG_OUT_RX1_IE_MODE3_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2C3_MODE_3},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1, PINMUX_FOR_SPI0_MODE_2},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT8, PINMUX_FOR_FUART_MODE_1},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT4, PINMUX_FOR_UART1_MODE_1},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5, PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTN_RX1_CH1, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outp_rx1_ch2_tbl[] = {
    {PAD_OUTP_RX1_CH2, PADGPIO_BANK, REG_OUTP_RX1_CH2_GPIO_MODE, REG_OUTP_RX1_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH2, MIPI1_BANK, REG_OUT_RX1_IE_MODE4, REG_OUT_RX1_IE_MODE4_MASK, BIT4, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4, PINMUX_FOR_SPI1_MODE_2},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT8, PINMUX_FOR_UART2_MODE_1},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5, PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTP_RX1_CH2, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outn_rx1_ch2_tbl[] = {
    {PAD_OUTN_RX1_CH2, PADGPIO_BANK, REG_OUTN_RX1_CH2_GPIO_MODE, REG_OUTN_RX1_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH2, MIPI1_BANK, REG_OUT_RX1_IE_MODE5, REG_OUT_RX1_IE_MODE5_MASK, BIT5, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4, PINMUX_FOR_SPI1_MODE_2},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT8, PINMUX_FOR_UART2_MODE_1},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5, PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTN_RX1_CH2, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outp_rx1_ch3_tbl[] = {
    {PAD_OUTP_RX1_CH3, PADGPIO_BANK, REG_OUTP_RX1_CH3_GPIO_MODE, REG_OUTP_RX1_CH3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH3, MIPI1_BANK, REG_OUT_RX1_IE_MODE6, REG_OUT_RX1_IE_MODE6_MASK, BIT6, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT1, PINMUX_FOR_I2C3_MODE_2},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4, PINMUX_FOR_SPI1_MODE_2},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT12, PINMUX_FOR_UART3_MODE_1},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5, PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
    {PAD_OUTP_RX1_CH3, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outn_rx1_ch3_tbl[] = {
    {PAD_OUTN_RX1_CH3, PADGPIO_BANK, REG_OUTN_RX1_CH3_GPIO_MODE, REG_OUTN_RX1_CH3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH3, MIPI1_BANK, REG_OUT_RX1_IE_MODE7, REG_OUT_RX1_IE_MODE7_MASK, BIT7, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT1, PINMUX_FOR_I2C3_MODE_2},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4, PINMUX_FOR_SPI1_MODE_2},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT12, PINMUX_FOR_UART3_MODE_1},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT5, PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTN_RX1_CH3, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outp_rx1_ch4_tbl[] = {
    {PAD_OUTP_RX1_CH4, PADGPIO_BANK, REG_OUTP_RX1_CH4_GPIO_MODE, REG_OUTP_RX1_CH4_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX1_CH4, MIPI1_BANK, REG_OUT_RX1_IE_MODE8, REG_OUT_RX1_IE_MODE8_MASK, BIT8, PINMUX_FOR_GPIO_MODE},
#ifndef CONFIG_OPTEE
    {PAD_OUTP_RX1_CH4, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT6, PINMUX_FOR_I2C4_MODE_4},
#endif
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SPI2_MODE_3},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT0, PINMUX_FOR_UART4_MODE_1},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR1_BT656_MODE, REG_SR1_BT656_MODE_MASK, BIT2, PINMUX_FOR_SR1_BT656_MODE_1},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT0, PINMUX_FOR_SR0_BT1120_MODE_1},
    {PAD_OUTP_RX1_CH4, PADTOP_BANK, REG_SR0_BT1120_MODE, REG_SR0_BT1120_MODE_MASK, BIT1, PINMUX_FOR_SR0_BT1120_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx1_ch4_tbl[] = {
    {PAD_OUTN_RX1_CH4, PADGPIO_BANK, REG_OUTN_RX1_CH4_GPIO_MODE, REG_OUTN_RX1_CH4_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX1_CH4, MIPI1_BANK, REG_OUT_RX1_IE_MODE9, REG_OUT_RX1_IE_MODE9_MASK, BIT9, PINMUX_FOR_GPIO_MODE},
#ifndef CONFIG_OPTEE
    {PAD_OUTN_RX1_CH4, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT6, PINMUX_FOR_I2C4_MODE_4},
#endif
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SPI2_MODE_3},
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT0, PINMUX_FOR_UART4_MODE_1},
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_SR1_MIPI_MODE, REG_SR1_MIPI_MODE_MASK, BIT4, PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_SR0_SUB_LVDS_MODE, REG_SR0_SUB_LVDS_MODE_MASK, BIT14,
     PINMUX_FOR_SR0_SUB_LVDS_MODE_1},
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTN_RX1_CH4, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
};
const hal_gpio_st_padmux_info outp_rx2_ch0_tbl[] = {
    {PAD_OUTP_RX2_CH0, PADGPIO_BANK, REG_OUTP_RX2_CH0_GPIO_MODE, REG_OUTP_RX2_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX2_CH0, MIPI2_BANK, REG_OUT_RX2_IE_MODE0, REG_OUT_RX2_IE_MODE0_MASK, BIT0, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_I2C5_MODE, REG_I2C5_MODE_MASK, BIT8, PINMUX_FOR_I2C5_MODE_1},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SPI2_MODE_3},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT4, PINMUX_FOR_UART5_MODE_1},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_SR2_MIPI_MODE, REG_SR2_MIPI_MODE_MASK, BIT8, PINMUX_FOR_SR2_MIPI_MODE_1},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTP_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
};
const hal_gpio_st_padmux_info outn_rx2_ch0_tbl[] = {
    {PAD_OUTN_RX2_CH0, PADGPIO_BANK, REG_OUTN_RX2_CH0_GPIO_MODE, REG_OUTN_RX2_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX2_CH0, MIPI2_BANK, REG_OUT_RX2_IE_MODE1, REG_OUT_RX2_IE_MODE1_MASK, BIT1, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_I2C5_MODE, REG_I2C5_MODE_MASK, BIT8, PINMUX_FOR_I2C5_MODE_1},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SPI2_MODE_3},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT4, PINMUX_FOR_UART5_MODE_1},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_SR2_MIPI_MODE, REG_SR2_MIPI_MODE_MASK, BIT8, PINMUX_FOR_SR2_MIPI_MODE_1},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_SR0_MODE_6},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0, PINMUX_FOR_SR0_MODE_7},
    {PAD_OUTN_RX2_CH0, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT3, PINMUX_FOR_SR0_MODE_8},
};
const hal_gpio_st_padmux_info outp_rx2_ch1_tbl[] = {
    {PAD_OUTP_RX2_CH1, PADGPIO_BANK, REG_OUTP_RX2_CH1_GPIO_MODE, REG_OUTP_RX2_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX2_CH1, MIPI2_BANK, REG_OUT_RX2_IE_MODE2, REG_OUT_RX2_IE_MODE2_MASK, BIT2, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX2_CH1, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT8, PINMUX_FOR_UART6_MODE_1},
    {PAD_OUTP_RX2_CH1, PADTOP_BANK, REG_SR2_MIPI_MODE, REG_SR2_MIPI_MODE_MASK, BIT8, PINMUX_FOR_SR2_MIPI_MODE_1},
    {PAD_OUTP_RX2_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX2_CH1, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT9 | BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_3},
    {PAD_OUTP_RX2_CH1, PADTOP_BANK, REG_SR0_VSYNC_MODE, REG_SR0_VSYNC_MODE_MASK, BIT13, PINMUX_FOR_SR0_VSYNC_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx2_ch1_tbl[] = {
    {PAD_OUTN_RX2_CH1, PADGPIO_BANK, REG_OUTN_RX2_CH1_GPIO_MODE, REG_OUTN_RX2_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX2_CH1, MIPI2_BANK, REG_OUT_RX2_IE_MODE3, REG_OUT_RX2_IE_MODE3_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX2_CH1, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT8, PINMUX_FOR_UART6_MODE_1},
    {PAD_OUTN_RX2_CH1, PADTOP_BANK, REG_SR2_MIPI_MODE, REG_SR2_MIPI_MODE_MASK, BIT8, PINMUX_FOR_SR2_MIPI_MODE_1},
    {PAD_OUTN_RX2_CH1, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX2_CH1, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT9 | BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_3},
    {PAD_OUTN_RX2_CH1, PADTOP_BANK, REG_SR0_HSYNC_MODE, REG_SR0_HSYNC_MODE_MASK, BIT9, PINMUX_FOR_SR0_HSYNC_MODE_2},
};
const hal_gpio_st_padmux_info outp_rx2_ch2_tbl[] = {
    {PAD_OUTP_RX2_CH2, PADGPIO_BANK, REG_OUTP_RX2_CH2_GPIO_MODE, REG_OUTP_RX2_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX2_CH2, MIPI2_BANK, REG_OUT_RX2_IE_MODE4, REG_OUT_RX2_IE_MODE4_MASK, BIT4, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_RX2_CH2, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT0, PINMUX_FOR_SPI3_MODE_1},
    {PAD_OUTP_RX2_CH2, PADTOP_BANK, REG_SR2_MIPI_MODE, REG_SR2_MIPI_MODE_MASK, BIT8, PINMUX_FOR_SR2_MIPI_MODE_1},
    {PAD_OUTP_RX2_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTP_RX2_CH2, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT9 | BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_3},
    {PAD_OUTP_RX2_CH2, PADTOP_BANK, REG_SR0_PCLK_MODE, REG_SR0_PCLK_MODE_MASK, BIT1, PINMUX_FOR_SR0_PCLK_MODE_2},
};
const hal_gpio_st_padmux_info outn_rx2_ch2_tbl[] = {
    {PAD_OUTN_RX2_CH2, PADGPIO_BANK, REG_OUTN_RX2_CH2_GPIO_MODE, REG_OUTN_RX2_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX2_CH2, MIPI2_BANK, REG_OUT_RX2_IE_MODE5, REG_OUT_RX2_IE_MODE5_MASK, BIT5, PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_RX2_CH2, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT0, PINMUX_FOR_SPI3_MODE_1},
    {PAD_OUTN_RX2_CH2, PADTOP_BANK, REG_SR2_MIPI_MODE, REG_SR2_MIPI_MODE_MASK, BIT8, PINMUX_FOR_SR2_MIPI_MODE_1},
    {PAD_OUTN_RX2_CH2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_OUTN_RX2_CH2, PADTOP_BANK, REG_SR0_PDN_MODE, REG_SR0_PDN_MODE_MASK, BIT9, PINMUX_FOR_SR0_PDN_MODE_2},
};
const hal_gpio_st_padmux_info i2c2_scl_tbl[] = {
    {PAD_I2C2_SCL, PADGPIO_BANK, REG_I2C2_SCL_GPIO_MODE, REG_I2C2_SCL_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C2_SCL, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2C2_SCL, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_I2C2_SCL, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2C0_MODE_5},
    {PAD_I2C2_SCL, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C2_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C2_SCL, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT8, PINMUX_FOR_I2C2_MODE_1},
    {PAD_I2C2_SCL, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT0, PINMUX_FOR_SPI3_MODE_1},
    {PAD_I2C2_SCL, PADTOP_BANK, REG_PWM0_MODE, REG_PWM0_MODE_MASK, BIT2, PINMUX_FOR_PWM0_MODE_4},
};
const hal_gpio_st_padmux_info i2c2_sda_tbl[] = {
    {PAD_I2C2_SDA, PADGPIO_BANK, REG_I2C2_SDA_GPIO_MODE, REG_I2C2_SDA_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C2_SDA, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2C2_SDA, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_I2C2_SDA, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_I2C2_SDA, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2C0_MODE_5},
    {PAD_I2C2_SDA, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C2_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C2_SDA, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT8, PINMUX_FOR_I2C2_MODE_1},
    {PAD_I2C2_SDA, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT0, PINMUX_FOR_SPI3_MODE_1},
    {PAD_I2C2_SDA, PADTOP_BANK, REG_PWM1_MODE, REG_PWM1_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_PWM1_MODE_3},
};
const hal_gpio_st_padmux_info sr_rst2_tbl[] = {
    {PAD_SR_RST2, PADGPIO_BANK, REG_SR_RST2_GPIO_MODE, REG_SR_RST2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_RST2, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SR_RST2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_RST2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR_RST2, PADTOP_BANK, REG_PWM2_MODE, REG_PWM2_MODE_MASK, BIT10, PINMUX_FOR_PWM2_MODE_4},
    {PAD_SR_RST2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_SR_RST2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_SR_RST2, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8, PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_SR_RST2, PADTOP_BANK, REG_SR00_RST_MODE, REG_SR00_RST_MODE_MASK, BIT1, PINMUX_FOR_SR00_RST_MODE_2},
    {PAD_SR_RST2, PADTOP_BANK, REG_SR1_RST_MODE, REG_SR1_RST_MODE_MASK, BIT4, PINMUX_FOR_SR1_RST_MODE_1},
};
const hal_gpio_st_padmux_info sr_mclk2_tbl[] = {
    {PAD_SR_MCLK2, PADGPIO_BANK, REG_SR_MCLK2_GPIO_MODE, REG_SR_MCLK2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_MCLK2, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SR_MCLK2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_MCLK2, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR_MCLK2, PADTOP_BANK, REG_PWM3_MODE, REG_PWM3_MODE_MASK, BIT14, PINMUX_FOR_PWM3_MODE_4},
    {PAD_SR_MCLK2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1, PINMUX_FOR_SR0_MODE_2},
    {PAD_SR_MCLK2, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_SR_MCLK2, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_SR_MCLK2, PADTOP_BANK, REG_ISR_MCLK_MODE, REG_ISR_MCLK_MODE_MASK, BIT13, PINMUX_FOR_ISR_MCLK_MODE_2},
    {PAD_SR_MCLK2, PADTOP_BANK, REG_SR00_MCLK_MODE, REG_SR00_MCLK_MODE_MASK, BIT1, PINMUX_FOR_SR00_MCLK_MODE_2},
    {PAD_SR_MCLK2, PADTOP_BANK, REG_SR1_MCLK_MODE, REG_SR1_MCLK_MODE_MASK, BIT4, PINMUX_FOR_SR1_MCLK_MODE_1},
};
const hal_gpio_st_padmux_info i2c3_scl_tbl[] = {
    {PAD_I2C3_SCL, PADGPIO_BANK, REG_I2C3_SCL_GPIO_MODE, REG_I2C3_SCL_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8, PINMUX_FOR_EJ_MODE_2},
    {PAD_I2C3_SCL, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_I2C3_SCL, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_I2C3_SCL, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_I2C3_SCL, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2C1_MODE_5},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT0, PINMUX_FOR_I2C3_MODE_1},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_PWM4_MODE, REG_PWM4_MODE_MASK, BIT2, PINMUX_FOR_PWM4_MODE_4},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_I2C3_SCL, PADTOP_BANK, REG_SR0_VSYNC_MODE, REG_SR0_VSYNC_MODE_MASK, BIT13 | BIT12,
     PINMUX_FOR_SR0_VSYNC_MODE_3},
};
const hal_gpio_st_padmux_info i2c3_sda_tbl[] = {
    {PAD_I2C3_SDA, PADGPIO_BANK, REG_I2C3_SDA_GPIO_MODE, REG_I2C3_SDA_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8, PINMUX_FOR_EJ_MODE_2},
    {PAD_I2C3_SDA, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_I2C3_SDA, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_I2C3_SDA, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_I2C3_SDA, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C0_MODE_6},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2C1_MODE_5},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT2 | BIT1, PINMUX_FOR_I2C1_MODE_6},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT0, PINMUX_FOR_I2C3_MODE_1},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_PWM5_MODE, REG_PWM5_MODE_MASK, BIT6, PINMUX_FOR_PWM5_MODE_4},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_SR0_INFRARED_MODE, REG_SR0_INFRARED_MODE_MASK, BIT8,
     PINMUX_FOR_SR0_INFRARED_MODE_1},
    {PAD_I2C3_SDA, PADTOP_BANK, REG_SR0_HSYNC_MODE, REG_SR0_HSYNC_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SR0_HSYNC_MODE_3},
};
const hal_gpio_st_padmux_info sr_rst3_tbl[] = {
    {PAD_SR_RST3, PADGPIO_BANK, REG_SR_RST3_GPIO_MODE, REG_SR_RST3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_RST3, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8, PINMUX_FOR_EJ_MODE_2},
    {PAD_SR_RST3, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_RST3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_RST3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR_RST3, PADTOP_BANK, REG_PWM6_MODE, REG_PWM6_MODE_MASK, BIT10, PINMUX_FOR_PWM6_MODE_4},
    {PAD_SR_RST3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_SR_RST3, PADTOP_BANK, REG_SR01_RST_MODE, REG_SR01_RST_MODE_MASK, BIT3, PINMUX_FOR_SR01_RST_MODE_2},
    {PAD_SR_RST3, PADTOP_BANK, REG_SR2_RST_MODE, REG_SR2_RST_MODE_MASK, BIT6, PINMUX_FOR_SR2_RST_MODE_1},
    {PAD_SR_RST3, PADTOP_BANK, REG_SR0_PDN_MODE, REG_SR0_PDN_MODE_MASK, BIT10, PINMUX_FOR_SR0_PDN_MODE_4},
};
const hal_gpio_st_padmux_info sr_mclk3_tbl[] = {
    {PAD_SR_MCLK3, PADGPIO_BANK, REG_SR_MCLK3_GPIO_MODE, REG_SR_MCLK3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SR_MCLK3, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8, PINMUX_FOR_EJ_MODE_2},
    {PAD_SR_MCLK3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_MCLK3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR_MCLK3, PADTOP_BANK, REG_PWM7_MODE, REG_PWM7_MODE_MASK, BIT14, PINMUX_FOR_PWM7_MODE_4},
    {PAD_SR_MCLK3, PADTOP_BANK, REG_SR0_MODE, REG_SR0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_MODE_3},
    {PAD_SR_MCLK3, PADTOP_BANK, REG_SR01_MCLK_MODE, REG_SR01_MCLK_MODE_MASK, BIT3, PINMUX_FOR_SR01_MCLK_MODE_2},
    {PAD_SR_MCLK3, PADTOP_BANK, REG_SR2_MCLK_MODE, REG_SR2_MCLK_MODE_MASK, BIT6, PINMUX_FOR_SR2_MCLK_MODE_1},
    {PAD_SR_MCLK3, PADTOP_BANK, REG_SR0_PCLK_MODE, REG_SR0_PCLK_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR0_PCLK_MODE_3},
    {PAD_SR_MCLK3, PADTOP_BANK, REG_SR_SLAVE_XLK_MODE, REG_SR_SLAVE_XLK_MODE_MASK, BIT0,
     PINMUX_FOR_SR_SLAVE_XLK_MODE_1},
};
const hal_gpio_st_padmux_info isp0_xvs_tbl[] = {
    {PAD_ISP0_XVS, PADGPIO_BANK, REG_ISP0_XVS_GPIO_MODE, REG_ISP0_XVS_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_ISP0_XVS, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_ISP0_XVS, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_ISP0_XVS, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT1, PINMUX_FOR_UART4_MODE_2},
    {PAD_ISP0_XVS, PADTOP_BANK, REG_PWM2_MODE, REG_PWM2_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_PWM2_MODE_5},
    {PAD_ISP0_XVS, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT5, PINMUX_FOR_DMIC_6CH_MODE_6},
    {PAD_ISP0_XVS, PADTOP_BANK, REG_SR0_SLAVE_MODE, REG_SR0_SLAVE_MODE_MASK, BIT12, PINMUX_FOR_SR0_SLAVE_MODE_1},
};
const hal_gpio_st_padmux_info isp0_xhs_tbl[] = {
    {PAD_ISP0_XHS, PADGPIO_BANK, REG_ISP0_XHS_GPIO_MODE, REG_ISP0_XHS_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_ISP0_XHS, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_ISP0_XHS, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_ISP0_XHS, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT1, PINMUX_FOR_UART4_MODE_2},
    {PAD_ISP0_XHS, PADTOP_BANK, REG_PWM3_MODE, REG_PWM3_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_PWM3_MODE_5},
    {PAD_ISP0_XHS, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT5, PINMUX_FOR_DMIC_6CH_MODE_6},
    {PAD_ISP0_XHS, PADTOP_BANK, REG_SR1_PDN_MODE, REG_SR1_PDN_MODE_MASK, BIT2, PINMUX_FOR_SR1_PDN_MODE_1},
    {PAD_ISP0_XHS, PADTOP_BANK, REG_SR0_SLAVE_MODE, REG_SR0_SLAVE_MODE_MASK, BIT12, PINMUX_FOR_SR0_SLAVE_MODE_1},
};
const hal_gpio_st_padmux_info isp0_xtrig_tbl[] = {
    {PAD_ISP0_XTRIG, PADGPIO_BANK, REG_ISP0_XTRIG_GPIO_MODE, REG_ISP0_XTRIG_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_ISP0_XTRIG, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT5, PINMUX_FOR_DMIC_6CH_MODE_6},
    {PAD_ISP0_XTRIG, PADTOP_BANK, REG_SR2_PDN_MODE, REG_SR2_PDN_MODE_MASK, BIT4, PINMUX_FOR_SR2_PDN_MODE_1},
    {PAD_ISP0_XTRIG, PADTOP_BANK, REG_SR0_SLAVE_MODE, REG_SR0_SLAVE_MODE_MASK, BIT12, PINMUX_FOR_SR0_SLAVE_MODE_1},
};
const hal_gpio_st_padmux_info pm_sr1_d0_tbl[] = {
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_PM_SR1_D0_GPIO_MODE, REG_PM_SR1_D0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT0, PINMUX_FOR_SSI_MODE_1},
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT1, PINMUX_FOR_SSI_MODE_2},
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SSI_MODE_3},
    {PAD_PM_SR1_D0, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE0, REG_PM_PAD_EXT_MODE0_MASK, BIT0,
     PINMUX_FOR_PM_PAD_EXT_MODE0_1},
    {PAD_PM_SR1_D0, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9, PINMUX_FOR_SPI2_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_d4_tbl[] = {
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_PM_SR1_D4_GPIO_MODE, REG_PM_SR1_D4_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT0, PINMUX_FOR_SSI_MODE_1},
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT1, PINMUX_FOR_SSI_MODE_2},
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SSI_MODE_3},
    {PAD_PM_SR1_D4, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE1, REG_PM_PAD_EXT_MODE1_MASK, BIT1,
     PINMUX_FOR_PM_PAD_EXT_MODE1_1},
};
const hal_gpio_st_padmux_info pm_sr1_pclk_tbl[] = {
    {PAD_PM_SR1_PCLK, PM_PADTOP_BANK, REG_PM_SR1_PCLK_GPIO_MODE, REG_PM_SR1_PCLK_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_PCLK, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_PCLK, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE2, REG_PM_PAD_EXT_MODE2_MASK, BIT2,
     PINMUX_FOR_PM_PAD_EXT_MODE2_1},
    {PAD_PM_SR1_PCLK, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT9, PINMUX_FOR_I2S0_RX_MODE_2},
    {PAD_PM_SR1_PCLK, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1, PINMUX_FOR_I2S0_RXTX_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_rst_tbl[] = {
    {PAD_PM_SR1_RST, PM_PADTOP_BANK, REG_PM_SR1_RST_GPIO_MODE, REG_PM_SR1_RST_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_RST, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SR1_RST, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SR1_RST, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_SR1_RST, PM_PADTOP_BANK, REG_SR_RST_MODE, REG_SR_RST_MODE_MASK, BIT4, PINMUX_FOR_SR_RST_MODE_1},
    {PAD_PM_SR1_RST, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE3, REG_PM_PAD_EXT_MODE3_MASK, BIT3,
     PINMUX_FOR_PM_PAD_EXT_MODE3_1},
    {PAD_PM_SR1_RST, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT5, PINMUX_FOR_SPI1_MODE_4},
    {PAD_PM_SR1_RST, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT5, PINMUX_FOR_UART1_MODE_2},
    {PAD_PM_SR1_RST, PADTOP_BANK, REG_PWM9_MODE, REG_PWM9_MODE_MASK, BIT4, PINMUX_FOR_PWM9_MODE_1},
    {PAD_PM_SR1_RST, PADTOP_BANK, REG_SR00_RST_MODE, REG_SR00_RST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SR00_RST_MODE_3},
    {PAD_PM_SR1_RST, PADTOP_BANK, REG_SR1_RST_MODE, REG_SR1_RST_MODE_MASK, BIT5, PINMUX_FOR_SR1_RST_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_int_tbl[] = {
    {PAD_PM_SR1_INT, PM_PADTOP_BANK, REG_PM_SR1_INT_GPIO_MODE, REG_PM_SR1_INT_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_INT, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT11, PINMUX_FOR_PM_SPI0_MODE_2},
    {PAD_PM_SR1_INT, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT7, PINMUX_FOR_MSPI1_MODE_2},
    {PAD_PM_SR1_INT, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9, PINMUX_FOR_SSPI0_MODE_2},
    {PAD_PM_SR1_INT, PM_PADTOP_BANK, REG_SR_MCLK_MODE, REG_SR_MCLK_MODE_MASK, BIT0, PINMUX_FOR_SR_MCLK_MODE_1},
    {PAD_PM_SR1_INT, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE4, REG_PM_PAD_EXT_MODE4_MASK, BIT4,
     PINMUX_FOR_PM_PAD_EXT_MODE4_1},
    {PAD_PM_SR1_INT, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT5, PINMUX_FOR_UART1_MODE_2},
    {PAD_PM_SR1_INT, PADTOP_BANK, REG_PWM8_MODE, REG_PWM8_MODE_MASK, BIT0, PINMUX_FOR_PWM8_MODE_1},
    {PAD_PM_SR1_INT, PADTOP_BANK, REG_I2S0_MCK_MODE, REG_I2S0_MCK_MODE_MASK, BIT1, PINMUX_FOR_I2S0_MCK_MODE_2},
    {PAD_PM_SR1_INT, PADTOP_BANK, REG_SR00_MCLK_MODE, REG_SR00_MCLK_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_SR00_MCLK_MODE_3},
    {PAD_PM_SR1_INT, PADTOP_BANK, REG_SR1_MCLK_MODE, REG_SR1_MCLK_MODE_MASK, BIT5, PINMUX_FOR_SR1_MCLK_MODE_2},
};
const hal_gpio_st_padmux_info pm_mi2c1_scl_tbl[] = {
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_PM_MI2C1_SCL_GPIO_MODE, REG_PM_MI2C1_SCL_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT0, PINMUX_FOR_CM4_EJ_MODE_1},
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_I2CM1_MODE, REG_I2CM1_MODE_MASK, BIT2, PINMUX_FOR_I2CM1_MODE_1},
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT6 | BIT4,
     PINMUX_FOR_PM_FUART_2W_MODE_5},
    {PAD_PM_MI2C1_SCL, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE5, REG_PM_PAD_EXT_MODE5_MASK, BIT5,
     PINMUX_FOR_PM_PAD_EXT_MODE5_1},
    {PAD_PM_MI2C1_SCL, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2, PINMUX_FOR_I2C0_MODE_4},
    {PAD_PM_MI2C1_SCL, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT10, PINMUX_FOR_I2C2_MODE_4},
#ifndef CONFIG_OPTEE
    {PAD_PM_MI2C1_SCL, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT5, PINMUX_FOR_I2C4_MODE_2},
#endif
    {PAD_PM_MI2C1_SCL, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT5, PINMUX_FOR_SPI1_MODE_4},
    {PAD_PM_MI2C1_SCL, PADTOP_BANK, REG_PWM10_MODE, REG_PWM10_MODE_MASK, BIT8, PINMUX_FOR_PWM10_MODE_1},
};
const hal_gpio_st_padmux_info pm_mi2c1_sda_tbl[] = {
    {PAD_PM_MI2C1_SDA, PM_PADTOP_BANK, REG_PM_MI2C1_SDA_GPIO_MODE, REG_PM_MI2C1_SDA_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_MI2C1_SDA, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT0, PINMUX_FOR_CM4_EJ_MODE_1},
    {PAD_PM_MI2C1_SDA, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_MI2C1_SDA, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_MI2C1_SDA, PM_PADTOP_BANK, REG_I2CM1_MODE, REG_I2CM1_MODE_MASK, BIT2, PINMUX_FOR_I2CM1_MODE_1},
    {PAD_PM_MI2C1_SDA, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT6 | BIT4,
     PINMUX_FOR_PM_FUART_2W_MODE_5},
    {PAD_PM_MI2C1_SDA, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE6, REG_PM_PAD_EXT_MODE6_MASK, BIT6,
     PINMUX_FOR_PM_PAD_EXT_MODE6_1},
    {PAD_PM_MI2C1_SDA, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT2, PINMUX_FOR_I2C0_MODE_4},
    {PAD_PM_MI2C1_SDA, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT10, PINMUX_FOR_I2C2_MODE_4},
#ifndef CONFIG_OPTEE
    {PAD_PM_MI2C1_SDA, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT5, PINMUX_FOR_I2C4_MODE_2},
#endif
    {PAD_PM_MI2C1_SDA, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT5, PINMUX_FOR_SPI1_MODE_4},
    {PAD_PM_MI2C1_SDA, PADTOP_BANK, REG_PWM11_MODE, REG_PWM11_MODE_MASK, BIT12, PINMUX_FOR_PWM11_MODE_1},
};
const hal_gpio_st_padmux_info pm_sr1_d1_tbl[] = {
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_PM_SR1_D1_GPIO_MODE, REG_PM_SR1_D1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_PIR_DIRLK0_MODE, REG_PIR_DIRLK0_MODE_MASK, BIT1, PINMUX_FOR_PIR_DIRLK0_MODE_2},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT13, PINMUX_FOR_PIR_DIRLK3_MODE_2},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT0, PINMUX_FOR_SSI_MODE_1},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT1, PINMUX_FOR_SSI_MODE_2},
    {PAD_PM_SR1_D1, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE7, REG_PM_PAD_EXT_MODE7_MASK, BIT7,
     PINMUX_FOR_PM_PAD_EXT_MODE7_1},
    {PAD_PM_SR1_D1, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9, PINMUX_FOR_SPI2_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_d2_tbl[] = {
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_PM_SR1_D2_GPIO_MODE, REG_PM_SR1_D2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_PIR_DIRLK1_MODE, REG_PIR_DIRLK1_MODE_MASK, BIT5, PINMUX_FOR_PIR_DIRLK1_MODE_2},
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT13, PINMUX_FOR_PIR_DIRLK3_MODE_2},
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT0, PINMUX_FOR_SSI_MODE_1},
    {PAD_PM_SR1_D2, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE8, REG_PM_PAD_EXT_MODE8_MASK, BIT8,
     PINMUX_FOR_PM_PAD_EXT_MODE8_1},
    {PAD_PM_SR1_D2, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9, PINMUX_FOR_SPI2_MODE_2},
    {PAD_PM_SR1_D2, PADTOP_BANK, REG_SDIO_CDZ_MODE, REG_SDIO_CDZ_MODE_MASK, BIT2, PINMUX_FOR_SDIO_CDZ_MODE_4},
    {PAD_PM_SR1_D2, PADTOP_BANK, REG_SDIO_RST_MODE, REG_SDIO_RST_MODE_MASK, BIT6, PINMUX_FOR_SDIO_RST_MODE_4},
};
const hal_gpio_st_padmux_info pm_sr1_d3_tbl[] = {
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_PM_SR1_D3_GPIO_MODE, REG_PM_SR1_D3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_PIR_DIRLK2_MODE, REG_PIR_DIRLK2_MODE_MASK, BIT9, PINMUX_FOR_PIR_DIRLK2_MODE_2},
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT13, PINMUX_FOR_PIR_DIRLK3_MODE_2},
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT0, PINMUX_FOR_SSI_MODE_1},
    {PAD_PM_SR1_D3, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE9, REG_PM_PAD_EXT_MODE9_MASK, BIT9,
     PINMUX_FOR_PM_PAD_EXT_MODE9_1},
    {PAD_PM_SR1_D3, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT9, PINMUX_FOR_SPI2_MODE_2},
    {PAD_PM_SR1_D3, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14, PINMUX_FOR_SDIO_MODE_4},
};
const hal_gpio_st_padmux_info pm_sr1_d5_tbl[] = {
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_PM_SR1_D5_GPIO_MODE, REG_PM_SR1_D5_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT11, PINMUX_FOR_PM_SPI0_MODE_2},
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT7, PINMUX_FOR_MSPI1_MODE_2},
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9, PINMUX_FOR_SSPI0_MODE_2},
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_PIR_SERIAL0_MODE, REG_PIR_SERIAL0_MODE_MASK, BIT9,
     PINMUX_FOR_PIR_SERIAL0_MODE_2},
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D5, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE10, REG_PM_PAD_EXT_MODE10_MASK, BIT10,
     PINMUX_FOR_PM_PAD_EXT_MODE10_1},
    {PAD_PM_SR1_D5, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SPI3_MODE_5},
    {PAD_PM_SR1_D5, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14, PINMUX_FOR_SDIO_MODE_4},
    {PAD_PM_SR1_D5, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT13, PINMUX_FOR_I2S0_TX_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_d6_tbl[] = {
    {PAD_PM_SR1_D6, PM_PADTOP_BANK, REG_PM_SR1_D6_GPIO_MODE, REG_PM_SR1_D6_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D6, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT11, PINMUX_FOR_PM_SPI0_MODE_2},
    {PAD_PM_SR1_D6, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT7, PINMUX_FOR_MSPI1_MODE_2},
    {PAD_PM_SR1_D6, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9, PINMUX_FOR_SSPI0_MODE_2},
    {PAD_PM_SR1_D6, PM_PADTOP_BANK, REG_I2S_RX_MODE, REG_I2S_RX_MODE_MASK, BIT0, PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_PM_SR1_D6, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D6, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE11, REG_PM_PAD_EXT_MODE11_MASK, BIT11,
     PINMUX_FOR_PM_PAD_EXT_MODE11_1},
    {PAD_PM_SR1_D6, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT2, PINMUX_FOR_I2C3_MODE_4},
    {PAD_PM_SR1_D6, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SPI3_MODE_5},
    {PAD_PM_SR1_D6, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14, PINMUX_FOR_SDIO_MODE_4},
    {PAD_PM_SR1_D6, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT13, PINMUX_FOR_I2S0_TX_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_d7_tbl[] = {
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_PM_SR1_D7_GPIO_MODE, REG_PM_SR1_D7_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT11, PINMUX_FOR_PM_SPI0_MODE_2},
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT7, PINMUX_FOR_MSPI1_MODE_2},
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9, PINMUX_FOR_SSPI0_MODE_2},
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_I2S_RX_MODE, REG_I2S_RX_MODE_MASK, BIT0, PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_SSI_CS_MODE, REG_SSI_CS_MODE_MASK, BIT4, PINMUX_FOR_SSI_CS_MODE_1},
    {PAD_PM_SR1_D7, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE12, REG_PM_PAD_EXT_MODE12_MASK, BIT12,
     PINMUX_FOR_PM_PAD_EXT_MODE12_1},
    {PAD_PM_SR1_D7, PADTOP_BANK, REG_I2C3_MODE, REG_I2C3_MODE_MASK, BIT2, PINMUX_FOR_I2C3_MODE_4},
    {PAD_PM_SR1_D7, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SPI3_MODE_5},
    {PAD_PM_SR1_D7, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14, PINMUX_FOR_SDIO_MODE_4},
    {PAD_PM_SR1_D7, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT13, PINMUX_FOR_I2S0_TX_MODE_2},
    {PAD_PM_SR1_D7, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1, PINMUX_FOR_I2S0_RXTX_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_vs_tbl[] = {
    {PAD_PM_SR1_VS, PM_PADTOP_BANK, REG_PM_SR1_VS_GPIO_MODE, REG_PM_SR1_VS_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_VS, PM_PADTOP_BANK, REG_PSPI0_VSYNC_MODE, REG_PSPI0_VSYNC_MODE_MASK, BIT4,
     PINMUX_FOR_PSPI0_VSYNC_MODE_1},
    {PAD_PM_SR1_VS, PM_PADTOP_BANK, REG_I2S_RX_MODE, REG_I2S_RX_MODE_MASK, BIT0, PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_PM_SR1_VS, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_VS, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE13, REG_PM_PAD_EXT_MODE13_MASK, BIT13,
     PINMUX_FOR_PM_PAD_EXT_MODE13_1},
    {PAD_PM_SR1_VS, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SPI3_MODE_5},
    {PAD_PM_SR1_VS, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14, PINMUX_FOR_SDIO_MODE_4},
    {PAD_PM_SR1_VS, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT9, PINMUX_FOR_I2S0_RX_MODE_2},
    {PAD_PM_SR1_VS, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1, PINMUX_FOR_I2S0_RXTX_MODE_2},
};
const hal_gpio_st_padmux_info pm_sr1_hs_tbl[] = {
    {PAD_PM_SR1_HS, PM_PADTOP_BANK, REG_PM_SR1_HS_GPIO_MODE, REG_PM_SR1_HS_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SR1_HS, PM_PADTOP_BANK, REG_PM_SPI0_CZ1_MODE, REG_PM_SPI0_CZ1_MODE_MASK, BIT9,
     PINMUX_FOR_PM_SPI0_CZ1_MODE_2},
    {PAD_PM_SR1_HS, PM_PADTOP_BANK, REG_MSPI1_CZ1_MODE, REG_MSPI1_CZ1_MODE_MASK, BIT5, PINMUX_FOR_MSPI1_CZ1_MODE_2},
    {PAD_PM_SR1_HS, PM_PADTOP_BANK, REG_I2S_MCLK_MODE, REG_I2S_MCLK_MODE_MASK, BIT4, PINMUX_FOR_I2S_MCLK_MODE_1},
    {PAD_PM_SR1_HS, PM_PADTOP_BANK, REG_DVP_MODE, REG_DVP_MODE_MASK, BIT8, PINMUX_FOR_DVP_MODE_1},
    {PAD_PM_SR1_HS, PM_PADTOP_BANK, REG_SR_PD_MODE, REG_SR_PD_MODE_MASK, BIT9, PINMUX_FOR_SR_PD_MODE_2},
    {PAD_PM_SR1_HS, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE14, REG_PM_PAD_EXT_MODE14_MASK, BIT14,
     PINMUX_FOR_PM_PAD_EXT_MODE14_1},
    {PAD_PM_SR1_HS, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14, PINMUX_FOR_SDIO_MODE_4},
    {PAD_PM_SR1_HS, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT9, PINMUX_FOR_I2S0_RX_MODE_2},
    {PAD_PM_SR1_HS, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1, PINMUX_FOR_I2S0_RXTX_MODE_2},
    {PAD_PM_SR1_HS, PADTOP_BANK, REG_SR0_PDN_MODE, REG_SR0_PDN_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SR0_PDN_MODE_3},
    {PAD_PM_SR1_HS, PADTOP_BANK, REG_SR1_PDN_MODE, REG_SR1_PDN_MODE_MASK, BIT3, PINMUX_FOR_SR1_PDN_MODE_2},
};
const hal_gpio_st_padmux_info pm_dmic0_clk_tbl[] = {
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_PM_DMIC0_CLK_GPIO_MODE, REG_PM_DMIC0_CLK_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_I2CM1_MODE, REG_I2CM1_MODE_MASK, BIT3, PINMUX_FOR_I2CM1_MODE_2},
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT5 | BIT4,
     PINMUX_FOR_PM_FUART_2W_MODE_3},
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_PM_PWM0_MODE, REG_PM_PWM0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_PWM0_MODE_3},
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_PM_DMIC_MODE, REG_PM_DMIC_MODE_MASK, BIT8, PINMUX_FOR_PM_DMIC_MODE_1},
    {PAD_PM_DMIC0_CLK, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE15, REG_PM_PAD_EXT_MODE15_MASK, BIT15,
     PINMUX_FOR_PM_PAD_EXT_MODE15_1},
    {PAD_PM_DMIC0_CLK, PADTOP_BANK, REG_PWM0_MODE, REG_PWM0_MODE_MASK, BIT1, PINMUX_FOR_PWM0_MODE_2},
    {PAD_PM_DMIC0_CLK, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT1, PINMUX_FOR_DMIC_2CH_MODE_2},
    {PAD_PM_DMIC0_CLK, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT1, PINMUX_FOR_DMIC_4CH_MODE_2},
    {PAD_PM_DMIC0_CLK, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_DMIC_4CH_MODE_5},
    {PAD_PM_DMIC0_CLK, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5, PINMUX_FOR_DMIC_6CH_MODE_2},
    {PAD_PM_DMIC0_CLK, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_5},
    {PAD_PM_DMIC0_CLK, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT5, PINMUX_FOR_DMIC_6CH_MODE_6},
};
const hal_gpio_st_padmux_info pm_dmic0_d0_tbl[] = {
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_PM_DMIC0_D0_GPIO_MODE, REG_PM_DMIC0_D0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_I2CM1_MODE, REG_I2CM1_MODE_MASK, BIT3, PINMUX_FOR_I2CM1_MODE_2},
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT5 | BIT4,
     PINMUX_FOR_PM_FUART_2W_MODE_3},
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_PM_PWM1_MODE, REG_PM_PWM1_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_PM_PWM1_MODE_3},
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_PM_DMIC_MODE, REG_PM_DMIC_MODE_MASK, BIT8, PINMUX_FOR_PM_DMIC_MODE_1},
    {PAD_PM_DMIC0_D0, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE16, REG_PM_PAD_EXT_MODE16_MASK, BIT0,
     PINMUX_FOR_PM_PAD_EXT_MODE16_1},
    {PAD_PM_DMIC0_D0, PADTOP_BANK, REG_PWM1_MODE, REG_PWM1_MODE_MASK, BIT5, PINMUX_FOR_PWM1_MODE_2},
    {PAD_PM_DMIC0_D0, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT1, PINMUX_FOR_DMIC_2CH_MODE_2},
    {PAD_PM_DMIC0_D0, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT1, PINMUX_FOR_DMIC_4CH_MODE_2},
    {PAD_PM_DMIC0_D0, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_DMIC_4CH_MODE_5},
    {PAD_PM_DMIC0_D0, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5, PINMUX_FOR_DMIC_6CH_MODE_2},
    {PAD_PM_DMIC0_D0, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_5},
    {PAD_PM_DMIC0_D0, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6 | BIT5, PINMUX_FOR_DMIC_6CH_MODE_6},
};
const hal_gpio_st_padmux_info pm_spi_ck_tbl[] = {
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PM_SPI_CK_GPIO_MODE, REG_PM_SPI_CK_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8 | BIT7, PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1, PINMUX_FOR_CM4_EJ_MODE_2},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT6, PINMUX_FOR_MSPI1_MODE_1},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SSPI0_MODE_3},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PSPI0_SLAVE_MODE, REG_PSPI0_SLAVE_MODE_MASK, BIT1,
     PINMUX_FOR_PSPI0_SLAVE_MODE_1},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PIR_DIRLK0_MODE, REG_PIR_DIRLK0_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_PIR_DIRLK0_MODE_3},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT13 | BIT12,
     PINMUX_FOR_PIR_DIRLK3_MODE_3},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_I2S_RX_MODE, REG_I2S_RX_MODE_MASK, BIT1, PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE17, REG_PM_PAD_EXT_MODE17_MASK, BIT1,
     PINMUX_FOR_PM_PAD_EXT_MODE17_1},
    {PAD_PM_SPI_CK, PMSLEEP_BANK, REG_SPI_GPIO, REG_SPI_GPIO_MASK, BIT0, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_CK, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_PM_SPI_CK, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_SPI2_MODE_5},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_5},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT1, PINMUX_FOR_DMIC_4CH_MODE_2},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5, PINMUX_FOR_DMIC_6CH_MODE_2},
};
const hal_gpio_st_padmux_info pm_spi_di_tbl[] = {
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_PM_SPI_DI_GPIO_MODE, REG_PM_SPI_DI_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8 | BIT7, PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1, PINMUX_FOR_CM4_EJ_MODE_2},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT6, PINMUX_FOR_MSPI1_MODE_1},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SSPI0_MODE_3},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_PSPI0_SLAVE_MODE, REG_PSPI0_SLAVE_MODE_MASK, BIT1,
     PINMUX_FOR_PSPI0_SLAVE_MODE_1},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_PIR_DIRLK1_MODE, REG_PIR_DIRLK1_MODE_MASK, BIT5 | BIT4,
     PINMUX_FOR_PIR_DIRLK1_MODE_3},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT13 | BIT12,
     PINMUX_FOR_PIR_DIRLK3_MODE_3},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_I2S_RX_MODE, REG_I2S_RX_MODE_MASK, BIT1, PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE18, REG_PM_PAD_EXT_MODE18_MASK, BIT2,
     PINMUX_FOR_PM_PAD_EXT_MODE18_1},
    {PAD_PM_SPI_DI, PMSLEEP_BANK, REG_SPI_GPIO, REG_SPI_GPIO_MASK, BIT0, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_SPI2_MODE_5},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_5},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5, PINMUX_FOR_DMIC_6CH_MODE_2},
};
const hal_gpio_st_padmux_info pm_spi_hld_tbl[] = {
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_SPI_HLD_GPIO_MODE, REG_PM_SPI_HLD_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_SPIHOLDN_MODE, REG_SPIHOLDN_MODE_MASK, BIT0, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_IR_IN_MODE, REG_IR_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_IR_IN_MODE_3},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PSPI0_SLAVE_MODE, REG_PSPI0_SLAVE_MODE_MASK, BIT1,
     PINMUX_FOR_PSPI0_SLAVE_MODE_1},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT6,
     PINMUX_FOR_PM_FUART_2W_MODE_4},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_PWM0_MODE, REG_PM_PWM0_MODE_MASK, BIT1, PINMUX_FOR_PM_PWM0_MODE_2},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_DMIC_MODE, REG_PM_DMIC_MODE_MASK, BIT9, PINMUX_FOR_PM_DMIC_MODE_2},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE19, REG_PM_PAD_EXT_MODE19_MASK, BIT3,
     PINMUX_FOR_PM_PAD_EXT_MODE19_1},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_SPIHOLDN_MODE, REG_SPIHOLDN_MODE_MASK, 0, PINMUX_FOR_SPIHOLDN_MODE_0},
    {PAD_PM_SPI_HLD, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_PM_SPI_HLD, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_HLD, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_DMIC_2CH_MODE_5},
};
const hal_gpio_st_padmux_info pm_spi_do_tbl[] = {
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PM_SPI_DO_GPIO_MODE, REG_PM_SPI_DO_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8 | BIT7, PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1, PINMUX_FOR_CM4_EJ_MODE_2},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT6, PINMUX_FOR_MSPI1_MODE_1},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SSPI0_MODE_3},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PSPI0_SLAVE_MODE, REG_PSPI0_SLAVE_MODE_MASK, BIT1,
     PINMUX_FOR_PSPI0_SLAVE_MODE_1},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PIR_DIRLK2_MODE, REG_PIR_DIRLK2_MODE_MASK, BIT9 | BIT8,
     PINMUX_FOR_PIR_DIRLK2_MODE_3},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT13 | BIT12,
     PINMUX_FOR_PIR_DIRLK3_MODE_3},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE20, REG_PM_PAD_EXT_MODE20_MASK, BIT4,
     PINMUX_FOR_PM_PAD_EXT_MODE20_1},
    {PAD_PM_SPI_DO, PMSLEEP_BANK, REG_SPI_GPIO, REG_SPI_GPIO_MASK, BIT0, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_DO, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_PM_SPI_DO, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_SPI2_MODE_5},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_5},
};
const hal_gpio_st_padmux_info pm_spi_wpz_tbl[] = {
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_SPI_WPZ_GPIO_MODE, REG_PM_SPI_WPZ_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_MSPI1_CZ1_MODE, REG_MSPI1_CZ1_MODE_MASK, BIT4, PINMUX_FOR_MSPI1_CZ1_MODE_1},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PSPI0_SLAVE_MODE, REG_PSPI0_SLAVE_MODE_MASK, BIT1,
     PINMUX_FOR_PSPI0_SLAVE_MODE_1},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT6,
     PINMUX_FOR_PM_FUART_2W_MODE_4},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_PWM1_MODE, REG_PM_PWM1_MODE_MASK, BIT5, PINMUX_FOR_PM_PWM1_MODE_2},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_DMIC_MODE, REG_PM_DMIC_MODE_MASK, BIT9, PINMUX_FOR_PM_DMIC_MODE_2},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_I2S_MCLK_MODE, REG_I2S_MCLK_MODE_MASK, BIT5, PINMUX_FOR_I2S_MCLK_MODE_2},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE21, REG_PM_PAD_EXT_MODE21_MASK, BIT5,
     PINMUX_FOR_PM_PAD_EXT_MODE21_1},
    {PAD_PM_SPI_WPZ, PMSLEEP_BANK, REG_SPIWPN_GPIO, REG_SPIWPN_GPIO_MASK, BIT4, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_WPZ, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_PM_SPI_WPZ, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_WPZ, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_DMIC_2CH_MODE_5},
};
const hal_gpio_st_padmux_info pm_spi_cz_tbl[] = {
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PM_SPI_CZ_GPIO_MODE, REG_PM_SPI_CZ_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT8 | BIT7, PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1, PINMUX_FOR_CM4_EJ_MODE_2},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_MSPI1_MODE, REG_MSPI1_MODE_MASK, BIT6, PINMUX_FOR_MSPI1_MODE_1},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_SSPI0_MODE_3},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PSPI0_SLAVE_MODE, REG_PSPI0_SLAVE_MODE_MASK, BIT1,
     PINMUX_FOR_PSPI0_SLAVE_MODE_1},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PIR_SERIAL0_MODE, REG_PIR_SERIAL0_MODE_MASK, BIT9 | BIT8,
     PINMUX_FOR_PIR_SERIAL0_MODE_3},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_I2S_RX_MODE, REG_I2S_RX_MODE_MASK, BIT1, PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE22, REG_PM_PAD_EXT_MODE22_MASK, BIT6,
     PINMUX_FOR_PM_PAD_EXT_MODE22_1},
    {PAD_PM_SPI_CZ, PMSLEEP_BANK, REG_SPICSZ1_GPIO, REG_SPICSZ1_GPIO_MASK, BIT2, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_CZ, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_PM_SPI_CZ, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_SPI2_MODE_5},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_5},
};
const hal_gpio_st_padmux_info pm_radar_sar_gpio0_tbl[] = {
    {PAD_PM_RADAR_SAR_GPIO0, PM_PADTOP_BANK, REG_PM_RADAR_SAR_GPIO0_GPIO_MODE, REG_PM_RADAR_SAR_GPIO0_GPIO_MODE_MASK,
     BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_RADAR_SAR_GPIO0, PM_PADTOP_BANK, REG_VBUS_DET_MODE, REG_VBUS_DET_MODE_MASK, BIT4,
     PINMUX_FOR_VBUS_DET_MODE_1},
    {PAD_PM_RADAR_SAR_GPIO0, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT1,
     PINMUX_FOR_PM_FUART_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO0, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT5,
     PINMUX_FOR_PM_FUART_2W_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO0, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE23, REG_PM_PAD_EXT_MODE23_MASK, BIT7,
     PINMUX_FOR_PM_PAD_EXT_MODE23_1},
    {PAD_PM_RADAR_SAR_GPIO0, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1, PINMUX_FOR_SPI3_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO0, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10, PINMUX_FOR_FUART_MODE_4},
    {PAD_PM_RADAR_SAR_GPIO0, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT13, PINMUX_FOR_UART3_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO0, PADTOP_BANK, REG_PWM8_MODE, REG_PWM8_MODE_MASK, BIT1, PINMUX_FOR_PWM8_MODE_2},
};
const hal_gpio_st_padmux_info pm_radar_sar_gpio1_tbl[] = {
    {PAD_PM_RADAR_SAR_GPIO1, PM_PADTOP_BANK, REG_PM_RADAR_SAR_GPIO1_GPIO_MODE, REG_PM_RADAR_SAR_GPIO1_GPIO_MODE_MASK,
     BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_RADAR_SAR_GPIO1, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT1,
     PINMUX_FOR_PM_FUART_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO1, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT5,
     PINMUX_FOR_PM_FUART_2W_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO1, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE24, REG_PM_PAD_EXT_MODE24_MASK, BIT8,
     PINMUX_FOR_PM_PAD_EXT_MODE24_1},
    {PAD_PM_RADAR_SAR_GPIO1, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1, PINMUX_FOR_SPI3_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO1, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10, PINMUX_FOR_FUART_MODE_4},
    {PAD_PM_RADAR_SAR_GPIO1, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT13, PINMUX_FOR_UART3_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO1, PADTOP_BANK, REG_PWM9_MODE, REG_PWM9_MODE_MASK, BIT5, PINMUX_FOR_PWM9_MODE_2},
};
const hal_gpio_st_padmux_info pm_radar_sar_gpio2_tbl[] = {
    {PAD_PM_RADAR_SAR_GPIO2, PM_PADTOP_BANK, REG_PM_RADAR_SAR_GPIO2_GPIO_MODE, REG_PM_RADAR_SAR_GPIO2_GPIO_MODE_MASK,
     BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_RADAR_SAR_GPIO2, PM_PADTOP_BANK, REG_I2CM0_MODE, REG_I2CM0_MODE_MASK, BIT1, PINMUX_FOR_I2CM0_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO2, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT1,
     PINMUX_FOR_PM_FUART_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO2, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE25, REG_PM_PAD_EXT_MODE25_MASK, BIT9,
     PINMUX_FOR_PM_PAD_EXT_MODE25_1},
    {PAD_PM_RADAR_SAR_GPIO2, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1, PINMUX_FOR_SPI3_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO2, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10, PINMUX_FOR_FUART_MODE_4},
    {PAD_PM_RADAR_SAR_GPIO2, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT14, PINMUX_FOR_FUART_2W_MODE_4},
    {PAD_PM_RADAR_SAR_GPIO2, PADTOP_BANK, REG_PWM10_MODE, REG_PWM10_MODE_MASK, BIT9, PINMUX_FOR_PWM10_MODE_2},
};
const hal_gpio_st_padmux_info pm_radar_sar_gpio3_tbl[] = {
    {PAD_PM_RADAR_SAR_GPIO3, PM_PADTOP_BANK, REG_PM_RADAR_SAR_GPIO3_GPIO_MODE, REG_PM_RADAR_SAR_GPIO3_GPIO_MODE_MASK,
     BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_RADAR_SAR_GPIO3, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO3, PM_PADTOP_BANK, REG_I2CM0_MODE, REG_I2CM0_MODE_MASK, BIT1, PINMUX_FOR_I2CM0_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO3, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT1,
     PINMUX_FOR_PM_FUART_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO3, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE26, REG_PM_PAD_EXT_MODE26_MASK, BIT10,
     PINMUX_FOR_PM_PAD_EXT_MODE26_1},
    {PAD_PM_RADAR_SAR_GPIO3, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1, PINMUX_FOR_SPI3_MODE_2},
    {PAD_PM_RADAR_SAR_GPIO3, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10, PINMUX_FOR_FUART_MODE_4},
    {PAD_PM_RADAR_SAR_GPIO3, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT14, PINMUX_FOR_FUART_2W_MODE_4},
    {PAD_PM_RADAR_SAR_GPIO3, PADTOP_BANK, REG_PWM11_MODE, REG_PWM11_MODE_MASK, BIT13, PINMUX_FOR_PWM11_MODE_2},
};
const hal_gpio_st_padmux_info pm_uart_rx_tbl[] = {
    {PAD_PM_UART_RX, PM_PADTOP_BANK, REG_PM_UART_RX_GPIO_MODE, REG_PM_UART_RX_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_UART_RX, PM_PADTOP_BANK, REG_PM_UART0_MODE, REG_PM_UART0_MODE_MASK, BIT8, PINMUX_FOR_PM_UART0_MODE_1},
    {PAD_PM_UART_RX, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE27, REG_PM_PAD_EXT_MODE27_MASK, BIT11,
     PINMUX_FOR_PM_PAD_EXT_MODE27_1},
    {PAD_PM_UART_RX, PM_PADTOP_BANK, REG_UART_GPIO_EN, REG_UART_GPIO_EN_MASK, BIT4, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_UART_RX, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT9, PINMUX_FOR_UART6_MODE_2},
};
const hal_gpio_st_padmux_info pm_uart_tx_tbl[] = {
    {PAD_PM_UART_TX, PM_PADTOP_BANK, REG_PM_UART_TX_GPIO_MODE, REG_PM_UART_TX_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_UART_TX, PM_PADTOP_BANK, REG_PM_UART0_MODE, REG_PM_UART0_MODE_MASK, BIT8, PINMUX_FOR_PM_UART0_MODE_1},
    {PAD_PM_UART_TX, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE28, REG_PM_PAD_EXT_MODE28_MASK, BIT12,
     PINMUX_FOR_PM_PAD_EXT_MODE28_1},
    {PAD_PM_UART_TX, PM_PADTOP_BANK, REG_UART_GPIO_EN, REG_UART_GPIO_EN_MASK, BIT4, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_UART_TX, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT9, PINMUX_FOR_UART6_MODE_2},
};
const hal_gpio_st_padmux_info pm_intout_tbl[] = {
    {PAD_PM_INTOUT, PM_PADTOP_BANK, REG_PM_INTOUT_GPIO_MODE, REG_PM_INTOUT_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_INTOUT, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT7, PINMUX_FOR_EJ_MODE_1},
    {PAD_PM_INTOUT, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_CM4_EJ_MODE_3},
    {PAD_PM_INTOUT, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_INTOUT, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_INTOUT, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT0, PINMUX_FOR_PM_FUART_MODE_1},
    {PAD_PM_INTOUT, PM_PADTOP_BANK, REG_INT_O_MODE, REG_INT_O_MODE_MASK, BIT8, PINMUX_FOR_INT_O_MODE_1},
    {PAD_PM_INTOUT, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE29, REG_PM_PAD_EXT_MODE29_MASK, BIT13,
     PINMUX_FOR_PM_PAD_EXT_MODE29_1},
    {PAD_PM_INTOUT, PADTOP_BANK, REG_SPI0_CZ1_MODE, REG_SPI0_CZ1_MODE_MASK, BIT4, PINMUX_FOR_SPI0_CZ1_MODE_1},
    {PAD_PM_INTOUT, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9, PINMUX_FOR_FUART_MODE_2},
};
const hal_gpio_st_padmux_info pm_gpio0_tbl[] = {
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_GPIO0_GPIO_MODE, REG_PM_GPIO0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT0, PINMUX_FOR_CM4_EJ_MODE_1},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_PWM0_MODE, REG_PM_PWM0_MODE_MASK, BIT0, PINMUX_FOR_PM_PWM0_MODE_1},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PIR_DIRLK0_MODE, REG_PIR_DIRLK0_MODE_MASK, BIT0, PINMUX_FOR_PIR_DIRLK0_MODE_1},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT12, PINMUX_FOR_PIR_DIRLK3_MODE_1},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_SR_PD_MODE, REG_SR_PD_MODE_MASK, BIT8, PINMUX_FOR_SR_PD_MODE_1},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE30, REG_PM_PAD_EXT_MODE30_MASK, BIT14,
     PINMUX_FOR_PM_PAD_EXT_MODE30_1},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_SPICSZ2_MODE, REG_SPICSZ2_MODE_MASK, BIT0, PINMUX_FOR_SPICSZ2_MODE_1},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_FUART_MODE_5},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_FUART_2W_MODE_5},
};
const hal_gpio_st_padmux_info pm_gpio1_tbl[] = {
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_GPIO1_GPIO_MODE, REG_PM_GPIO1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT0, PINMUX_FOR_CM4_EJ_MODE_1},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_PWM1_MODE, REG_PM_PWM1_MODE_MASK, BIT4, PINMUX_FOR_PM_PWM1_MODE_1},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PIR_DIRLK1_MODE, REG_PIR_DIRLK1_MODE_MASK, BIT4, PINMUX_FOR_PIR_DIRLK1_MODE_1},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT12, PINMUX_FOR_PIR_DIRLK3_MODE_1},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE31, REG_PM_PAD_EXT_MODE31_MASK, BIT15,
     PINMUX_FOR_PM_PAD_EXT_MODE31_1},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_FUART_MODE_5},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_FUART_2W_MODE_5},
};
const hal_gpio_st_padmux_info pm_gpio2_tbl[] = {
    {PAD_PM_GPIO2, PM_PADTOP_BANK, REG_PM_GPIO2_GPIO_MODE, REG_PM_GPIO2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO2, PM_PADTOP_BANK, REG_PIR_DIRLK2_MODE, REG_PIR_DIRLK2_MODE_MASK, BIT8, PINMUX_FOR_PIR_DIRLK2_MODE_1},
    {PAD_PM_GPIO2, PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE, REG_PIR_DIRLK3_MODE_MASK, BIT12, PINMUX_FOR_PIR_DIRLK3_MODE_1},
    {PAD_PM_GPIO2, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE32, REG_PM_PAD_EXT_MODE32_MASK, BIT0,
     PINMUX_FOR_PM_PAD_EXT_MODE32_1},
};
const hal_gpio_st_padmux_info pm_gpio3_tbl[] = {
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_GPIO3_GPIO_MODE, REG_PM_GPIO3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PM_TEST_MODE_3},
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE33, REG_PM_PAD_EXT_MODE33_MASK, BIT1,
     PINMUX_FOR_PM_PAD_EXT_MODE33_1},
    {PAD_PM_GPIO3, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_PM_GPIO3, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
};
const hal_gpio_st_padmux_info pm_gpio4_tbl[] = {
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_GPIO4_GPIO_MODE, REG_PM_GPIO4_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PIR_SERIAL0_MODE, REG_PIR_SERIAL0_MODE_MASK, BIT8,
     PINMUX_FOR_PIR_SERIAL0_MODE_1},
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE34, REG_PM_PAD_EXT_MODE34_MASK, BIT2,
     PINMUX_FOR_PM_PAD_EXT_MODE34_1},
    {PAD_PM_GPIO4, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_PM_GPIO4, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_PM_GPIO4, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
};
const hal_gpio_st_padmux_info pm_gpio5_tbl[] = {
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_GPIO5_GPIO_MODE, REG_PM_GPIO5_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE35, REG_PM_PAD_EXT_MODE35_MASK, BIT3,
     PINMUX_FOR_PM_PAD_EXT_MODE35_1},
    {PAD_PM_GPIO5, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_PM_GPIO5, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
};
const hal_gpio_st_padmux_info pm_fuart_rx_tbl[] = {
    {PAD_PM_FUART_RX, PM_PADTOP_BANK, REG_PM_FUART_RX_GPIO_MODE, REG_PM_FUART_RX_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_FUART_RX, PM_PADTOP_BANK, REG_I2CM0_MODE, REG_I2CM0_MODE_MASK, BIT0, PINMUX_FOR_I2CM0_MODE_1},
    {PAD_PM_FUART_RX, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT0, PINMUX_FOR_PM_FUART_MODE_1},
    {PAD_PM_FUART_RX, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT4,
     PINMUX_FOR_PM_FUART_2W_MODE_1},
    {PAD_PM_FUART_RX, PM_PADTOP_BANK, REG_PM_PWM0_MODE, REG_PM_PWM0_MODE_MASK, BIT2, PINMUX_FOR_PM_PWM0_MODE_4},
    {PAD_PM_FUART_RX, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE36, REG_PM_PAD_EXT_MODE36_MASK, BIT4,
     PINMUX_FOR_PM_PAD_EXT_MODE36_1},
    {PAD_PM_FUART_RX, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9, PINMUX_FOR_FUART_MODE_2},
    {PAD_PM_FUART_RX, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT13, PINMUX_FOR_FUART_2W_MODE_2},
    {PAD_PM_FUART_RX, PADTOP_BANK, REG_PWM2_MODE, REG_PWM2_MODE_MASK, BIT9, PINMUX_FOR_PWM2_MODE_2},
};
const hal_gpio_st_padmux_info pm_fuart_tx_tbl[] = {
    {PAD_PM_FUART_TX, PM_PADTOP_BANK, REG_PM_FUART_TX_GPIO_MODE, REG_PM_FUART_TX_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_FUART_TX, PM_PADTOP_BANK, REG_I2CM0_MODE, REG_I2CM0_MODE_MASK, BIT0, PINMUX_FOR_I2CM0_MODE_1},
    {PAD_PM_FUART_TX, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT0, PINMUX_FOR_PM_FUART_MODE_1},
    {PAD_PM_FUART_TX, PM_PADTOP_BANK, REG_PM_FUART_2W_MODE, REG_PM_FUART_2W_MODE_MASK, BIT4,
     PINMUX_FOR_PM_FUART_2W_MODE_1},
    {PAD_PM_FUART_TX, PM_PADTOP_BANK, REG_PM_PWM1_MODE, REG_PM_PWM1_MODE_MASK, BIT6, PINMUX_FOR_PM_PWM1_MODE_4},
    {PAD_PM_FUART_TX, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE37, REG_PM_PAD_EXT_MODE37_MASK, BIT5,
     PINMUX_FOR_PM_PAD_EXT_MODE37_1},
    {PAD_PM_FUART_TX, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9, PINMUX_FOR_FUART_MODE_2},
    {PAD_PM_FUART_TX, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT13, PINMUX_FOR_FUART_2W_MODE_2},
    {PAD_PM_FUART_TX, PADTOP_BANK, REG_PWM3_MODE, REG_PWM3_MODE_MASK, BIT13, PINMUX_FOR_PWM3_MODE_2},
};
const hal_gpio_st_padmux_info pm_irin_tbl[] = {
    {PAD_PM_IRIN, PM_PADTOP_BANK, REG_PM_IRIN_GPIO_MODE, REG_PM_IRIN_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_PM_IRIN, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_IRIN, PM_PADTOP_BANK, REG_IR_IN_MODE, REG_IR_IN_MODE_MASK, BIT0, PINMUX_FOR_IR_IN_MODE_1},
    {PAD_PM_IRIN, PM_PADTOP_BANK, REG_RADAR_20M_MODE, REG_RADAR_20M_MODE_MASK, BIT1, PINMUX_FOR_RADAR_20M_MODE_1},
    {PAD_PM_IRIN, PM_PADTOP_BANK, REG_PM_FUART_MODE, REG_PM_FUART_MODE_MASK, BIT0, PINMUX_FOR_PM_FUART_MODE_1},
    {PAD_PM_IRIN, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE38, REG_PM_PAD_EXT_MODE38_MASK, BIT6,
     PINMUX_FOR_PM_PAD_EXT_MODE38_1},
    {PAD_PM_IRIN, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9, PINMUX_FOR_FUART_MODE_2},
};
const hal_gpio_st_padmux_info pm_mspi0_do_tbl[] = {
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_PM_MSPI0_DO_GPIO_MODE, REG_PM_MSPI0_DO_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_MSPI0_DO, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT7, PINMUX_FOR_EJ_MODE_1},
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_CM4_EJ_MODE_3},
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT10, PINMUX_FOR_PM_SPI0_MODE_1},
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT8, PINMUX_FOR_SSPI0_MODE_1},
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT2, PINMUX_FOR_SSI_MODE_4},
    {PAD_PM_MSPI0_DO, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE39, REG_PM_PAD_EXT_MODE39_MASK, BIT7,
     PINMUX_FOR_PM_PAD_EXT_MODE39_1},
    {PAD_PM_MSPI0_DO, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT0, PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_MSPI0_DO, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT5, PINMUX_FOR_UART5_MODE_2},
    {PAD_PM_MSPI0_DO, PADTOP_BANK, REG_PWM7_MODE, REG_PWM7_MODE_MASK, BIT13, PINMUX_FOR_PWM7_MODE_2},
    {PAD_PM_MSPI0_DO, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT8, PINMUX_FOR_VD_MODE_1},
};
const hal_gpio_st_padmux_info pm_mspi0_di_tbl[] = {
    {PAD_PM_MSPI0_DI, PM_PADTOP_BANK, REG_PM_MSPI0_DI_GPIO_MODE, REG_PM_MSPI0_DI_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_MSPI0_DI, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT10, PINMUX_FOR_PM_SPI0_MODE_1},
    {PAD_PM_MSPI0_DI, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT8, PINMUX_FOR_SSPI0_MODE_1},
    {PAD_PM_MSPI0_DI, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE40, REG_PM_PAD_EXT_MODE40_MASK, BIT8,
     PINMUX_FOR_PM_PAD_EXT_MODE40_1},
    {PAD_PM_MSPI0_DI, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT0, PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_MSPI0_DI, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT5, PINMUX_FOR_UART5_MODE_2},
    {PAD_PM_MSPI0_DI, PADTOP_BANK, REG_PWM6_MODE, REG_PWM6_MODE_MASK, BIT9, PINMUX_FOR_PWM6_MODE_2},
};
const hal_gpio_st_padmux_info pm_mspi0_ck_tbl[] = {
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_PM_MSPI0_CK_GPIO_MODE, REG_PM_MSPI0_CK_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_MSPI0_CK, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT7, PINMUX_FOR_EJ_MODE_1},
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_CM4_EJ_MODE_3},
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT10, PINMUX_FOR_PM_SPI0_MODE_1},
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT8, PINMUX_FOR_SSPI0_MODE_1},
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_SSI_MODE, REG_SSI_MODE_MASK, BIT2, PINMUX_FOR_SSI_MODE_4},
    {PAD_PM_MSPI0_CK, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE41, REG_PM_PAD_EXT_MODE41_MASK, BIT9,
     PINMUX_FOR_PM_PAD_EXT_MODE41_1},
    {PAD_PM_MSPI0_CK, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT0, PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_MSPI0_CK, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_FUART_MODE_5},
    {PAD_PM_MSPI0_CK, PADTOP_BANK, REG_PWM5_MODE, REG_PWM5_MODE_MASK, BIT5, PINMUX_FOR_PWM5_MODE_2},
};
const hal_gpio_st_padmux_info pm_mspi0_cz_tbl[] = {
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_PM_MSPI0_CZ_GPIO_MODE, REG_PM_MSPI0_CZ_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_MSPI0_CZ, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT7, PINMUX_FOR_EJ_MODE_1},
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_CM4_EJ_MODE, REG_CM4_EJ_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_CM4_EJ_MODE_3},
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT0, PINMUX_FOR_PM_TEST_MODE_1},
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_PM_SPI0_MODE, REG_PM_SPI0_MODE_MASK, BIT10, PINMUX_FOR_PM_SPI0_MODE_1},
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_SSPI0_MODE, REG_SSPI0_MODE_MASK, BIT8, PINMUX_FOR_SSPI0_MODE_1},
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_SSI_CS_MODE, REG_SSI_CS_MODE_MASK, BIT5, PINMUX_FOR_SSI_CS_MODE_2},
    {PAD_PM_MSPI0_CZ, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE42, REG_PM_PAD_EXT_MODE42_MASK, BIT10,
     PINMUX_FOR_PM_PAD_EXT_MODE42_1},
    {PAD_PM_MSPI0_CZ, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT0, PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_MSPI0_CZ, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_FUART_MODE_5},
    {PAD_PM_MSPI0_CZ, PADTOP_BANK, REG_PWM4_MODE, REG_PWM4_MODE_MASK, BIT1, PINMUX_FOR_PWM4_MODE_2},
    {PAD_PM_MSPI0_CZ, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT8, PINMUX_FOR_VD_MODE_1},
};
const hal_gpio_st_padmux_info pm_usb3_int_tbl[] = {
    {PAD_PM_USB3_INT, PM_PADTOP_BANK, REG_PM_USB3_INT_GPIO_MODE, REG_PM_USB3_INT_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_USB3_INT, PM_PADTOP_BANK, REG_PM_TEST_MODE, REG_PM_TEST_MODE_MASK, BIT1, PINMUX_FOR_PM_TEST_MODE_2},
    {PAD_PM_USB3_INT, PM_PADTOP_BANK, REG_IR_IN_MODE, REG_IR_IN_MODE_MASK, BIT1, PINMUX_FOR_IR_IN_MODE_2},
    {PAD_PM_USB3_INT, PM_PADTOP_BANK, REG_PM_SPI0_CZ1_MODE, REG_PM_SPI0_CZ1_MODE_MASK, BIT8,
     PINMUX_FOR_PM_SPI0_CZ1_MODE_1},
    {PAD_PM_USB3_INT, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE43, REG_PM_PAD_EXT_MODE43_MASK, BIT11,
     PINMUX_FOR_PM_PAD_EXT_MODE43_1},
    {PAD_PM_USB3_INT, PADTOP_BANK, REG_I2C5_MODE, REG_I2C5_MODE_MASK, BIT9, PINMUX_FOR_I2C5_MODE_2},
    {PAD_PM_USB3_INT, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT5, PINMUX_FOR_SPI1_MODE_4},
};
const hal_gpio_st_padmux_info pm_usb3_id_tbl[] = {
    {PAD_PM_USB3_ID, PM_PADTOP_BANK, REG_PM_USB3_ID_GPIO_MODE, REG_PM_USB3_ID_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_PM_USB3_ID, PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE44, REG_PM_PAD_EXT_MODE44_MASK, BIT12,
     PINMUX_FOR_PM_PAD_EXT_MODE44_1},
    {PAD_PM_USB3_ID, PADTOP_BANK, REG_I2C5_MODE, REG_I2C5_MODE_MASK, BIT9, PINMUX_FOR_I2C5_MODE_2},
};
const hal_gpio_st_padmux_info sar_gpio0_tbl[] = {
    {PAD_SAR_GPIO0, PM_SAR_BANK, REG_SAR_GPIO0_GPIO_MODE, REG_SAR_GPIO0_GPIO_MODE_MASK, 0, PINMUX_FOR_GPIO_MODE},
};
const hal_gpio_st_padmux_info sar_gpio1_tbl[] = {
    {PAD_SAR_GPIO1, PM_SAR_BANK, REG_SAR_GPIO1_GPIO_MODE, REG_SAR_GPIO1_GPIO_MODE_MASK, 0, PINMUX_FOR_GPIO_MODE},
};
const hal_gpio_st_padmux_info rgmii0_mclk_tbl[] = {
    {PAD_RGMII0_MCLK, PADGPIO_BANK, REG_RGMII0_MCLK_GPIO_MODE, REG_RGMII0_MCLK_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI0_MODE_3},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_FUART_MODE_3},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT13 | BIT12,
     PINMUX_FOR_FUART_2W_MODE_3},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT9, PINMUX_FOR_VD_MODE_2},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_GPHY0_REF_MODE, REG_GPHY0_REF_MODE_MASK, BIT0, PINMUX_FOR_GPHY0_REF_MODE_1},
    {PAD_RGMII0_MCLK, PADTOP_BANK, REG_GMII_SEL_MODE, REG_GMII_SEL_MASK, BIT0, PINMUX_FOR_GPHY0_REF_MODE_1},
};
const hal_gpio_st_padmux_info rgmii0_rstn_tbl[] = {
    {PAD_RGMII0_RSTN, PADGPIO_BANK, REG_RGMII0_RSTN_GPIO_MODE, REG_RGMII0_RSTN_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI0_MODE_3},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_FUART_MODE_3},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_FUART_2W_MODE, REG_FUART_2W_MODE_MASK, BIT13 | BIT12,
     PINMUX_FOR_FUART_2W_MODE_3},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT9, PINMUX_FOR_VD_MODE_2},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_RGMII0_RSTN, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_rxclk_tbl[] = {
    {PAD_RGMII0_RXCLK, PADGPIO_BANK, REG_RGMII0_RXCLK_GPIO_MODE, REG_RGMII0_RXCLK_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI0_MODE_3},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_FUART_MODE_3},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_UART1_MODE_3},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_RXCLK, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_rxctl_tbl[] = {
    {PAD_RGMII0_RXCTL, PADGPIO_BANK, REG_RGMII0_RXCTL_GPIO_MODE, REG_RGMII0_RXCTL_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_SPI0_MODE, REG_SPI0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI0_MODE_3},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_FUART_MODE, REG_FUART_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_FUART_MODE_3},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_UART1_MODE_3},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_RXCTL, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_rxd0_tbl[] = {
    {PAD_RGMII0_RXD0, PADGPIO_BANK, REG_RGMII0_RXD0_GPIO_MODE, REG_RGMII0_RXD0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_SPI0_CZ1_MODE, REG_SPI0_CZ1_MODE_MASK, BIT5, PINMUX_FOR_SPI0_CZ1_MODE_2},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT9, PINMUX_FOR_UART2_MODE_2},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_DMIC_2CH_MODE_3},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_DMIC_4CH_MODE_3},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_3},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_RXD0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_rxd1_tbl[] = {
    {PAD_RGMII0_RXD1, PADGPIO_BANK, REG_RGMII0_RXD1_GPIO_MODE, REG_RGMII0_RXD1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_RXD1, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT9, PINMUX_FOR_UART2_MODE_2},
    {PAD_RGMII0_RXD1, PADTOP_BANK, REG_DMIC_2CH_MODE, REG_DMIC_2CH_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_DMIC_2CH_MODE_3},
    {PAD_RGMII0_RXD1, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_DMIC_4CH_MODE_3},
    {PAD_RGMII0_RXD1, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_3},
    {PAD_RGMII0_RXD1, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_RXD1, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_RXD1, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
};
const hal_gpio_st_padmux_info rgmii0_rxd2_tbl[] = {
    {PAD_RGMII0_RXD2, PADGPIO_BANK, REG_RGMII0_RXD2_GPIO_MODE, REG_RGMII0_RXD2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10, PINMUX_FOR_SPI2_MODE_4},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_UART3_MODE_3},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_DMIC_4CH_MODE, REG_DMIC_4CH_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_DMIC_4CH_MODE_3},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_3},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_RGMII0_RXD2, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
};
const hal_gpio_st_padmux_info rgmii0_rxd3_tbl[] = {
    {PAD_RGMII0_RXD3, PADGPIO_BANK, REG_RGMII0_RXD3_GPIO_MODE, REG_RGMII0_RXD3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10, PINMUX_FOR_SPI2_MODE_4},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_UART3_MODE_3},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_DMIC_6CH_MODE_3},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_RGMII0_RXD3, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
};
const hal_gpio_st_padmux_info rgmii0_txclk_tbl[] = {
    {PAD_RGMII0_TXCLK, PADGPIO_BANK, REG_RGMII0_TXCLK_GPIO_MODE, REG_RGMII0_TXCLK_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_TXCLK, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10, PINMUX_FOR_SPI2_MODE_4},
    {PAD_RGMII0_TXCLK, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_UART4_MODE_3},
    {PAD_RGMII0_TXCLK, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_TXCLK, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_TXCLK, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_TXCLK, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_RGMII0_TXCLK, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
};
const hal_gpio_st_padmux_info rgmii0_txctl_tbl[] = {
    {PAD_RGMII0_TXCTL, PADGPIO_BANK, REG_RGMII0_TXCTL_GPIO_MODE, REG_RGMII0_TXCTL_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_SPI2_MODE, REG_SPI2_MODE_MASK, BIT10, PINMUX_FOR_SPI2_MODE_4},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_UART4_MODE, REG_UART4_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_UART4_MODE_3},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_TXCTL, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_txd0_tbl[] = {
    {PAD_RGMII0_TXD0, PADGPIO_BANK, REG_RGMII0_TXD0_GPIO_MODE, REG_RGMII0_TXD0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_TXD0, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI3_MODE_3},
    {PAD_RGMII0_TXD0, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_TXD0, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_TXD0, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_TXD0, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_TXD0, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_TXD0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_txd1_tbl[] = {
    {PAD_RGMII0_TXD1, PADGPIO_BANK, REG_RGMII0_TXD1_GPIO_MODE, REG_RGMII0_TXD1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_TXD1, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI3_MODE_3},
    {PAD_RGMII0_TXD1, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_TXD1, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_TXD1, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_TXD1, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_TXD1, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_TXD1, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_txd2_tbl[] = {
    {PAD_RGMII0_TXD2, PADGPIO_BANK, REG_RGMII0_TXD2_GPIO_MODE, REG_RGMII0_TXD2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
#ifndef CONFIG_OPTEE
    {PAD_RGMII0_TXD2, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_I2C4_MODE_3},
#endif
    {PAD_RGMII0_TXD2, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI3_MODE_3},
    {PAD_RGMII0_TXD2, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_TXD2, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_RGMII0_TXD2, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_TXD2, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_txd3_tbl[] = {
    {PAD_RGMII0_TXD3, PADGPIO_BANK, REG_RGMII0_TXD3_GPIO_MODE, REG_RGMII0_TXD3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
#ifndef CONFIG_OPTEE
    {PAD_RGMII0_TXD3, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_I2C4_MODE_3},
#endif
    {PAD_RGMII0_TXD3, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SPI3_MODE_3},
    {PAD_RGMII0_TXD3, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_TXD3, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_RGMII0_TXD3, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_TXD3, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_mdio_tbl[] = {
    {PAD_RGMII0_MDIO, PADGPIO_BANK, REG_RGMII0_MDIO_GPIO_MODE, REG_RGMII0_MDIO_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_UART5_MODE_3},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_LED0_MODE, REG_LED0_MODE_MASK, BIT1, PINMUX_FOR_LED0_MODE_2},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_MDIO, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info rgmii0_mdc_tbl[] = {
    {PAD_RGMII0_MDC, PADGPIO_BANK, REG_RGMII0_MDC_GPIO_MODE, REG_RGMII0_MDC_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_UART5_MODE, REG_UART5_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_UART5_MODE_3},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_LED1_MODE, REG_LED1_MODE_MASK, BIT5, PINMUX_FOR_LED1_MODE_2},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_RGMII0_MODE, REG_RGMII0_MODE_MASK, BIT0, PINMUX_FOR_RGMII0_MODE_1},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_RMII_MODE, REG_RMII_MODE_MASK, BIT8, PINMUX_FOR_RMII_MODE_1},
    {PAD_RGMII0_MDC, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_gpio0_tbl[] = {
    {PAD_SD1_GPIO0, PADGPIO_BANK, REG_SD1_GPIO0_GPIO_MODE, REG_SD1_GPIO0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_GPIO0, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT1, PINMUX_FOR_I2C1_MODE_2},
    {PAD_SD1_GPIO0, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_UART6_MODE_3},
    {PAD_SD1_GPIO0, PADTOP_BANK, REG_PWM10_MODE, REG_PWM10_MODE_MASK, BIT10, PINMUX_FOR_PWM10_MODE_4},
    {PAD_SD1_GPIO0, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_VD_MODE_3},
    {PAD_SD1_GPIO0, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_I2S0_RX_MODE_3},
    {PAD_SD1_GPIO0, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_3},
    {PAD_SD1_GPIO0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_gpio1_tbl[] = {
    {PAD_SD1_GPIO1, PADGPIO_BANK, REG_SD1_GPIO1_GPIO_MODE, REG_SD1_GPIO1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT1, PINMUX_FOR_I2C1_MODE_2},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_UART6_MODE_3},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_PWM11_MODE, REG_PWM11_MODE_MASK, BIT14, PINMUX_FOR_PWM11_MODE_4},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_VD_MODE, REG_VD_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_VD_MODE_3},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_I2S0_RX_MODE_3},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_3},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_DMIC_6CH_MODE, REG_DMIC_6CH_MODE_MASK, BIT6, PINMUX_FOR_DMIC_6CH_MODE_4},
    {PAD_SD1_GPIO1, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_cdz_tbl[] = {
    {PAD_SD1_CDZ, PADGPIO_BANK, REG_SD1_CDZ_GPIO_MODE, REG_SD1_CDZ_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_CDZ, PADTOP_BANK, REG_SDIO_CDZ_MODE, REG_SDIO_CDZ_MODE_MASK, BIT0, PINMUX_FOR_SDIO_CDZ_MODE_1},
    {PAD_SD1_CDZ, PADTOP_BANK, REG_SDIO_RST_MODE, REG_SDIO_RST_MODE_MASK, BIT4, PINMUX_FOR_SDIO_RST_MODE_1},
    {PAD_SD1_CDZ, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_d1_tbl[] = {
    {PAD_SD1_D1, PADGPIO2_BANK, REG_SD1_D1_GPIO_MODE, REG_SD1_D1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_D1, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT12, PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD1_D1, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_I2S0_RX_MODE_3},
    {PAD_SD1_D1, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_3},
    {PAD_SD1_D1, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_d0_tbl[] = {
    {PAD_SD1_D0, PADGPIO2_BANK, REG_SD1_D0_GPIO_MODE, REG_SD1_D0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_D0, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2, PINMUX_FOR_SPI3_MODE_4},
    {PAD_SD1_D0, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT12, PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD1_D0, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_I2S0_TX_MODE_3},
    {PAD_SD1_D0, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2S0_RXTX_MODE_3},
    {PAD_SD1_D0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_clk_tbl[] = {
    {PAD_SD1_CLK, PADGPIO2_BANK, REG_SD1_CLK_GPIO_MODE, REG_SD1_CLK_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_CLK, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2, PINMUX_FOR_SPI3_MODE_4},
    {PAD_SD1_CLK, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT12, PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD1_CLK, PADTOP_BANK, REG_I2S0_MCK_MODE, REG_I2S0_MCK_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2S0_MCK_MODE_3},
    {PAD_SD1_CLK, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_cmd_tbl[] = {
    {PAD_SD1_CMD, PADGPIO2_BANK, REG_SD1_CMD_GPIO_MODE, REG_SD1_CMD_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_CMD, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2, PINMUX_FOR_SPI3_MODE_4},
    {PAD_SD1_CMD, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT12, PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD1_CMD, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_I2S0_TX_MODE_3},
    {PAD_SD1_CMD, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_d3_tbl[] = {
    {PAD_SD1_D3, PADGPIO2_BANK, REG_SD1_D3_GPIO_MODE, REG_SD1_D3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_D3, PADTOP_BANK, REG_SPI3_MODE, REG_SPI3_MODE_MASK, BIT2, PINMUX_FOR_SPI3_MODE_4},
    {PAD_SD1_D3, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT12, PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD1_D3, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_I2S0_TX_MODE_3},
    {PAD_SD1_D3, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info sd1_d2_tbl[] = {
    {PAD_SD1_D2, PADGPIO2_BANK, REG_SD1_D2_GPIO_MODE, REG_SD1_D2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_D2, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT12, PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD1_D2, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outp_tx0_ch0_tbl[] = {
    {PAD_OUTP_TX0_CH0, PADGPIO2_BANK, REG_OUTP_TX0_CH0_GPIO_MODE, REG_OUTP_TX0_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_I2C2_MODE_5},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT6, PINMUX_FOR_UART1_MODE_4},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_PWM0_MODE, REG_PWM0_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PWM0_MODE_3},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT10, PINMUX_FOR_I2S0_RX_MODE_4},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2, PINMUX_FOR_I2S0_RXTX_MODE_4},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_3},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT2,
     PINMUX_FOR_MIPITX0_OUT_MODE_4},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_OUTP_TX0_CH0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outn_tx0_ch0_tbl[] = {
    {PAD_OUTN_TX0_CH0, PADGPIO2_BANK, REG_OUTN_TX0_CH0_GPIO_MODE, REG_OUTN_TX0_CH0_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_I2C2_MODE, REG_I2C2_MODE_MASK, BIT10 | BIT8, PINMUX_FOR_I2C2_MODE_5},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_UART1_MODE, REG_UART1_MODE_MASK, BIT6, PINMUX_FOR_UART1_MODE_4},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_PWM1_MODE, REG_PWM1_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_PWM1_MODE_3},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT10, PINMUX_FOR_I2S0_RX_MODE_4},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2, PINMUX_FOR_I2S0_RXTX_MODE_4},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_3},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT2,
     PINMUX_FOR_MIPITX0_OUT_MODE_4},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
    {PAD_OUTN_TX0_CH0, PADTOP_BANK, REG_OTP_TEST, REG_OTP_TEST_MASK, BIT8, PINMUX_FOR_OTP_TEST_1},
};
const hal_gpio_st_padmux_info outp_tx0_ch1_tbl[] = {
    {PAD_OUTP_TX0_CH1, PADGPIO2_BANK, REG_OUTP_TX0_CH1_GPIO_MODE, REG_OUTP_TX0_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_UART2_MODE_3},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_PWM2_MODE, REG_PWM2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_PWM2_MODE_3},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_I2S0_RX_MODE, REG_I2S0_RX_MODE_MASK, BIT10, PINMUX_FOR_I2S0_RX_MODE_4},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2, PINMUX_FOR_I2S0_RXTX_MODE_4},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_3},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_OUTP_TX0_CH1, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info outn_tx0_ch1_tbl[] = {
    {PAD_OUTN_TX0_CH1, PADGPIO2_BANK, REG_OUTN_TX0_CH1_GPIO_MODE, REG_OUTN_TX0_CH1_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_UART2_MODE_3},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_SDIO_CDZ_MODE, REG_SDIO_CDZ_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_SDIO_CDZ_MODE_3},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_SDIO_RST_MODE, REG_SDIO_RST_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_SDIO_RST_MODE_3},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_PWM3_MODE, REG_PWM3_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_PWM3_MODE_3},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT14, PINMUX_FOR_I2S0_TX_MODE_4},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_I2S0_RXTX_MODE, REG_I2S0_RXTX_MODE_MASK, BIT2, PINMUX_FOR_I2S0_RXTX_MODE_4},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_3},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_OUTN_TX0_CH1, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info outp_tx0_ch2_tbl[] = {
    {PAD_OUTP_TX0_CH2, PADGPIO2_BANK, REG_OUTP_TX0_CH2_GPIO_MODE, REG_OUTP_TX0_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT14, PINMUX_FOR_UART3_MODE_4},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_SDIO_MODE_3},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_PWM4_MODE, REG_PWM4_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PWM4_MODE_3},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT14, PINMUX_FOR_I2S0_TX_MODE_4},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_3},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT2,
     PINMUX_FOR_MIPITX0_OUT_MODE_4},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTP_TX0_CH2, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info outn_tx0_ch2_tbl[] = {
    {PAD_OUTN_TX0_CH2, PADGPIO2_BANK, REG_OUTN_TX0_CH2_GPIO_MODE, REG_OUTN_TX0_CH2_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_UART3_MODE, REG_UART3_MODE_MASK, BIT14, PINMUX_FOR_UART3_MODE_4},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_SDIO_MODE_3},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_PWM5_MODE, REG_PWM5_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_PWM5_MODE_3},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT14, PINMUX_FOR_I2S0_TX_MODE_4},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1 | BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_3},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT2,
     PINMUX_FOR_MIPITX0_OUT_MODE_4},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTN_TX0_CH2, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info outp_tx0_ch3_tbl[] = {
    {PAD_OUTP_TX0_CH3, PADGPIO2_BANK, REG_OUTP_TX0_CH3_GPIO_MODE, REG_OUTP_TX0_CH3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4 | BIT3, PINMUX_FOR_SPI1_MODE_3},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_SDIO_MODE_3},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_PWM6_MODE, REG_PWM6_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_PWM6_MODE_3},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_I2S0_MCK_MODE, REG_I2S0_MCK_MODE_MASK, BIT2, PINMUX_FOR_I2S0_MCK_MODE_4},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTP_TX0_CH3, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info outn_tx0_ch3_tbl[] = {
    {PAD_OUTN_TX0_CH3, PADGPIO2_BANK, REG_OUTN_TX0_CH3_GPIO_MODE, REG_OUTN_TX0_CH3_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4 | BIT3, PINMUX_FOR_SPI1_MODE_3},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_SDIO_MODE_3},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_PWM7_MODE, REG_PWM7_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_PWM7_MODE_3},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT1,
     PINMUX_FOR_MIPITX0_OUT_MODE_2},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT4, PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_OUTN_TX0_CH3, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info outp_tx0_ch4_tbl[] = {
    {PAD_OUTP_TX0_CH4, PADGPIO2_BANK, REG_OUTP_TX0_CH4_GPIO_MODE, REG_OUTP_TX0_CH4_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4 | BIT3, PINMUX_FOR_SPI1_MODE_3},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_SDIO_MODE_3},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_PWM8_MODE, REG_PWM8_MODE_MASK, BIT2, PINMUX_FOR_PWM8_MODE_4},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_OUTP_TX0_CH4, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info outn_tx0_ch4_tbl[] = {
    {PAD_OUTN_TX0_CH4, PADGPIO2_BANK, REG_OUTN_TX0_CH4_GPIO_MODE, REG_OUTN_TX0_CH4_GPIO_MODE_MASK, BIT3,
     PINMUX_FOR_GPIO_MODE},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_SPI1_MODE, REG_SPI1_MODE_MASK, BIT4 | BIT3, PINMUX_FOR_SPI1_MODE_3},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_SDIO_MODE_3},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_PWM9_MODE, REG_PWM9_MODE_MASK, BIT6, PINMUX_FOR_PWM9_MODE_4},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_MIPITX0_OUT_MODE, REG_MIPITX0_OUT_MODE_MASK, BIT0,
     PINMUX_FOR_MIPITX0_OUT_MODE_1},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_TTL24_MODE, REG_TTL24_MODE_MASK, BIT8, PINMUX_FOR_TTL24_MODE_1},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_TTL18_MODE, REG_TTL18_MODE_MASK, BIT4, PINMUX_FOR_TTL18_MODE_1},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT0, PINMUX_FOR_TTL16_MODE_1},
    {PAD_OUTN_TX0_CH4, PADTOP_BANK, REG_TTL16_MODE, REG_TTL16_MODE_MASK, BIT1, PINMUX_FOR_TTL16_MODE_2},
};
const hal_gpio_st_padmux_info emmc_rstn_tbl[] = {
    {PAD_EMMC_RSTN, PADGPIO2_BANK, REG_EMMC_RSTN_GPIO_MODE, REG_EMMC_RSTN_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_RSTN, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_RSTN, PADTOP_BANK, REG_EMMC4B_BOOT_MODE, REG_EMMC4B_BOOT_MODE_MASK, BIT8, PINMUX_FOR_EMMC4B_BOOT_MODE_1},
    {PAD_EMMC_RSTN, PADTOP_BANK, REG_EMMC_RST_MODE, REG_EMMC_RST_MODE_MASK, BIT4, PINMUX_FOR_EMMC_RST_MODE_1},
    {PAD_EMMC_RSTN, PADTOP_BANK, REG_EMMC_AS_SD_CDZ_MODE, REG_EMMC_AS_SD_CDZ_MODE_MASK, BIT8,
     PINMUX_FOR_EMMC_AS_SD_CDZ_MODE_1},
    {PAD_EMMC_RSTN, PADTOP_BANK, REG_SDIO_CDZ_MODE, REG_SDIO_CDZ_MODE_MASK, BIT1, PINMUX_FOR_SDIO_CDZ_MODE_2},
    {PAD_EMMC_RSTN, PADTOP_BANK, REG_SDIO_RST_MODE, REG_SDIO_RST_MODE_MASK, BIT5, PINMUX_FOR_SDIO_RST_MODE_2},
};
const hal_gpio_st_padmux_info emmc_clk_tbl[] = {
    {PAD_EMMC_CLK, PADGPIO2_BANK, REG_EMMC_CLK_GPIO_MODE, REG_EMMC_CLK_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_CLK, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_CLK, PADTOP_BANK, REG_EMMC4B_BOOT_MODE, REG_EMMC4B_BOOT_MODE_MASK, BIT8, PINMUX_FOR_EMMC4B_BOOT_MODE_1},
    {PAD_EMMC_CLK, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_CLK, PADTOP_BANK, REG_EMMC_4B_MODE, REG_EMMC_4B_MODE_MASK, BIT0, PINMUX_FOR_EMMC_4B_MODE_1},
    {PAD_EMMC_CLK, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13, PINMUX_FOR_SDIO_MODE_2},
};
const hal_gpio_st_padmux_info emmc_cmd_tbl[] = {
    {PAD_EMMC_CMD, PADGPIO2_BANK, REG_EMMC_CMD_GPIO_MODE, REG_EMMC_CMD_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_CMD, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_CMD, PADTOP_BANK, REG_EMMC4B_BOOT_MODE, REG_EMMC4B_BOOT_MODE_MASK, BIT8, PINMUX_FOR_EMMC4B_BOOT_MODE_1},
    {PAD_EMMC_CMD, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_CMD, PADTOP_BANK, REG_EMMC_4B_MODE, REG_EMMC_4B_MODE_MASK, BIT0, PINMUX_FOR_EMMC_4B_MODE_1},
    {PAD_EMMC_CMD, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13, PINMUX_FOR_SDIO_MODE_2},
};
const hal_gpio_st_padmux_info emmc_ds_tbl[] = {
    {PAD_EMMC_DS, PADGPIO2_BANK, REG_EMMC_DS_GPIO_MODE, REG_EMMC_DS_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_DS, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_DS, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
};
const hal_gpio_st_padmux_info emmc_d3_tbl[] = {
    {PAD_EMMC_D3, PADGPIO2_BANK, REG_EMMC_D3_GPIO_MODE, REG_EMMC_D3_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D3, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D3, PADTOP_BANK, REG_EMMC4B_BOOT_MODE, REG_EMMC4B_BOOT_MODE_MASK, BIT8, PINMUX_FOR_EMMC4B_BOOT_MODE_1},
    {PAD_EMMC_D3, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D3, PADTOP_BANK, REG_EMMC_4B_MODE, REG_EMMC_4B_MODE_MASK, BIT0, PINMUX_FOR_EMMC_4B_MODE_1},
    {PAD_EMMC_D3, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13, PINMUX_FOR_SDIO_MODE_2},
};
const hal_gpio_st_padmux_info emmc_d4_tbl[] = {
    {PAD_EMMC_D4, PADGPIO2_BANK, REG_EMMC_D4_GPIO_MODE, REG_EMMC_D4_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D4, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT1, PINMUX_FOR_I2C0_MODE_2},
    {PAD_EMMC_D4, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT10, PINMUX_FOR_UART2_MODE_4},
    {PAD_EMMC_D4, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D4, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D4, PADTOP_BANK, REG_PWM8_MODE, REG_PWM8_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_PWM8_MODE_3},
};
const hal_gpio_st_padmux_info emmc_d0_tbl[] = {
    {PAD_EMMC_D0, PADGPIO2_BANK, REG_EMMC_D0_GPIO_MODE, REG_EMMC_D0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D0, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D0, PADTOP_BANK, REG_EMMC4B_BOOT_MODE, REG_EMMC4B_BOOT_MODE_MASK, BIT8, PINMUX_FOR_EMMC4B_BOOT_MODE_1},
    {PAD_EMMC_D0, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D0, PADTOP_BANK, REG_EMMC_4B_MODE, REG_EMMC_4B_MODE_MASK, BIT0, PINMUX_FOR_EMMC_4B_MODE_1},
    {PAD_EMMC_D0, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13, PINMUX_FOR_SDIO_MODE_2},
};
const hal_gpio_st_padmux_info emmc_d5_tbl[] = {
    {PAD_EMMC_D5, PADGPIO2_BANK, REG_EMMC_D5_GPIO_MODE, REG_EMMC_D5_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D5, PADTOP_BANK, REG_I2C0_MODE, REG_I2C0_MODE_MASK, BIT1, PINMUX_FOR_I2C0_MODE_2},
    {PAD_EMMC_D5, PADTOP_BANK, REG_UART2_MODE, REG_UART2_MODE_MASK, BIT10, PINMUX_FOR_UART2_MODE_4},
    {PAD_EMMC_D5, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D5, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D5, PADTOP_BANK, REG_PWM9_MODE, REG_PWM9_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_PWM9_MODE_3},
};
const hal_gpio_st_padmux_info emmc_d1_tbl[] = {
    {PAD_EMMC_D1, PADGPIO2_BANK, REG_EMMC_D1_GPIO_MODE, REG_EMMC_D1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D1, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D1, PADTOP_BANK, REG_EMMC4B_BOOT_MODE, REG_EMMC4B_BOOT_MODE_MASK, BIT8, PINMUX_FOR_EMMC4B_BOOT_MODE_1},
    {PAD_EMMC_D1, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D1, PADTOP_BANK, REG_EMMC_4B_MODE, REG_EMMC_4B_MODE_MASK, BIT0, PINMUX_FOR_EMMC_4B_MODE_1},
    {PAD_EMMC_D1, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13, PINMUX_FOR_SDIO_MODE_2},
};
const hal_gpio_st_padmux_info emmc_d6_tbl[] = {
    {PAD_EMMC_D6, PADGPIO2_BANK, REG_EMMC_D6_GPIO_MODE, REG_EMMC_D6_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D6, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D6, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D6, PADTOP_BANK, REG_PWM10_MODE, REG_PWM10_MODE_MASK, BIT9 | BIT8, PINMUX_FOR_PWM10_MODE_3},
};
const hal_gpio_st_padmux_info emmc_d2_tbl[] = {
    {PAD_EMMC_D2, PADGPIO2_BANK, REG_EMMC_D2_GPIO_MODE, REG_EMMC_D2_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D2, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D2, PADTOP_BANK, REG_EMMC4B_BOOT_MODE, REG_EMMC4B_BOOT_MODE_MASK, BIT8, PINMUX_FOR_EMMC4B_BOOT_MODE_1},
    {PAD_EMMC_D2, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D2, PADTOP_BANK, REG_EMMC_4B_MODE, REG_EMMC_4B_MODE_MASK, BIT0, PINMUX_FOR_EMMC_4B_MODE_1},
    {PAD_EMMC_D2, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT13, PINMUX_FOR_SDIO_MODE_2},
};
const hal_gpio_st_padmux_info emmc_d7_tbl[] = {
    {PAD_EMMC_D7, PADGPIO2_BANK, REG_EMMC_D7_GPIO_MODE, REG_EMMC_D7_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_EMMC_D7, PADTOP_BANK, REG_EMMC8B_BOOT_MODE, REG_EMMC8B_BOOT_MODE_MASK, BIT0, PINMUX_FOR_EMMC8B_BOOT_MODE_1},
    {PAD_EMMC_D7, PADTOP_BANK, REG_EMMC_8B_MODE, REG_EMMC_8B_MODE_MASK, BIT2, PINMUX_FOR_EMMC_8B_MODE_1},
    {PAD_EMMC_D7, PADTOP_BANK, REG_PWM11_MODE, REG_PWM11_MODE_MASK, BIT13 | BIT12, PINMUX_FOR_PWM11_MODE_3},
};
const hal_gpio_st_padmux_info eth_led0_tbl[] = {
    {PAD_ETH_LED0, PADGPIO2_BANK, REG_ETH_LED0_GPIO_MODE, REG_ETH_LED0_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_ETH_LED0, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_ETH_LED0, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_ETH_LED0, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_ETH_LED0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_ETH_LED0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_ETH_LED0, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_ETH_LED0, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2C1_MODE_3},
    {PAD_ETH_LED0, PADTOP_BANK, REG_UART0_MODE, REG_UART0_MODE_MASK, BIT1, PINMUX_FOR_UART0_MODE_2},
    {PAD_ETH_LED0, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT10, PINMUX_FOR_UART6_MODE_4},
    {PAD_ETH_LED0, PADTOP_BANK, REG_SD0_CDZ_MODE, REG_SD0_CDZ_MODE_MASK, BIT11, PINMUX_FOR_SD0_CDZ_MODE_2},
    {PAD_ETH_LED0, PADTOP_BANK, REG_SD0_RSTN_MODE, REG_SD0_RSTN_MODE_MASK, BIT1, PINMUX_FOR_SD0_RSTN_MODE_2},
    {PAD_ETH_LED0, PADTOP_BANK, REG_PWM0_MODE, REG_PWM0_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_PWM0_MODE_5},
    {PAD_ETH_LED0, PADTOP_BANK, REG_LED0_MODE, REG_LED0_MODE_MASK, BIT0, PINMUX_FOR_LED0_MODE_1},
    {PAD_ETH_LED0, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT12, PINMUX_FOR_I2S0_TX_MODE_1},
    {PAD_ETH_LED0, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_ETH_LED0, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
};
const hal_gpio_st_padmux_info eth_led1_tbl[] = {
    {PAD_ETH_LED1, PADGPIO2_BANK, REG_ETH_LED1_GPIO_MODE, REG_ETH_LED1_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_ETH_LED1, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_ETH_LED1, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_ETH_LED1, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_ETH_LED1, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_ETH_LED1, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5, PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_ETH_LED1, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_ETH_LED1, PADTOP_BANK, REG_I2C1_MODE, REG_I2C1_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_I2C1_MODE_3},
    {PAD_ETH_LED1, PADTOP_BANK, REG_UART0_MODE, REG_UART0_MODE_MASK, BIT1, PINMUX_FOR_UART0_MODE_2},
    {PAD_ETH_LED1, PADTOP_BANK, REG_UART6_MODE, REG_UART6_MODE_MASK, BIT10, PINMUX_FOR_UART6_MODE_4},
    {PAD_ETH_LED1, PADTOP_BANK, REG_SD0_MODE, REG_SD0_MODE_MASK, BIT9, PINMUX_FOR_SD0_MODE_2},
    {PAD_ETH_LED1, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_SDIO_MODE_5},
    {PAD_ETH_LED1, PADTOP_BANK, REG_PWM1_MODE, REG_PWM1_MODE_MASK, BIT6 | BIT4, PINMUX_FOR_PWM1_MODE_5},
    {PAD_ETH_LED1, PADTOP_BANK, REG_LED1_MODE, REG_LED1_MODE_MASK, BIT4, PINMUX_FOR_LED1_MODE_1},
    {PAD_ETH_LED1, PADTOP_BANK, REG_I2S0_TX_MODE, REG_I2S0_TX_MODE_MASK, BIT12, PINMUX_FOR_I2S0_TX_MODE_1},
    {PAD_ETH_LED1, PADTOP_BANK, REG_BT656_OUT_MODE, REG_BT656_OUT_MODE_MASK, BIT5, PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_ETH_LED1, PADTOP_BANK, REG_BT1120_OUT_MODE, REG_BT1120_OUT_MODE_MASK, BIT0, PINMUX_FOR_BT1120_OUT_MODE_1},
};
const hal_gpio_st_padmux_info i2c4_scl_tbl[] = {
    {PAD_I2C4_SCL, PADGPIO2_BANK, REG_I2C4_SCL_GPIO_MODE, REG_I2C4_SCL_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C4_SCL, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT9, PINMUX_FOR_EJ_MODE_4},
    {PAD_I2C4_SCL, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2C4_SCL, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_I2C4_SCL, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_I2C4_SCL, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_I2C4_SCL, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT4, PINMUX_FOR_I2C4_MODE_1},
#endif
};
const hal_gpio_st_padmux_info i2c4_sda_tbl[] = {
    {PAD_I2C4_SDA, PADGPIO2_BANK, REG_I2C4_SDA_GPIO_MODE, REG_I2C4_SDA_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_I2C4_SDA, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT9, PINMUX_FOR_EJ_MODE_4},
    {PAD_I2C4_SDA, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2C4_SDA, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_I2C4_SDA, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_I2C4_SDA, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_I2C4_SDA, TZMISC_BANK, REG_I2C4_MODE, REG_I2C4_MODE_MASK, BIT4, PINMUX_FOR_I2C4_MODE_1},
#endif
};
const hal_gpio_st_padmux_info spi_ck_tbl[] = {
    {PAD_SPI_CK, PADGPIO2_BANK, REG_SPI_CK_GPIO_MODE, REG_SPI_CK_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_CK, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SPI_CK, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SPI_CK, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_SPI_CK, TZMISC_BANK, REG_QSPI_MODE, REG_QSPI_MODE_MASK, 0, PINMUX_FOR_QSPI_MODE_0},
#endif
    {PAD_SPI_CK, PADTOP_BANK, REG_SPI_EXT_EN_MODE0, REG_SPI_EXT_EN_MODE0_MASK, BIT0, PINMUX_FOR_SPI_EXT_EN_MODE0_1},
};
const hal_gpio_st_padmux_info spi_di_tbl[] = {
    {PAD_SPI_DI, PADGPIO2_BANK, REG_SPI_DI_GPIO_MODE, REG_SPI_DI_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_DI, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SPI_DI, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SPI_DI, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_SPI_DI, TZMISC_BANK, REG_QSPI_MODE, REG_QSPI_MODE_MASK, 0, PINMUX_FOR_QSPI_MODE_0},
#endif
    {PAD_SPI_DI, PADTOP_BANK, REG_SPI_EXT_EN_MODE1, REG_SPI_EXT_EN_MODE1_MASK, BIT1, PINMUX_FOR_SPI_EXT_EN_MODE1_1},
};
const hal_gpio_st_padmux_info spi_hld_tbl[] = {
    {PAD_SPI_HLD, PADGPIO2_BANK, REG_SPI_HLD_GPIO_MODE, REG_SPI_HLD_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_HLD, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SPI_HLD, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_SPI_HLD, TZMISC_BANK, REG_QSPI_MODE, REG_QSPI_MODE_MASK, 0, PINMUX_FOR_QSPI_MODE_0},
#endif
    {PAD_SPI_HLD, PADTOP_BANK, REG_SPI_EXT_EN_MODE2, REG_SPI_EXT_EN_MODE2_MASK, BIT2, PINMUX_FOR_SPI_EXT_EN_MODE2_1},
};
const hal_gpio_st_padmux_info spi_do_tbl[] = {
    {PAD_SPI_DO, PADGPIO2_BANK, REG_SPI_DO_GPIO_MODE, REG_SPI_DO_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_DO, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SPI_DO, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_SPI_DO, TZMISC_BANK, REG_QSPI_MODE, REG_QSPI_MODE_MASK, 0, PINMUX_FOR_QSPI_MODE_0},
#endif
    {PAD_SPI_DO, PADTOP_BANK, REG_SPI_EXT_EN_MODE3, REG_SPI_EXT_EN_MODE3_MASK, BIT3, PINMUX_FOR_SPI_EXT_EN_MODE3_1},
};
const hal_gpio_st_padmux_info spi_wpz_tbl[] = {
    {PAD_SPI_WPZ, PADGPIO2_BANK, REG_SPI_WPZ_GPIO_MODE, REG_SPI_WPZ_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_WPZ, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SPI_WPZ, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_SPI_WPZ, TZMISC_BANK, REG_QSPI_MODE, REG_QSPI_MODE_MASK, 0, PINMUX_FOR_QSPI_MODE_0},
#endif
    {PAD_SPI_WPZ, PADTOP_BANK, REG_SPI_EXT_EN_MODE4, REG_SPI_EXT_EN_MODE4_MASK, BIT4, PINMUX_FOR_SPI_EXT_EN_MODE4_1},
};
const hal_gpio_st_padmux_info spi_cz_tbl[] = {
    {PAD_SPI_CZ, PADGPIO2_BANK, REG_SPI_CZ_GPIO_MODE, REG_SPI_CZ_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_CZ, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SPI_CZ, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
#ifndef CONFIG_OPTEE
    {PAD_SPI_CZ, TZMISC_BANK, REG_QSPI_MODE, REG_QSPI_MODE_MASK, 0, PINMUX_FOR_QSPI_MODE_0},
#endif
    {PAD_SPI_CZ, PADTOP_BANK, REG_SPI_EXT_EN_MODE5, REG_SPI_EXT_EN_MODE5_MASK, BIT5, PINMUX_FOR_SPI_EXT_EN_MODE5_1},
};
const hal_gpio_st_padmux_info uart_rx_tbl[] = {
    {PAD_UART_RX, PADGPIO2_BANK, REG_UART_RX_GPIO_MODE, REG_UART_RX_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_UART_RX, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT9, PINMUX_FOR_EJ_MODE_4},
    {PAD_UART_RX, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT0, PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_UART_RX, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1, PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_UART_RX, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_UART_RX, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_UART_RX, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_UART_RX, PADTOP_BANK, REG_UART0_MODE, REG_UART0_MODE_MASK, BIT0, PINMUX_FOR_UART0_MODE_1},
    {PAD_UART_RX, PADTOP_BANK, REG_SDIO_MODE, REG_SDIO_MODE_MASK, BIT14 | BIT12, PINMUX_FOR_SDIO_MODE_5},
    {PAD_UART_RX, PADTOP_BANK, REG_QSPICSZ2_MODE, REG_QSPICSZ2_MODE_MASK, BIT4, PINMUX_FOR_QSPICSZ2_MODE_1},
};
const hal_gpio_st_padmux_info uart_tx_tbl[] = {
    {PAD_UART_TX, PADGPIO2_BANK, REG_UART_TX_GPIO_MODE, REG_UART_TX_GPIO_MODE_MASK, BIT3, PINMUX_FOR_GPIO_MODE},
    {PAD_UART_TX, PADTOP_BANK, REG_EJ_MODE, REG_EJ_MODE_MASK, BIT9, PINMUX_FOR_EJ_MODE_4},
    {PAD_UART_TX, CHIPTOP_BANK, REG_TEST_IN_MODE, REG_TEST_IN_MODE_MASK, BIT1 | BIT0, PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_UART_TX, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT4, PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_UART_TX, CHIPTOP_BANK, REG_TEST_OUT_MODE, REG_TEST_OUT_MODE_MASK, BIT5 | BIT4, PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_UART_TX, PADTOP_BANK, REG_UART0_MODE, REG_UART0_MODE_MASK, BIT0, PINMUX_FOR_UART0_MODE_1},
    {PAD_UART_TX, PADTOP_BANK, REG_SDIO_CDZ_MODE, REG_SDIO_CDZ_MODE_MASK, BIT2 | BIT0, PINMUX_FOR_SDIO_CDZ_MODE_5},
    {PAD_UART_TX, PADTOP_BANK, REG_SDIO_RST_MODE, REG_SDIO_RST_MODE_MASK, BIT6 | BIT4, PINMUX_FOR_SDIO_RST_MODE_5},
};

const hal_gpio_st_padmux_en m_hal_gpio_st_padmux_entry[] = {
    {(sizeof(pwm_out0_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out0_tbl},
    {(sizeof(pwm_out1_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out1_tbl},
    {(sizeof(pwm_out2_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out2_tbl},
    {(sizeof(pwm_out3_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out3_tbl},
    {(sizeof(pwm_out4_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out4_tbl},
    {(sizeof(pwm_out5_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out5_tbl},
    {(sizeof(pwm_out6_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out6_tbl},
    {(sizeof(pwm_out7_tbl) / sizeof(hal_gpio_st_padmux_info)), pwm_out7_tbl},
    {(sizeof(sd0_gpio0_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_gpio0_tbl},
    {(sizeof(sd0_vctrl_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_vctrl_tbl},
    {(sizeof(sd0_cdz_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_cdz_tbl},
    {(sizeof(sd0_d1_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_d1_tbl},
    {(sizeof(sd0_d0_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_d0_tbl},
    {(sizeof(sd0_clk_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_clk_tbl},
    {(sizeof(sd0_cmd_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_cmd_tbl},
    {(sizeof(sd0_d3_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_d3_tbl},
    {(sizeof(sd0_d2_tbl) / sizeof(hal_gpio_st_padmux_info)), sd0_d2_tbl},
    {(sizeof(i2c0_scl_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c0_scl_tbl},
    {(sizeof(i2c0_sda_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c0_sda_tbl},
    {(sizeof(sr_rst0_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_rst0_tbl},
    {(sizeof(sr_mclk0_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_mclk0_tbl},
    {(sizeof(i2c1_scl_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c1_scl_tbl},
    {(sizeof(i2c1_sda_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c1_sda_tbl},
    {(sizeof(sr_rst1_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_rst1_tbl},
    {(sizeof(sr_mclk1_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_mclk1_tbl},
    {(sizeof(outp_rx0_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx0_ch0_tbl},
    {(sizeof(outn_rx0_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx0_ch0_tbl},
    {(sizeof(outp_rx0_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx0_ch1_tbl},
    {(sizeof(outn_rx0_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx0_ch1_tbl},
    {(sizeof(outp_rx0_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx0_ch2_tbl},
    {(sizeof(outn_rx0_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx0_ch2_tbl},
    {(sizeof(outp_rx0_ch3_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx0_ch3_tbl},
    {(sizeof(outn_rx0_ch3_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx0_ch3_tbl},
    {(sizeof(outp_rx0_ch4_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx0_ch4_tbl},
    {(sizeof(outn_rx0_ch4_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx0_ch4_tbl},
    {(sizeof(outp_rx0_ch5_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx0_ch5_tbl},
    {(sizeof(outn_rx0_ch5_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx0_ch5_tbl},
    {(sizeof(outp_rx1_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx1_ch0_tbl},
    {(sizeof(outn_rx1_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx1_ch0_tbl},
    {(sizeof(outp_rx1_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx1_ch1_tbl},
    {(sizeof(outn_rx1_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx1_ch1_tbl},
    {(sizeof(outp_rx1_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx1_ch2_tbl},
    {(sizeof(outn_rx1_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx1_ch2_tbl},
    {(sizeof(outp_rx1_ch3_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx1_ch3_tbl},
    {(sizeof(outn_rx1_ch3_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx1_ch3_tbl},
    {(sizeof(outp_rx1_ch4_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx1_ch4_tbl},
    {(sizeof(outn_rx1_ch4_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx1_ch4_tbl},
    {(sizeof(outp_rx2_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx2_ch0_tbl},
    {(sizeof(outn_rx2_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx2_ch0_tbl},
    {(sizeof(outp_rx2_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx2_ch1_tbl},
    {(sizeof(outn_rx2_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx2_ch1_tbl},
    {(sizeof(outp_rx2_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_rx2_ch2_tbl},
    {(sizeof(outn_rx2_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_rx2_ch2_tbl},
    {(sizeof(i2c2_scl_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c2_scl_tbl},
    {(sizeof(i2c2_sda_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c2_sda_tbl},
    {(sizeof(sr_rst2_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_rst2_tbl},
    {(sizeof(sr_mclk2_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_mclk2_tbl},
    {(sizeof(i2c3_scl_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c3_scl_tbl},
    {(sizeof(i2c3_sda_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c3_sda_tbl},
    {(sizeof(sr_rst3_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_rst3_tbl},
    {(sizeof(sr_mclk3_tbl) / sizeof(hal_gpio_st_padmux_info)), sr_mclk3_tbl},
    {(sizeof(isp0_xvs_tbl) / sizeof(hal_gpio_st_padmux_info)), isp0_xvs_tbl},
    {(sizeof(isp0_xhs_tbl) / sizeof(hal_gpio_st_padmux_info)), isp0_xhs_tbl},
    {(sizeof(isp0_xtrig_tbl) / sizeof(hal_gpio_st_padmux_info)), isp0_xtrig_tbl},
    {(sizeof(pm_sr1_d0_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d0_tbl},
    {(sizeof(pm_sr1_d4_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d4_tbl},
    {(sizeof(pm_sr1_pclk_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_pclk_tbl},
    {(sizeof(pm_sr1_rst_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_rst_tbl},
    {(sizeof(pm_sr1_int_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_int_tbl},
    {(sizeof(pm_mi2c1_scl_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_mi2c1_scl_tbl},
    {(sizeof(pm_mi2c1_sda_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_mi2c1_sda_tbl},
    {(sizeof(pm_sr1_d1_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d1_tbl},
    {(sizeof(pm_sr1_d2_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d2_tbl},
    {(sizeof(pm_sr1_d3_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d3_tbl},
    {(sizeof(pm_sr1_d5_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d5_tbl},
    {(sizeof(pm_sr1_d6_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d6_tbl},
    {(sizeof(pm_sr1_d7_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_d7_tbl},
    {(sizeof(pm_sr1_vs_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_vs_tbl},
    {(sizeof(pm_sr1_hs_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_sr1_hs_tbl},
    {(sizeof(pm_dmic0_clk_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_dmic0_clk_tbl},
    {(sizeof(pm_dmic0_d0_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_dmic0_d0_tbl},
    {(sizeof(pm_spi_ck_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_spi_ck_tbl},
    {(sizeof(pm_spi_di_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_spi_di_tbl},
    {(sizeof(pm_spi_hld_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_spi_hld_tbl},
    {(sizeof(pm_spi_do_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_spi_do_tbl},
    {(sizeof(pm_spi_wpz_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_spi_wpz_tbl},
    {(sizeof(pm_spi_cz_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_spi_cz_tbl},
    {(sizeof(pm_radar_sar_gpio0_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_radar_sar_gpio0_tbl},
    {(sizeof(pm_radar_sar_gpio1_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_radar_sar_gpio1_tbl},
    {(sizeof(pm_radar_sar_gpio2_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_radar_sar_gpio2_tbl},
    {(sizeof(pm_radar_sar_gpio3_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_radar_sar_gpio3_tbl},
    {(sizeof(pm_uart_rx_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_uart_rx_tbl},
    {(sizeof(pm_uart_tx_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_uart_tx_tbl},
    {(sizeof(pm_intout_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_intout_tbl},
    {(sizeof(pm_gpio0_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_gpio0_tbl},
    {(sizeof(pm_gpio1_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_gpio1_tbl},
    {(sizeof(pm_gpio2_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_gpio2_tbl},
    {(sizeof(pm_gpio3_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_gpio3_tbl},
    {(sizeof(pm_gpio4_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_gpio4_tbl},
    {(sizeof(pm_gpio5_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_gpio5_tbl},
    {(sizeof(pm_fuart_rx_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_fuart_rx_tbl},
    {(sizeof(pm_fuart_tx_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_fuart_tx_tbl},
    {(sizeof(pm_irin_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_irin_tbl},
    {(sizeof(pm_mspi0_do_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_mspi0_do_tbl},
    {(sizeof(pm_mspi0_di_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_mspi0_di_tbl},
    {(sizeof(pm_mspi0_ck_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_mspi0_ck_tbl},
    {(sizeof(pm_mspi0_cz_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_mspi0_cz_tbl},
    {(sizeof(pm_usb3_int_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_usb3_int_tbl},
    {(sizeof(pm_usb3_id_tbl) / sizeof(hal_gpio_st_padmux_info)), pm_usb3_id_tbl},
    {(sizeof(sar_gpio0_tbl) / sizeof(hal_gpio_st_padmux_info)), sar_gpio0_tbl},
    {(sizeof(sar_gpio1_tbl) / sizeof(hal_gpio_st_padmux_info)), sar_gpio1_tbl},
    {(sizeof(rgmii0_mclk_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_mclk_tbl},
    {(sizeof(rgmii0_rstn_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_rstn_tbl},
    {(sizeof(rgmii0_rxclk_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_rxclk_tbl},
    {(sizeof(rgmii0_rxctl_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_rxctl_tbl},
    {(sizeof(rgmii0_rxd0_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_rxd0_tbl},
    {(sizeof(rgmii0_rxd1_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_rxd1_tbl},
    {(sizeof(rgmii0_rxd2_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_rxd2_tbl},
    {(sizeof(rgmii0_rxd3_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_rxd3_tbl},
    {(sizeof(rgmii0_txclk_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_txclk_tbl},
    {(sizeof(rgmii0_txctl_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_txctl_tbl},
    {(sizeof(rgmii0_txd0_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_txd0_tbl},
    {(sizeof(rgmii0_txd1_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_txd1_tbl},
    {(sizeof(rgmii0_txd2_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_txd2_tbl},
    {(sizeof(rgmii0_txd3_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_txd3_tbl},
    {(sizeof(rgmii0_mdio_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_mdio_tbl},
    {(sizeof(rgmii0_mdc_tbl) / sizeof(hal_gpio_st_padmux_info)), rgmii0_mdc_tbl},
    {(sizeof(sd1_gpio0_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_gpio0_tbl},
    {(sizeof(sd1_gpio1_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_gpio1_tbl},
    {(sizeof(sd1_cdz_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_cdz_tbl},
    {(sizeof(sd1_d1_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_d1_tbl},
    {(sizeof(sd1_d0_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_d0_tbl},
    {(sizeof(sd1_clk_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_clk_tbl},
    {(sizeof(sd1_cmd_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_cmd_tbl},
    {(sizeof(sd1_d3_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_d3_tbl},
    {(sizeof(sd1_d2_tbl) / sizeof(hal_gpio_st_padmux_info)), sd1_d2_tbl},
    {(sizeof(outp_tx0_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_tx0_ch0_tbl},
    {(sizeof(outn_tx0_ch0_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_tx0_ch0_tbl},
    {(sizeof(outp_tx0_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_tx0_ch1_tbl},
    {(sizeof(outn_tx0_ch1_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_tx0_ch1_tbl},
    {(sizeof(outp_tx0_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_tx0_ch2_tbl},
    {(sizeof(outn_tx0_ch2_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_tx0_ch2_tbl},
    {(sizeof(outp_tx0_ch3_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_tx0_ch3_tbl},
    {(sizeof(outn_tx0_ch3_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_tx0_ch3_tbl},
    {(sizeof(outp_tx0_ch4_tbl) / sizeof(hal_gpio_st_padmux_info)), outp_tx0_ch4_tbl},
    {(sizeof(outn_tx0_ch4_tbl) / sizeof(hal_gpio_st_padmux_info)), outn_tx0_ch4_tbl},
    {(sizeof(emmc_rstn_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_rstn_tbl},
    {(sizeof(emmc_clk_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_clk_tbl},
    {(sizeof(emmc_cmd_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_cmd_tbl},
    {(sizeof(emmc_ds_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_ds_tbl},
    {(sizeof(emmc_d3_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d3_tbl},
    {(sizeof(emmc_d4_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d4_tbl},
    {(sizeof(emmc_d0_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d0_tbl},
    {(sizeof(emmc_d5_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d5_tbl},
    {(sizeof(emmc_d1_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d1_tbl},
    {(sizeof(emmc_d6_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d6_tbl},
    {(sizeof(emmc_d2_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d2_tbl},
    {(sizeof(emmc_d7_tbl) / sizeof(hal_gpio_st_padmux_info)), emmc_d7_tbl},
    {(sizeof(eth_led0_tbl) / sizeof(hal_gpio_st_padmux_info)), eth_led0_tbl},
    {(sizeof(eth_led1_tbl) / sizeof(hal_gpio_st_padmux_info)), eth_led1_tbl},
    {(sizeof(i2c4_scl_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c4_scl_tbl},
    {(sizeof(i2c4_sda_tbl) / sizeof(hal_gpio_st_padmux_info)), i2c4_sda_tbl},
    {(sizeof(spi_ck_tbl) / sizeof(hal_gpio_st_padmux_info)), spi_ck_tbl},
    {(sizeof(spi_di_tbl) / sizeof(hal_gpio_st_padmux_info)), spi_di_tbl},
    {(sizeof(spi_hld_tbl) / sizeof(hal_gpio_st_padmux_info)), spi_hld_tbl},
    {(sizeof(spi_do_tbl) / sizeof(hal_gpio_st_padmux_info)), spi_do_tbl},
    {(sizeof(spi_wpz_tbl) / sizeof(hal_gpio_st_padmux_info)), spi_wpz_tbl},
    {(sizeof(spi_cz_tbl) / sizeof(hal_gpio_st_padmux_info)), spi_cz_tbl},
    {(sizeof(uart_rx_tbl) / sizeof(hal_gpio_st_padmux_info)), uart_rx_tbl},
    {(sizeof(uart_tx_tbl) / sizeof(hal_gpio_st_padmux_info)), uart_tx_tbl},
};

static const hal_gpio_st_padmode_info m_hal_gpio_st_padmode_info_tbl[] = {
    {"GPIO_MODE", 0, 0, 0},
    {"EJ_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_EJ_MODE), REG_EJ_MODE_MASK, BIT7},
    {"EJ_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_EJ_MODE), REG_EJ_MODE_MASK, BIT8},
    {"EJ_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_EJ_MODE), REG_EJ_MODE_MASK, BIT8 | BIT7},
    {"EJ_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_EJ_MODE), REG_EJ_MODE_MASK, BIT9},
    {"CM4_EJ_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_CM4_EJ_MODE), REG_CM4_EJ_MODE_MASK, BIT0},
    {"CM4_EJ_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_CM4_EJ_MODE), REG_CM4_EJ_MODE_MASK, BIT1},
    {"CM4_EJ_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_CM4_EJ_MODE), REG_CM4_EJ_MODE_MASK, BIT1 | BIT0},
    {"PM_TEST_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_TEST_MODE), REG_PM_TEST_MODE_MASK, BIT0},
    {"PM_TEST_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_TEST_MODE), REG_PM_TEST_MODE_MASK, BIT1},
    {"PM_TEST_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_TEST_MODE), REG_PM_TEST_MODE_MASK, BIT1 | BIT0},
    {"IR_IN_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_IR_IN_MODE), REG_IR_IN_MODE_MASK, BIT0},
    {"IR_IN_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_IR_IN_MODE), REG_IR_IN_MODE_MASK, BIT1},
    {"IR_IN_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_IR_IN_MODE), REG_IR_IN_MODE_MASK, BIT1 | BIT0},
    {"VBUS_DET_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_VBUS_DET_MODE), REG_VBUS_DET_MODE_MASK, BIT4},
    {"RADAR_20M_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_RADAR_20M_MODE), REG_RADAR_20M_MODE_MASK, BIT1},
    {"I2CM0_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2CM0_MODE), REG_I2CM0_MODE_MASK, BIT0},
    {"I2CM0_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2CM0_MODE), REG_I2CM0_MODE_MASK, BIT1},
    {"I2CM1_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2CM1_MODE), REG_I2CM1_MODE_MASK, BIT2},
    {"I2CM1_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2CM1_MODE), REG_I2CM1_MODE_MASK, BIT3},
    {"PM_SPI0_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_SPI0_MODE), REG_PM_SPI0_MODE_MASK, BIT10},
    {"PM_SPI0_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_SPI0_MODE), REG_PM_SPI0_MODE_MASK, BIT11},
    {"PM_SPI0_CZ1_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_SPI0_CZ1_MODE), REG_PM_SPI0_CZ1_MODE_MASK, BIT8},
    {"PM_SPI0_CZ1_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_SPI0_CZ1_MODE), REG_PM_SPI0_CZ1_MODE_MASK, BIT9},
    {"MSPI1_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_MSPI1_MODE), REG_MSPI1_MODE_MASK, BIT6},
    {"MSPI1_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_MSPI1_MODE), REG_MSPI1_MODE_MASK, BIT7},
    {"MSPI1_CZ1_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_MSPI1_CZ1_MODE), REG_MSPI1_CZ1_MODE_MASK, BIT4},
    {"MSPI1_CZ1_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_MSPI1_CZ1_MODE), REG_MSPI1_CZ1_MODE_MASK, BIT5},
    {"SSPI0_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSPI0_MODE), REG_SSPI0_MODE_MASK, BIT8},
    {"SSPI0_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSPI0_MODE), REG_SSPI0_MODE_MASK, BIT9},
    {"SSPI0_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSPI0_MODE), REG_SSPI0_MODE_MASK, BIT9 | BIT8},
    {"PSPI0_SLAVE_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PSPI0_SLAVE_MODE), REG_PSPI0_SLAVE_MODE_MASK, BIT1},
    {"PSPI0_VSYNC_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PSPI0_VSYNC_MODE), REG_PSPI0_VSYNC_MODE_MASK, BIT4},
    {"PM_FUART_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_FUART_MODE), REG_PM_FUART_MODE_MASK, BIT0},
    {"PM_FUART_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_FUART_MODE), REG_PM_FUART_MODE_MASK, BIT1},
    {"PM_FUART_2W_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_FUART_2W_MODE), REG_PM_FUART_2W_MODE_MASK, BIT4},
    {"PM_FUART_2W_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_FUART_2W_MODE), REG_PM_FUART_2W_MODE_MASK, BIT5},
    {"PM_FUART_2W_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_FUART_2W_MODE), REG_PM_FUART_2W_MODE_MASK, BIT5 | BIT4},
    {"PM_FUART_2W_MODE_4", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_FUART_2W_MODE), REG_PM_FUART_2W_MODE_MASK, BIT6},
    {"PM_FUART_2W_MODE_5", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_FUART_2W_MODE), REG_PM_FUART_2W_MODE_MASK, BIT6 | BIT4},
    {"PM_UART0_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_UART0_MODE), REG_PM_UART0_MODE_MASK, BIT8},
    {"PM_PWM0_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM0_MODE), REG_PM_PWM0_MODE_MASK, BIT0},
    {"PM_PWM0_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM0_MODE), REG_PM_PWM0_MODE_MASK, BIT1},
    {"PM_PWM0_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM0_MODE), REG_PM_PWM0_MODE_MASK, BIT1 | BIT0},
    {"PM_PWM0_MODE_4", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM0_MODE), REG_PM_PWM0_MODE_MASK, BIT2},
    {"PM_PWM1_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM1_MODE), REG_PM_PWM1_MODE_MASK, BIT4},
    {"PM_PWM1_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM1_MODE), REG_PM_PWM1_MODE_MASK, BIT5},
    {"PM_PWM1_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM1_MODE), REG_PM_PWM1_MODE_MASK, BIT5 | BIT4},
    {"PM_PWM1_MODE_4", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PWM1_MODE), REG_PM_PWM1_MODE_MASK, BIT6},
    {"PM_DMIC_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_DMIC_MODE), REG_PM_DMIC_MODE_MASK, BIT8},
    {"PM_DMIC_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_DMIC_MODE), REG_PM_DMIC_MODE_MASK, BIT9},
    {"PIR_DIRLK0_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK0_MODE), REG_PIR_DIRLK0_MODE_MASK, BIT0},
    {"PIR_DIRLK0_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK0_MODE), REG_PIR_DIRLK0_MODE_MASK, BIT1},
    {"PIR_DIRLK0_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK0_MODE), REG_PIR_DIRLK0_MODE_MASK, BIT1 | BIT0},
    {"PIR_DIRLK1_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK1_MODE), REG_PIR_DIRLK1_MODE_MASK, BIT4},
    {"PIR_DIRLK1_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK1_MODE), REG_PIR_DIRLK1_MODE_MASK, BIT5},
    {"PIR_DIRLK1_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK1_MODE), REG_PIR_DIRLK1_MODE_MASK, BIT5 | BIT4},
    {"PIR_DIRLK2_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK2_MODE), REG_PIR_DIRLK2_MODE_MASK, BIT8},
    {"PIR_DIRLK2_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK2_MODE), REG_PIR_DIRLK2_MODE_MASK, BIT9},
    {"PIR_DIRLK2_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK2_MODE), REG_PIR_DIRLK2_MODE_MASK, BIT9 | BIT8},
    {"PIR_DIRLK3_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE), REG_PIR_DIRLK3_MODE_MASK, BIT12},
    {"PIR_DIRLK3_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE), REG_PIR_DIRLK3_MODE_MASK, BIT13},
    {"PIR_DIRLK3_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_DIRLK3_MODE), REG_PIR_DIRLK3_MODE_MASK, BIT13 | BIT12},
    {"PIR_SERIAL0_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_SERIAL0_MODE), REG_PIR_SERIAL0_MODE_MASK, BIT8},
    {"PIR_SERIAL0_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_SERIAL0_MODE), REG_PIR_SERIAL0_MODE_MASK, BIT9},
    {"PIR_SERIAL0_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_PIR_SERIAL0_MODE), REG_PIR_SERIAL0_MODE_MASK, BIT9 | BIT8},
    {"I2S_MCLK_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2S_MCLK_MODE), REG_I2S_MCLK_MODE_MASK, BIT4},
    {"I2S_MCLK_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2S_MCLK_MODE), REG_I2S_MCLK_MODE_MASK, BIT5},
    {"I2S_RX_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2S_RX_MODE), REG_I2S_RX_MODE_MASK, BIT0},
    {"I2S_RX_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_I2S_RX_MODE), REG_I2S_RX_MODE_MASK, BIT1},
    {"INT_O_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_INT_O_MODE), REG_INT_O_MODE_MASK, BIT8},
    {"DVP_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_DVP_MODE), REG_DVP_MODE_MASK, BIT8},
    {"SSI_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSI_MODE), REG_SSI_MODE_MASK, BIT0},
    {"SSI_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSI_MODE), REG_SSI_MODE_MASK, BIT1},
    {"SSI_MODE_3", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSI_MODE), REG_SSI_MODE_MASK, BIT1 | BIT0},
    {"SSI_MODE_4", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSI_MODE), REG_SSI_MODE_MASK, BIT2},
    {"SSI_CS_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSI_CS_MODE), REG_SSI_CS_MODE_MASK, BIT4},
    {"SSI_CS_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_SSI_CS_MODE), REG_SSI_CS_MODE_MASK, BIT5},
    {"SR_RST_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_SR_RST_MODE), REG_SR_RST_MODE_MASK, BIT4},
    {"SR_PD_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_SR_PD_MODE), REG_SR_PD_MODE_MASK, BIT8},
    {"SR_PD_MODE_2", _RIUA_16BIT(PM_PADTOP_BANK, REG_SR_PD_MODE), REG_SR_PD_MODE_MASK, BIT9},
    {"SR_MCLK_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_SR_MCLK_MODE), REG_SR_MCLK_MODE_MASK, BIT0},
    {"PM_PAD_EXT_MODE0_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE0), REG_PM_PAD_EXT_MODE0_MASK, BIT0},
    {"PM_PAD_EXT_MODE1_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE1), REG_PM_PAD_EXT_MODE1_MASK, BIT1},
    {"PM_PAD_EXT_MODE2_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE2), REG_PM_PAD_EXT_MODE2_MASK, BIT2},
    {"PM_PAD_EXT_MODE3_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE3), REG_PM_PAD_EXT_MODE3_MASK, BIT3},
    {"PM_PAD_EXT_MODE4_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE4), REG_PM_PAD_EXT_MODE4_MASK, BIT4},
    {"PM_PAD_EXT_MODE5_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE5), REG_PM_PAD_EXT_MODE5_MASK, BIT5},
    {"PM_PAD_EXT_MODE6_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE6), REG_PM_PAD_EXT_MODE6_MASK, BIT6},
    {"PM_PAD_EXT_MODE7_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE7), REG_PM_PAD_EXT_MODE7_MASK, BIT7},
    {"PM_PAD_EXT_MODE8_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE8), REG_PM_PAD_EXT_MODE8_MASK, BIT8},
    {"PM_PAD_EXT_MODE9_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE9), REG_PM_PAD_EXT_MODE9_MASK, BIT9},
    {"PM_PAD_EXT_MODE10_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE10), REG_PM_PAD_EXT_MODE10_MASK, BIT10},
    {"PM_PAD_EXT_MODE11_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE11), REG_PM_PAD_EXT_MODE11_MASK, BIT11},
    {"PM_PAD_EXT_MODE12_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE12), REG_PM_PAD_EXT_MODE12_MASK, BIT12},
    {"PM_PAD_EXT_MODE13_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE13), REG_PM_PAD_EXT_MODE13_MASK, BIT13},
    {"PM_PAD_EXT_MODE14_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE14), REG_PM_PAD_EXT_MODE14_MASK, BIT14},
    {"PM_PAD_EXT_MODE15_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE15), REG_PM_PAD_EXT_MODE15_MASK, BIT15},
    {"PM_PAD_EXT_MODE16_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE16), REG_PM_PAD_EXT_MODE16_MASK, BIT0},
    {"PM_PAD_EXT_MODE17_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE17), REG_PM_PAD_EXT_MODE17_MASK, BIT1},
    {"PM_PAD_EXT_MODE18_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE18), REG_PM_PAD_EXT_MODE18_MASK, BIT2},
    {"PM_PAD_EXT_MODE19_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE19), REG_PM_PAD_EXT_MODE19_MASK, BIT3},
    {"PM_PAD_EXT_MODE20_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE20), REG_PM_PAD_EXT_MODE20_MASK, BIT4},
    {"PM_PAD_EXT_MODE21_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE21), REG_PM_PAD_EXT_MODE21_MASK, BIT5},
    {"PM_PAD_EXT_MODE22_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE22), REG_PM_PAD_EXT_MODE22_MASK, BIT6},
    {"PM_PAD_EXT_MODE23_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE23), REG_PM_PAD_EXT_MODE23_MASK, BIT7},
    {"PM_PAD_EXT_MODE24_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE24), REG_PM_PAD_EXT_MODE24_MASK, BIT8},
    {"PM_PAD_EXT_MODE25_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE25), REG_PM_PAD_EXT_MODE25_MASK, BIT9},
    {"PM_PAD_EXT_MODE26_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE26), REG_PM_PAD_EXT_MODE26_MASK, BIT10},
    {"PM_PAD_EXT_MODE27_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE27), REG_PM_PAD_EXT_MODE27_MASK, BIT11},
    {"PM_PAD_EXT_MODE28_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE28), REG_PM_PAD_EXT_MODE28_MASK, BIT12},
    {"PM_PAD_EXT_MODE29_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE29), REG_PM_PAD_EXT_MODE29_MASK, BIT13},
    {"PM_PAD_EXT_MODE30_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE30), REG_PM_PAD_EXT_MODE30_MASK, BIT14},
    {"PM_PAD_EXT_MODE31_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE31), REG_PM_PAD_EXT_MODE31_MASK, BIT15},
    {"PM_PAD_EXT_MODE32_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE32), REG_PM_PAD_EXT_MODE32_MASK, BIT0},
    {"PM_PAD_EXT_MODE33_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE33), REG_PM_PAD_EXT_MODE33_MASK, BIT1},
    {"PM_PAD_EXT_MODE34_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE34), REG_PM_PAD_EXT_MODE34_MASK, BIT2},
    {"PM_PAD_EXT_MODE35_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE35), REG_PM_PAD_EXT_MODE35_MASK, BIT3},
    {"PM_PAD_EXT_MODE36_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE36), REG_PM_PAD_EXT_MODE36_MASK, BIT4},
    {"PM_PAD_EXT_MODE37_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE37), REG_PM_PAD_EXT_MODE37_MASK, BIT5},
    {"PM_PAD_EXT_MODE38_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE38), REG_PM_PAD_EXT_MODE38_MASK, BIT6},
    {"PM_PAD_EXT_MODE39_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE39), REG_PM_PAD_EXT_MODE39_MASK, BIT7},
    {"PM_PAD_EXT_MODE40_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE40), REG_PM_PAD_EXT_MODE40_MASK, BIT8},
    {"PM_PAD_EXT_MODE41_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE41), REG_PM_PAD_EXT_MODE41_MASK, BIT9},
    {"PM_PAD_EXT_MODE42_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE42), REG_PM_PAD_EXT_MODE42_MASK, BIT10},
    {"PM_PAD_EXT_MODE43_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE43), REG_PM_PAD_EXT_MODE43_MASK, BIT11},
    {"PM_PAD_EXT_MODE44_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_PM_PAD_EXT_MODE44), REG_PM_PAD_EXT_MODE44_MASK, BIT12},
    {"SPI_GPIO_0", _RIUA_16BIT(PMSLEEP_BANK, REG_SPI_GPIO), REG_SPI_GPIO_MASK, 0},
    {"SPIWPN_GPIO_0", _RIUA_16BIT(PMSLEEP_BANK, REG_SPIWPN_GPIO), REG_SPIWPN_GPIO_MASK, 0},
    {"SPICSZ1_GPIO_0", _RIUA_16BIT(PMSLEEP_BANK, REG_SPICSZ1_GPIO), REG_SPICSZ1_GPIO_MASK, 0},
    {"SPICSZ2_MODE_1", _RIUA_16BIT(PM_PADTOP_BANK, REG_SPICSZ2_MODE), REG_SPICSZ2_MODE_MASK, BIT0},
    {"SPIHOLDN_MODE_0", _RIUA_16BIT(PM_PADTOP_BANK, REG_SPIHOLDN_MODE), REG_SPIHOLDN_MODE_MASK, 0},
    {"UART_GPIO_EN_0", _RIUA_16BIT(PM_PADTOP_BANK, REG_UART_GPIO_EN), REG_UART_GPIO_EN_MASK, 0},
    {"TEST_IN_MODE_1", _RIUA_16BIT(CHIPTOP_BANK, REG_TEST_IN_MODE), REG_TEST_IN_MODE_MASK, BIT0},
    {"TEST_IN_MODE_2", _RIUA_16BIT(CHIPTOP_BANK, REG_TEST_IN_MODE), REG_TEST_IN_MODE_MASK, BIT1},
    {"TEST_IN_MODE_3", _RIUA_16BIT(CHIPTOP_BANK, REG_TEST_IN_MODE), REG_TEST_IN_MODE_MASK, BIT1 | BIT0},
    {"TEST_OUT_MODE_1", _RIUA_16BIT(CHIPTOP_BANK, REG_TEST_OUT_MODE), REG_TEST_OUT_MODE_MASK, BIT4},
    {"TEST_OUT_MODE_2", _RIUA_16BIT(CHIPTOP_BANK, REG_TEST_OUT_MODE), REG_TEST_OUT_MODE_MASK, BIT5},
    {"TEST_OUT_MODE_3", _RIUA_16BIT(CHIPTOP_BANK, REG_TEST_OUT_MODE), REG_TEST_OUT_MODE_MASK, BIT5 | BIT4},
    {"I2C0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2C0_MODE), REG_I2C0_MODE_MASK, BIT0},
    {"I2C0_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2C0_MODE), REG_I2C0_MODE_MASK, BIT1},
    {"I2C0_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2C0_MODE), REG_I2C0_MODE_MASK, BIT1 | BIT0},
    {"I2C0_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2C0_MODE), REG_I2C0_MODE_MASK, BIT2},
    {"I2C0_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_I2C0_MODE), REG_I2C0_MODE_MASK, BIT2 | BIT0},
    {"I2C0_MODE_6", _RIUA_16BIT(PADTOP_BANK, REG_I2C0_MODE), REG_I2C0_MODE_MASK, BIT2 | BIT1},
    {"I2C1_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2C1_MODE), REG_I2C1_MODE_MASK, BIT0},
    {"I2C1_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2C1_MODE), REG_I2C1_MODE_MASK, BIT1},
    {"I2C1_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2C1_MODE), REG_I2C1_MODE_MASK, BIT1 | BIT0},
    {"I2C1_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2C1_MODE), REG_I2C1_MODE_MASK, BIT2},
    {"I2C1_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_I2C1_MODE), REG_I2C1_MODE_MASK, BIT2 | BIT0},
    {"I2C1_MODE_6", _RIUA_16BIT(PADTOP_BANK, REG_I2C1_MODE), REG_I2C1_MODE_MASK, BIT2 | BIT1},
    {"I2C2_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2C2_MODE), REG_I2C2_MODE_MASK, BIT8},
    {"I2C2_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2C2_MODE), REG_I2C2_MODE_MASK, BIT9},
    {"I2C2_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2C2_MODE), REG_I2C2_MODE_MASK, BIT9 | BIT8},
    {"I2C2_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2C2_MODE), REG_I2C2_MODE_MASK, BIT10},
    {"I2C2_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_I2C2_MODE), REG_I2C2_MODE_MASK, BIT10 | BIT8},
    {"I2C2_MODE_6", _RIUA_16BIT(PADTOP_BANK, REG_I2C2_MODE), REG_I2C2_MODE_MASK, BIT10 | BIT9},
    {"I2C3_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2C3_MODE), REG_I2C3_MODE_MASK, BIT0},
    {"I2C3_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2C3_MODE), REG_I2C3_MODE_MASK, BIT1},
    {"I2C3_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2C3_MODE), REG_I2C3_MODE_MASK, BIT1 | BIT0},
    {"I2C3_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2C3_MODE), REG_I2C3_MODE_MASK, BIT2},
#ifndef CONFIG_OPTEE
    {"I2C4_MODE_1", _RIUA_16BIT(TZMISC_BANK, REG_I2C4_MODE), REG_I2C4_MODE_MASK, BIT4},
    {"I2C4_MODE_2", _RIUA_16BIT(TZMISC_BANK, REG_I2C4_MODE), REG_I2C4_MODE_MASK, BIT5},
    {"I2C4_MODE_3", _RIUA_16BIT(TZMISC_BANK, REG_I2C4_MODE), REG_I2C4_MODE_MASK, BIT5 | BIT4},
    {"I2C4_MODE_4", _RIUA_16BIT(TZMISC_BANK, REG_I2C4_MODE), REG_I2C4_MODE_MASK, BIT6},
#endif
    {"I2C5_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2C5_MODE), REG_I2C5_MODE_MASK, BIT8},
    {"I2C5_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2C5_MODE), REG_I2C5_MODE_MASK, BIT9},
    {"I2C5_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2C5_MODE), REG_I2C5_MODE_MASK, BIT9 | BIT8},
    {"SPI0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI0_MODE), REG_SPI0_MODE_MASK, BIT0},
    {"SPI0_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SPI0_MODE), REG_SPI0_MODE_MASK, BIT1},
    {"SPI0_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SPI0_MODE), REG_SPI0_MODE_MASK, BIT1 | BIT0},
    {"SPI0_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SPI0_MODE), REG_SPI0_MODE_MASK, BIT2},
    {"SPI0_CZ1_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI0_CZ1_MODE), REG_SPI0_CZ1_MODE_MASK, BIT4},
    {"SPI0_CZ1_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SPI0_CZ1_MODE), REG_SPI0_CZ1_MODE_MASK, BIT5},
    {"SPI0_CZ1_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SPI0_CZ1_MODE), REG_SPI0_CZ1_MODE_MASK, BIT5 | BIT4},
    {"SPI1_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI1_MODE), REG_SPI1_MODE_MASK, BIT3},
    {"SPI1_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SPI1_MODE), REG_SPI1_MODE_MASK, BIT4},
    {"SPI1_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SPI1_MODE), REG_SPI1_MODE_MASK, BIT4 | BIT3},
    {"SPI1_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SPI1_MODE), REG_SPI1_MODE_MASK, BIT5},
    {"SPI2_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI2_MODE), REG_SPI2_MODE_MASK, BIT8},
    {"SPI2_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SPI2_MODE), REG_SPI2_MODE_MASK, BIT9},
    {"SPI2_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SPI2_MODE), REG_SPI2_MODE_MASK, BIT9 | BIT8},
    {"SPI2_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SPI2_MODE), REG_SPI2_MODE_MASK, BIT10},
    {"SPI2_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_SPI2_MODE), REG_SPI2_MODE_MASK, BIT10 | BIT8},
    {"SPI3_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI3_MODE), REG_SPI3_MODE_MASK, BIT0},
    {"SPI3_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SPI3_MODE), REG_SPI3_MODE_MASK, BIT1},
    {"SPI3_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SPI3_MODE), REG_SPI3_MODE_MASK, BIT1 | BIT0},
    {"SPI3_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SPI3_MODE), REG_SPI3_MODE_MASK, BIT2},
    {"SPI3_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_SPI3_MODE), REG_SPI3_MODE_MASK, BIT2 | BIT0},
    {"FUART_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_FUART_MODE), REG_FUART_MODE_MASK, BIT8},
    {"FUART_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_FUART_MODE), REG_FUART_MODE_MASK, BIT9},
    {"FUART_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_FUART_MODE), REG_FUART_MODE_MASK, BIT9 | BIT8},
    {"FUART_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_FUART_MODE), REG_FUART_MODE_MASK, BIT10},
    {"FUART_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_FUART_MODE), REG_FUART_MODE_MASK, BIT10 | BIT8},
    {"FUART_2W_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_FUART_2W_MODE), REG_FUART_2W_MODE_MASK, BIT12},
    {"FUART_2W_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_FUART_2W_MODE), REG_FUART_2W_MODE_MASK, BIT13},
    {"FUART_2W_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_FUART_2W_MODE), REG_FUART_2W_MODE_MASK, BIT13 | BIT12},
    {"FUART_2W_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_FUART_2W_MODE), REG_FUART_2W_MODE_MASK, BIT14},
    {"FUART_2W_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_FUART_2W_MODE), REG_FUART_2W_MODE_MASK, BIT14 | BIT12},
    {"UART0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_UART0_MODE), REG_UART0_MODE_MASK, BIT0},
    {"UART0_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_UART0_MODE), REG_UART0_MODE_MASK, BIT1},
    {"UART1_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_UART1_MODE), REG_UART1_MODE_MASK, BIT4},
    {"UART1_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_UART1_MODE), REG_UART1_MODE_MASK, BIT5},
    {"UART1_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_UART1_MODE), REG_UART1_MODE_MASK, BIT5 | BIT4},
    {"UART1_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_UART1_MODE), REG_UART1_MODE_MASK, BIT6},
    {"UART2_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_UART2_MODE), REG_UART2_MODE_MASK, BIT8},
    {"UART2_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_UART2_MODE), REG_UART2_MODE_MASK, BIT9},
    {"UART2_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_UART2_MODE), REG_UART2_MODE_MASK, BIT9 | BIT8},
    {"UART2_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_UART2_MODE), REG_UART2_MODE_MASK, BIT10},
    {"UART3_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_UART3_MODE), REG_UART3_MODE_MASK, BIT12},
    {"UART3_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_UART3_MODE), REG_UART3_MODE_MASK, BIT13},
    {"UART3_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_UART3_MODE), REG_UART3_MODE_MASK, BIT13 | BIT12},
    {"UART3_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_UART3_MODE), REG_UART3_MODE_MASK, BIT14},
    {"UART4_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_UART4_MODE), REG_UART4_MODE_MASK, BIT0},
    {"UART4_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_UART4_MODE), REG_UART4_MODE_MASK, BIT1},
    {"UART4_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_UART4_MODE), REG_UART4_MODE_MASK, BIT1 | BIT0},
    {"UART4_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_UART4_MODE), REG_UART4_MODE_MASK, BIT2},
    {"UART5_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_UART5_MODE), REG_UART5_MODE_MASK, BIT4},
    {"UART5_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_UART5_MODE), REG_UART5_MODE_MASK, BIT5},
    {"UART5_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_UART5_MODE), REG_UART5_MODE_MASK, BIT5 | BIT4},
    {"UART5_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_UART5_MODE), REG_UART5_MODE_MASK, BIT6},
    {"UART6_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_UART6_MODE), REG_UART6_MODE_MASK, BIT8},
    {"UART6_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_UART6_MODE), REG_UART6_MODE_MASK, BIT9},
    {"UART6_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_UART6_MODE), REG_UART6_MODE_MASK, BIT9 | BIT8},
    {"UART6_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_UART6_MODE), REG_UART6_MODE_MASK, BIT10},
    {"SD0_BOOT_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SD0_BOOT_MODE), REG_SD0_BOOT_MODE_MASK, BIT0},
    {"SD0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SD0_MODE), REG_SD0_MODE_MASK, BIT8},
    {"SD0_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SD0_MODE), REG_SD0_MODE_MASK, BIT9},
    {"SD0_CDZ_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SD0_CDZ_MODE), REG_SD0_CDZ_MODE_MASK, BIT10},
    {"SD0_CDZ_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SD0_CDZ_MODE), REG_SD0_CDZ_MODE_MASK, BIT11},
    {"SD0_RSTN_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SD0_RSTN_MODE), REG_SD0_RSTN_MODE_MASK, BIT0},
    {"SD0_RSTN_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SD0_RSTN_MODE), REG_SD0_RSTN_MODE_MASK, BIT1},
    {"EMMC8B_BOOT_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_EMMC8B_BOOT_MODE), REG_EMMC8B_BOOT_MODE_MASK, BIT0},
    {"EMMC4B_BOOT_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_EMMC4B_BOOT_MODE), REG_EMMC4B_BOOT_MODE_MASK, BIT8},
    {"EMMC_8B_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_EMMC_8B_MODE), REG_EMMC_8B_MODE_MASK, BIT2},
    {"EMMC_4B_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_EMMC_4B_MODE), REG_EMMC_4B_MODE_MASK, BIT0},
    {"EMMC_RST_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_EMMC_RST_MODE), REG_EMMC_RST_MODE_MASK, BIT4},
    {"EMMC_AS_SD_CDZ_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_EMMC_AS_SD_CDZ_MODE), REG_EMMC_AS_SD_CDZ_MODE_MASK, BIT8},
    {"SDIO_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_MODE), REG_SDIO_MODE_MASK, BIT12},
    {"SDIO_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_MODE), REG_SDIO_MODE_MASK, BIT13},
    {"SDIO_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_MODE), REG_SDIO_MODE_MASK, BIT13 | BIT12},
    {"SDIO_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_MODE), REG_SDIO_MODE_MASK, BIT14},
    {"SDIO_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_MODE), REG_SDIO_MODE_MASK, BIT14 | BIT12},
    {"SDIO_CDZ_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_CDZ_MODE), REG_SDIO_CDZ_MODE_MASK, BIT0},
    {"SDIO_RST_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_RST_MODE), REG_SDIO_RST_MODE_MASK, BIT4},
    {"SDIO_CDZ_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_CDZ_MODE), REG_SDIO_CDZ_MODE_MASK, BIT1},
    {"SDIO_RST_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_RST_MODE), REG_SDIO_RST_MODE_MASK, BIT5},
    {"SDIO_CDZ_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_CDZ_MODE), REG_SDIO_CDZ_MODE_MASK, BIT1 | BIT0},
    {"SDIO_RST_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_RST_MODE), REG_SDIO_RST_MODE_MASK, BIT5 | BIT4},
    {"SDIO_CDZ_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_CDZ_MODE), REG_SDIO_CDZ_MODE_MASK, BIT2},
    {"SDIO_RST_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_RST_MODE), REG_SDIO_RST_MODE_MASK, BIT6},
    {"SDIO_CDZ_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_CDZ_MODE), REG_SDIO_CDZ_MODE_MASK, BIT2 | BIT0},
    {"SDIO_RST_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_SDIO_RST_MODE), REG_SDIO_RST_MODE_MASK, BIT6 | BIT4},
    {"PWM0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM0_MODE), REG_PWM0_MODE_MASK, BIT0},
    {"PWM0_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM0_MODE), REG_PWM0_MODE_MASK, BIT1},
    {"PWM0_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM0_MODE), REG_PWM0_MODE_MASK, BIT1 | BIT0},
    {"PWM0_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM0_MODE), REG_PWM0_MODE_MASK, BIT2},
    {"PWM0_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_PWM0_MODE), REG_PWM0_MODE_MASK, BIT2 | BIT0},
    {"PWM1_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM1_MODE), REG_PWM1_MODE_MASK, BIT4},
    {"PWM1_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM1_MODE), REG_PWM1_MODE_MASK, BIT5},
    {"PWM1_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM1_MODE), REG_PWM1_MODE_MASK, BIT5 | BIT4},
    {"PWM1_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM1_MODE), REG_PWM1_MODE_MASK, BIT6},
    {"PWM1_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_PWM1_MODE), REG_PWM1_MODE_MASK, BIT6 | BIT4},
    {"PWM2_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM2_MODE), REG_PWM2_MODE_MASK, BIT8},
    {"PWM2_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM2_MODE), REG_PWM2_MODE_MASK, BIT9},
    {"PWM2_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM2_MODE), REG_PWM2_MODE_MASK, BIT9 | BIT8},
    {"PWM2_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM2_MODE), REG_PWM2_MODE_MASK, BIT10},
    {"PWM2_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_PWM2_MODE), REG_PWM2_MODE_MASK, BIT10 | BIT8},
    {"PWM3_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM3_MODE), REG_PWM3_MODE_MASK, BIT12},
    {"PWM3_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM3_MODE), REG_PWM3_MODE_MASK, BIT13},
    {"PWM3_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM3_MODE), REG_PWM3_MODE_MASK, BIT13 | BIT12},
    {"PWM3_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM3_MODE), REG_PWM3_MODE_MASK, BIT14},
    {"PWM3_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_PWM3_MODE), REG_PWM3_MODE_MASK, BIT14 | BIT12},
    {"PWM4_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM4_MODE), REG_PWM4_MODE_MASK, BIT0},
    {"PWM4_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM4_MODE), REG_PWM4_MODE_MASK, BIT1},
    {"PWM4_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM4_MODE), REG_PWM4_MODE_MASK, BIT1 | BIT0},
    {"PWM4_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM4_MODE), REG_PWM4_MODE_MASK, BIT2},
    {"PWM5_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM5_MODE), REG_PWM5_MODE_MASK, BIT4},
    {"PWM5_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM5_MODE), REG_PWM5_MODE_MASK, BIT5},
    {"PWM5_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM5_MODE), REG_PWM5_MODE_MASK, BIT5 | BIT4},
    {"PWM5_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM5_MODE), REG_PWM5_MODE_MASK, BIT6},
    {"PWM6_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM6_MODE), REG_PWM6_MODE_MASK, BIT8},
    {"PWM6_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM6_MODE), REG_PWM6_MODE_MASK, BIT9},
    {"PWM6_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM6_MODE), REG_PWM6_MODE_MASK, BIT9 | BIT8},
    {"PWM6_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM6_MODE), REG_PWM6_MODE_MASK, BIT10},
    {"PWM7_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM7_MODE), REG_PWM7_MODE_MASK, BIT12},
    {"PWM7_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM7_MODE), REG_PWM7_MODE_MASK, BIT13},
    {"PWM7_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM7_MODE), REG_PWM7_MODE_MASK, BIT13 | BIT12},
    {"PWM7_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM7_MODE), REG_PWM7_MODE_MASK, BIT14},
    {"PWM8_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM8_MODE), REG_PWM8_MODE_MASK, BIT0},
    {"PWM8_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM8_MODE), REG_PWM8_MODE_MASK, BIT1},
    {"PWM8_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM8_MODE), REG_PWM8_MODE_MASK, BIT1 | BIT0},
    {"PWM8_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM8_MODE), REG_PWM8_MODE_MASK, BIT2},
    {"PWM9_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM9_MODE), REG_PWM9_MODE_MASK, BIT4},
    {"PWM9_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM9_MODE), REG_PWM9_MODE_MASK, BIT5},
    {"PWM9_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM9_MODE), REG_PWM9_MODE_MASK, BIT5 | BIT4},
    {"PWM9_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM9_MODE), REG_PWM9_MODE_MASK, BIT6},
    {"PWM10_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM10_MODE), REG_PWM10_MODE_MASK, BIT8},
    {"PWM10_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM10_MODE), REG_PWM10_MODE_MASK, BIT9},
    {"PWM10_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM10_MODE), REG_PWM10_MODE_MASK, BIT9 | BIT8},
    {"PWM10_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM10_MODE), REG_PWM10_MODE_MASK, BIT10},
    {"PWM11_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_PWM11_MODE), REG_PWM11_MODE_MASK, BIT12},
    {"PWM11_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_PWM11_MODE), REG_PWM11_MODE_MASK, BIT13},
    {"PWM11_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_PWM11_MODE), REG_PWM11_MODE_MASK, BIT13 | BIT12},
    {"PWM11_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_PWM11_MODE), REG_PWM11_MODE_MASK, BIT14},
    {"LED0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_LED0_MODE), REG_LED0_MODE_MASK, BIT0},
    {"LED0_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_LED0_MODE), REG_LED0_MODE_MASK, BIT1},
    {"LED0_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_LED0_MODE), REG_LED0_MODE_MASK, BIT1 | BIT0},
    {"LED1_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_LED1_MODE), REG_LED1_MODE_MASK, BIT4},
    {"LED1_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_LED1_MODE), REG_LED1_MODE_MASK, BIT5},
    {"LED1_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_LED1_MODE), REG_LED1_MODE_MASK, BIT5 | BIT4},
    {"VD_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_VD_MODE), REG_VD_MODE_MASK, BIT8},
    {"VD_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_VD_MODE), REG_VD_MODE_MASK, BIT9},
    {"VD_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_VD_MODE), REG_VD_MODE_MASK, BIT9 | BIT8},
    {"VD_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_VD_MODE), REG_VD_MODE_MASK, BIT10},
    {"I2S0_MCK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_MCK_MODE), REG_I2S0_MCK_MODE_MASK, BIT0},
    {"I2S0_MCK_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_MCK_MODE), REG_I2S0_MCK_MODE_MASK, BIT1},
    {"I2S0_MCK_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_MCK_MODE), REG_I2S0_MCK_MODE_MASK, BIT1 | BIT0},
    {"I2S0_MCK_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_MCK_MODE), REG_I2S0_MCK_MODE_MASK, BIT2},
    {"I2S0_RX_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RX_MODE), REG_I2S0_RX_MODE_MASK, BIT8},
    {"I2S0_RX_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RX_MODE), REG_I2S0_RX_MODE_MASK, BIT9},
    {"I2S0_RX_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RX_MODE), REG_I2S0_RX_MODE_MASK, BIT9 | BIT8},
    {"I2S0_RX_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RX_MODE), REG_I2S0_RX_MODE_MASK, BIT10},
    {"I2S0_TX_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_TX_MODE), REG_I2S0_TX_MODE_MASK, BIT12},
    {"I2S0_TX_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_TX_MODE), REG_I2S0_TX_MODE_MASK, BIT13},
    {"I2S0_TX_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_TX_MODE), REG_I2S0_TX_MODE_MASK, BIT13 | BIT12},
    {"I2S0_TX_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_TX_MODE), REG_I2S0_TX_MODE_MASK, BIT14},
    {"I2S0_RXTX_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RXTX_MODE), REG_I2S0_RXTX_MODE_MASK, BIT0},
    {"I2S0_RXTX_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RXTX_MODE), REG_I2S0_RXTX_MODE_MASK, BIT1},
    {"I2S0_RXTX_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RXTX_MODE), REG_I2S0_RXTX_MODE_MASK, BIT1 | BIT0},
    {"I2S0_RXTX_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RXTX_MODE), REG_I2S0_RXTX_MODE_MASK, BIT2},
    {"I2S0_RXTX_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_I2S0_RXTX_MODE), REG_I2S0_RXTX_MODE_MASK, BIT2 | BIT0},
    {"DMIC_2CH_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_2CH_MODE), REG_DMIC_2CH_MODE_MASK, BIT0},
    {"DMIC_2CH_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_2CH_MODE), REG_DMIC_2CH_MODE_MASK, BIT1},
    {"DMIC_2CH_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_2CH_MODE), REG_DMIC_2CH_MODE_MASK, BIT1 | BIT0},
    {"DMIC_2CH_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_2CH_MODE), REG_DMIC_2CH_MODE_MASK, BIT2},
    {"DMIC_2CH_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_2CH_MODE), REG_DMIC_2CH_MODE_MASK, BIT2 | BIT0},
    {"DMIC_4CH_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_4CH_MODE), REG_DMIC_4CH_MODE_MASK, BIT0},
    {"DMIC_4CH_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_4CH_MODE), REG_DMIC_4CH_MODE_MASK, BIT1},
    {"DMIC_4CH_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_4CH_MODE), REG_DMIC_4CH_MODE_MASK, BIT1 | BIT0},
    {"DMIC_4CH_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_4CH_MODE), REG_DMIC_4CH_MODE_MASK, BIT2},
    {"DMIC_4CH_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_4CH_MODE), REG_DMIC_4CH_MODE_MASK, BIT2 | BIT0},
    {"DMIC_6CH_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_6CH_MODE), REG_DMIC_6CH_MODE_MASK, BIT4},
    {"DMIC_6CH_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_6CH_MODE), REG_DMIC_6CH_MODE_MASK, BIT5},
    {"DMIC_6CH_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_6CH_MODE), REG_DMIC_6CH_MODE_MASK, BIT5 | BIT4},
    {"DMIC_6CH_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_6CH_MODE), REG_DMIC_6CH_MODE_MASK, BIT6},
    {"DMIC_6CH_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_6CH_MODE), REG_DMIC_6CH_MODE_MASK, BIT6 | BIT4},
    {"DMIC_6CH_MODE_6", _RIUA_16BIT(PADTOP_BANK, REG_DMIC_6CH_MODE), REG_DMIC_6CH_MODE_MASK, BIT6 | BIT5},
    {"SR0_MIPI_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MIPI_MODE), REG_SR0_MIPI_MODE_MASK, BIT0},
    {"SR0_MIPI_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MIPI_MODE), REG_SR0_MIPI_MODE_MASK, BIT1},
    {"SR0_MIPI_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MIPI_MODE), REG_SR0_MIPI_MODE_MASK, BIT1 | BIT0},
    {"SR0_MIPI_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MIPI_MODE), REG_SR0_MIPI_MODE_MASK, BIT2},
    {"SR0_MIPI_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MIPI_MODE), REG_SR0_MIPI_MODE_MASK, BIT2 | BIT0},
    {"SR1_MIPI_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR1_MIPI_MODE), REG_SR1_MIPI_MODE_MASK, BIT4},
    {"SR1_MIPI_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR1_MIPI_MODE), REG_SR1_MIPI_MODE_MASK, BIT5},
    {"SR1_MIPI_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR1_MIPI_MODE), REG_SR1_MIPI_MODE_MASK, BIT5 | BIT4},
    {"SR2_MIPI_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR2_MIPI_MODE), REG_SR2_MIPI_MODE_MASK, BIT8},
    {"SR0_SUB_LVDS_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_SUB_LVDS_MODE), REG_SR0_SUB_LVDS_MODE_MASK, BIT14},
    {"SR0_SUB_LVDS_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_SUB_LVDS_MODE), REG_SR0_SUB_LVDS_MODE_MASK, BIT15},
    {"SR0_SUB_LVDS_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_SUB_LVDS_MODE), REG_SR0_SUB_LVDS_MODE_MASK, BIT15 | BIT14},
    {"SR0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT0},
    {"SR0_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT1},
    {"SR0_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT1 | BIT0},
    {"SR0_MODE_9", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT3 | BIT0},
    {"SR0_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT2},
    {"SR0_MODE_5", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT2 | BIT0},
    {"SR0_MODE_6", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT2 | BIT1},
    {"SR0_MODE_7", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT2 | BIT1 | BIT0},
    {"SR0_MODE_8", _RIUA_16BIT(PADTOP_BANK, REG_SR0_MODE), REG_SR0_MODE_MASK, BIT3},
    {"SR0_INFRARED_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_INFRARED_MODE), REG_SR0_INFRARED_MODE_MASK, BIT8},
    {"SR0_INFRARED_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_INFRARED_MODE), REG_SR0_INFRARED_MODE_MASK, BIT9},
    {"SR0_INFRARED_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_INFRARED_MODE), REG_SR0_INFRARED_MODE_MASK, BIT9 | BIT8},
    {"ISR_MCLK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_ISR_MCLK_MODE), REG_ISR_MCLK_MODE_MASK, BIT12},
    {"ISR_MCLK_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_ISR_MCLK_MODE), REG_ISR_MCLK_MODE_MASK, BIT13},
    {"SR00_MCLK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR00_MCLK_MODE), REG_SR00_MCLK_MODE_MASK, BIT0},
    {"SR00_MCLK_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR00_MCLK_MODE), REG_SR00_MCLK_MODE_MASK, BIT1},
    {"SR00_MCLK_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR00_MCLK_MODE), REG_SR00_MCLK_MODE_MASK, BIT1 | BIT0},
    {"SR01_MCLK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR01_MCLK_MODE), REG_SR01_MCLK_MODE_MASK, BIT2},
    {"SR01_MCLK_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR01_MCLK_MODE), REG_SR01_MCLK_MODE_MASK, BIT3},
    {"SR1_MCLK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR1_MCLK_MODE), REG_SR1_MCLK_MODE_MASK, BIT4},
    {"SR1_MCLK_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR1_MCLK_MODE), REG_SR1_MCLK_MODE_MASK, BIT5},
    {"SR1_MCLK_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR1_MCLK_MODE), REG_SR1_MCLK_MODE_MASK, BIT5 | BIT4},
    {"SR2_MCLK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR2_MCLK_MODE), REG_SR2_MCLK_MODE_MASK, BIT6},
    {"SR00_RST_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR00_RST_MODE), REG_SR00_RST_MODE_MASK, BIT0},
    {"SR00_RST_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR00_RST_MODE), REG_SR00_RST_MODE_MASK, BIT1},
    {"SR00_RST_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR00_RST_MODE), REG_SR00_RST_MODE_MASK, BIT1 | BIT0},
    {"SR01_RST_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR01_RST_MODE), REG_SR01_RST_MODE_MASK, BIT2},
    {"SR01_RST_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR01_RST_MODE), REG_SR01_RST_MODE_MASK, BIT3},
    {"SR1_RST_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR1_RST_MODE), REG_SR1_RST_MODE_MASK, BIT4},
    {"SR1_RST_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR1_RST_MODE), REG_SR1_RST_MODE_MASK, BIT5},
    {"SR1_RST_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR1_RST_MODE), REG_SR1_RST_MODE_MASK, BIT5 | BIT4},
    {"SR2_RST_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR2_RST_MODE), REG_SR2_RST_MODE_MASK, BIT6},
    {"SR0_PDN_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PDN_MODE), REG_SR0_PDN_MODE_MASK, BIT8},
    {"SR0_PDN_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PDN_MODE), REG_SR0_PDN_MODE_MASK, BIT9},
    {"SR0_PDN_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PDN_MODE), REG_SR0_PDN_MODE_MASK, BIT9 | BIT8},
    {"SR0_PDN_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PDN_MODE), REG_SR0_PDN_MODE_MASK, BIT10},
    {"SR1_PDN_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR1_PDN_MODE), REG_SR1_PDN_MODE_MASK, BIT2},
    {"SR1_PDN_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR1_PDN_MODE), REG_SR1_PDN_MODE_MASK, BIT3},
    {"SR2_PDN_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR2_PDN_MODE), REG_SR2_PDN_MODE_MASK, BIT4},
    {"SR0_HSYNC_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_HSYNC_MODE), REG_SR0_HSYNC_MODE_MASK, BIT8},
    {"SR0_HSYNC_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_HSYNC_MODE), REG_SR0_HSYNC_MODE_MASK, BIT9},
    {"SR0_HSYNC_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_HSYNC_MODE), REG_SR0_HSYNC_MODE_MASK, BIT9 | BIT8},
    {"SR0_VSYNC_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_VSYNC_MODE), REG_SR0_VSYNC_MODE_MASK, BIT12},
    {"SR0_VSYNC_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_VSYNC_MODE), REG_SR0_VSYNC_MODE_MASK, BIT13},
    {"SR0_VSYNC_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_VSYNC_MODE), REG_SR0_VSYNC_MODE_MASK, BIT13 | BIT12},
    {"SR0_PCLK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PCLK_MODE), REG_SR0_PCLK_MODE_MASK, BIT0},
    {"SR0_PCLK_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PCLK_MODE), REG_SR0_PCLK_MODE_MASK, BIT1},
    {"SR0_PCLK_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PCLK_MODE), REG_SR0_PCLK_MODE_MASK, BIT1 | BIT0},
    {"SR0_PCLK_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_SR0_PCLK_MODE), REG_SR0_PCLK_MODE_MASK, BIT2},
    {"SR0_BT656_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_BT656_MODE), REG_SR0_BT656_MODE_MASK, BIT0},
    {"SR1_BT656_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR1_BT656_MODE), REG_SR1_BT656_MODE_MASK, BIT2},
    {"SR0_BT1120_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_BT1120_MODE), REG_SR0_BT1120_MODE_MASK, BIT0},
    {"SR0_BT1120_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_BT1120_MODE), REG_SR0_BT1120_MODE_MASK, BIT1},
    {"SR_SLAVE_XLK_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR_SLAVE_XLK_MODE), REG_SR_SLAVE_XLK_MODE_MASK, BIT0},
    {"SR_SLAVE_XLK_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR_SLAVE_XLK_MODE), REG_SR_SLAVE_XLK_MODE_MASK, BIT1},
    {"SR0_SLAVE_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_SR0_SLAVE_MODE), REG_SR0_SLAVE_MODE_MASK, BIT12},
    {"SR0_SLAVE_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_SR0_SLAVE_MODE), REG_SR0_SLAVE_MODE_MASK, BIT13},
    {"MIPITX0_OUT_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_MIPITX0_OUT_MODE), REG_MIPITX0_OUT_MODE_MASK, BIT0},
    {"MIPITX0_OUT_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_MIPITX0_OUT_MODE), REG_MIPITX0_OUT_MODE_MASK, BIT1},
    {"MIPITX0_OUT_MODE_3", _RIUA_16BIT(PADTOP_BANK, REG_MIPITX0_OUT_MODE), REG_MIPITX0_OUT_MODE_MASK, BIT1 | BIT0},
    {"MIPITX0_OUT_MODE_4", _RIUA_16BIT(PADTOP_BANK, REG_MIPITX0_OUT_MODE), REG_MIPITX0_OUT_MODE_MASK, BIT2},
    {"BT656_OUT_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_BT656_OUT_MODE), REG_BT656_OUT_MODE_MASK, BIT4},
    {"BT656_OUT_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_BT656_OUT_MODE), REG_BT656_OUT_MODE_MASK, BIT5},
    {"BT1120_OUT_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_BT1120_OUT_MODE), REG_BT1120_OUT_MODE_MASK, BIT0},
    {"TTL24_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_TTL24_MODE), REG_TTL24_MODE_MASK, BIT8},
    {"TTL18_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_TTL18_MODE), REG_TTL18_MODE_MASK, BIT4},
    {"TTL16_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_TTL16_MODE), REG_TTL16_MODE_MASK, BIT0},
    {"TTL16_MODE_2", _RIUA_16BIT(PADTOP_BANK, REG_TTL16_MODE), REG_TTL16_MODE_MASK, BIT1},
    {"RGMII0_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_RGMII0_MODE), REG_RGMII0_MODE_MASK, BIT0},
    {"GPHY0_REF_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_GPHY0_REF_MODE), REG_GPHY0_REF_MODE_MASK, BIT0},
    {"RMII_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_RMII_MODE), REG_RMII_MODE_MASK, BIT8},
    {"OTP_TEST_1", _RIUA_16BIT(PADTOP_BANK, REG_OTP_TEST), REG_OTP_TEST_MASK, BIT8},
#ifndef CONFIG_OPTEE
    {"QSPI_MODE_0", _RIUA_16BIT(TZMISC_BANK, REG_QSPI_MODE), REG_QSPI_MODE_MASK, 0},
#endif
    {"QSPICSZ2_MODE_1", _RIUA_16BIT(PADTOP_BANK, REG_QSPICSZ2_MODE), REG_QSPICSZ2_MODE_MASK, BIT4},
    {"SPI_EXT_EN_MODE0_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI_EXT_EN_MODE0), REG_SPI_EXT_EN_MODE0_MASK, BIT0},
    {"SPI_EXT_EN_MODE1_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI_EXT_EN_MODE1), REG_SPI_EXT_EN_MODE1_MASK, BIT1},
    {"SPI_EXT_EN_MODE2_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI_EXT_EN_MODE2), REG_SPI_EXT_EN_MODE2_MASK, BIT2},
    {"SPI_EXT_EN_MODE3_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI_EXT_EN_MODE3), REG_SPI_EXT_EN_MODE3_MASK, BIT3},
    {"SPI_EXT_EN_MODE4_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI_EXT_EN_MODE4), REG_SPI_EXT_EN_MODE4_MASK, BIT4},
    {"SPI_EXT_EN_MODE5_1", _RIUA_16BIT(PADTOP_BANK, REG_SPI_EXT_EN_MODE5), REG_SPI_EXT_EN_MODE5_MASK, BIT5},
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : hal_gpio_check_pin
//  Description :
//------------------------------------------------------------------------------
static s32 hal_gpio_check_pin(u32 pad_id)
{
    if (GPIO_NR <= pad_id)
    {
        return FALSE;
    }
    return TRUE;
}

void hal_gpio_dis_padmux(u32 pad_mode_id)
{
    if (_GPIO_R_WORD_MASK(m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu,
                          m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_mask))
    {
        _GPIO_W_WORD_MASK(m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu, 0,
                          m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_mask);
    }
}

void hal_gpio_en_padmux(u32 pad_mode_id)
{
    _GPIO_W_WORD_MASK(m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu,
                      m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_val,
                      m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_mask);
}

static s32 hal_gpio_pad_set_mode_general(u32 pad_id, u32 mode)
{
    u64 reg_addr     = 0;
    u16 reg_val      = 0;
    u8  mode_is_find = 0;
    u16 i, ext_item_id = 0;

    for (i = 0; i < m_hal_gpio_st_padmux_entry[pad_id].size; i++)
    {
        reg_addr = _RIUA_16BIT(m_hal_gpio_st_padmux_entry[pad_id].padmux[i].base,
                               m_hal_gpio_st_padmux_entry[pad_id].padmux[i].offset);
        if (mode == m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode)
        {
            reg_val = _GPIO_R_WORD_MASK(reg_addr, 0xFFFF);
            reg_val &= ~(m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask);
            reg_val |= m_hal_gpio_st_padmux_entry[pad_id].padmux[i].val; // CHECK Multi-Pad Mode
            _GPIO_W_WORD_MASK(reg_addr, reg_val, 0xFFFF);
            mode_is_find             = 1;
            pad_mode_recoder[pad_id] = mode;
#if (ENABLE_CHECK_ALL_PAD_CONFLICT == 0)
            break;
#endif
        }
        else if ((m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode >= PINMUX_FOR_PM_PAD_EXT_MODE0_1)
                 && (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode <= PINMUX_FOR_PM_PAD_EXT_MODE44_1))
        {
            ext_item_id = i;
        }
        else if ((m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode >= PINMUX_FOR_SPI_EXT_EN_MODE0_1)
                 && (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode <= PINMUX_FOR_SPI_EXT_EN_MODE5_1))
        {
            ext_item_id = i;
        }
        else
        {
            if ((pad_id < PAD_RGMII0_MCLK) || (pad_id > PAD_RGMII0_MDC))
            {
                if ((mode == PINMUX_FOR_GPIO_MODE)
                    && (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode > PRIORITY_GREATER_GPIO))
                    continue;
            }
            reg_val = _GPIO_R_WORD_MASK(reg_addr, m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask);
            if (reg_val == m_hal_gpio_st_padmux_entry[pad_id].padmux[i].val)
            {
                hal_pinmux_info(
                    "[Padmux]reset PAD%d(reg 0x%x:%x; mask0x%x) t0 %s (org: %s)\n", pad_id,
                    m_hal_gpio_st_padmux_entry[pad_id].padmux[i].base,
                    m_hal_gpio_st_padmux_entry[pad_id].padmux[i].offset,
                    m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask, m_hal_gpio_st_padmode_info_tbl[mode].pad_name,
                    m_hal_gpio_st_padmode_info_tbl[m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode].pad_name);
                if (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].val != 0)
                {
                    _GPIO_W_WORD_MASK(reg_addr, 0, m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask);
                }
                else
                {
                    _GPIO_W_WORD_MASK(reg_addr, m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask,
                                      m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask);
                }
            }
        }
    }

    if ((mode_is_find) && (ext_item_id))
    {
        // set external data mode
        reg_addr = _RIUA_16BIT(m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].base,
                               m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].offset);
        reg_val  = _GPIO_R_WORD_MASK(reg_addr, 0xFFFF);
        reg_val &= ~(m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].mask);
        reg_val |= m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].val; // CHECK Multi-Pad Mode

        _GPIO_W_WORD_MASK(reg_addr, reg_val, 0xFFFF);
    }

    return (mode_is_find) ? 0 : 1;
}

static s32 hal_gpio_pad_clr_mode_general(u32 pad_id, u32 mode)
{
    u64 reg_addr     = 0;
    u8  mode_is_find = 0;
    u16 i            = 0;

    for (i = 0; i < m_hal_gpio_st_padmux_entry[pad_id].size; i++)
    {
        reg_addr = _RIUA_16BIT(m_hal_gpio_st_padmux_entry[pad_id].padmux[i].base,
                               m_hal_gpio_st_padmux_entry[pad_id].padmux[i].offset);
        if (mode == m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode)
        {
            _GPIO_W_WORD_MASK(reg_addr, 0, m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask);
            if (pad_mode_recoder[pad_id] == mode)
            {
                pad_mode_recoder[pad_id] = PINMUX_FOR_UNKNOWN_MODE;
            }
            mode_is_find = 1;
        }
    }

    return (mode_is_find) ? 0 : 1;
}

//------------------------------------------------------------------------------
//  Function    : hal_gpio_pad_set_val
//  Description :
//------------------------------------------------------------------------------
s32 hal_gpio_pad_set_val(u32 pad_id, u32 mode)
{
    if (FALSE == hal_gpio_check_pin(pad_id))
    {
        return 1;
    }
    else
    {
        return hal_gpio_pad_set_mode_general(pad_id, mode);
    }
}

//------------------------------------------------------------------------------
//  Function    : hal_gpio_pad_clr_val
//  Description :
//------------------------------------------------------------------------------
s32 hal_gpio_pad_clr_val(u32 pad_id, u32 mode)
{
    if (FALSE == hal_gpio_check_pin(pad_id))
    {
        return 1;
    }
    else
    {
        return hal_gpio_pad_clr_mode_general(pad_id, mode);
    }
}

//------------------------------------------------------------------------------
//  Function    : hal_gpio_pad_get_val
//  Description :
//------------------------------------------------------------------------------
s32 hal_gpio_pad_get_val(u32 pad_id, u32* mode)
{
    if (FALSE == hal_gpio_check_pin(pad_id))
    {
        return 1;
    }
    *mode = pad_mode_recoder[pad_id];
    return 0;
}

//------------------------------------------------------------------------------
//  Function    :set GPIO voltage value
//  Description :only for i7
//------------------------------------------------------------------------------
void hal_gpio_set_vol(u32 group, u32 mode) {}
//------------------------------------------------------------------------------
//  Function    : HalPadSet
//  Description :
//------------------------------------------------------------------------------
s32 hal_gpio_pad_set_mode(u32 mode)
{
    u32 pad_id;
    u16 k = 0;
    u16 i = 0;

    for (k = 0; k < sizeof(m_hal_gpio_st_padmux_entry) / sizeof(struct hal_gpio_st_padmux_entry); k++)
    {
        for (i = 0; i < m_hal_gpio_st_padmux_entry[k].size; i++)
        {
            if (mode == m_hal_gpio_st_padmux_entry[k].padmux[i].mode)
            {
                pad_id = m_hal_gpio_st_padmux_entry[k].padmux[i].pad_id;
                if (pad_id >= GPIO_NR)
                {
                    return 1;
                }
                else
                {
                    if (hal_gpio_pad_set_mode_general(pad_id, mode))
                    {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

s32 hal_gpio_pad_check_dis_padmux(u32 pad_mode_id)
{
    u16 reg_val = 0;

    reg_val = _GPIO_R_WORD_MASK(m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu,
                                m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_mask);

    if (reg_val != 0)
    {
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].base =
            (u16)((m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu & 0xFFFF00) >> 9);
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].offset =
            (u16)((m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu & 0x1FF) >> 2);
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].mask =
            m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_mask;
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].val    = 0;
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].regval = reg_val;
        m_hal_gpio_st_pad_checkVal.infocount++;
        return 1;
    }
    return 0;
}

s32 hal_gpio_pad_check_en_padmux(u32 pad_mode_id)
{
    u16 reg_val = 0;

    reg_val = _GPIO_R_WORD_MASK(m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu,
                                m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_mask);

    if (reg_val != m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_val)
    {
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].base =
            (u16)((m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu & 0xFFFF00) >> 9);
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].offset =
            (u16)((m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_riu & 0x1FF) >> 2);
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].mask =
            m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_mask;
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].val =
            m_hal_gpio_st_padmode_info_tbl[pad_mode_id].mode_val;
        m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].regval = reg_val;
        m_hal_gpio_st_pad_checkVal.infocount++;
        return 1;
    }
    return 0;
}

static s32 hal_gpio_pad_check_mode_general(u32 pad_id, u32 mode)
{
    u64 reg_addr     = 0;
    u16 reg_val      = 0;
    u8  mode_is_find = 0;
    u8  mode_is_err  = 0;
    u16 i, ext_item_id = 0;

    for (i = 0; i < m_hal_gpio_st_padmux_entry[pad_id].size; i++)
    {
        if (pad_id == m_hal_gpio_st_padmux_entry[pad_id].padmux[i].pad_id)
        {
            reg_addr = _RIUA_16BIT(m_hal_gpio_st_padmux_entry[pad_id].padmux[i].base,
                                   m_hal_gpio_st_padmux_entry[pad_id].padmux[i].offset);

            if (mode == m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode)
            {
                reg_val = _GPIO_R_WORD_MASK(reg_addr, 0xFFFF);
                reg_val &= (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask);
                if (reg_val != m_hal_gpio_st_padmux_entry[pad_id].padmux[i].val) // CHECK Multi-Pad Mode
                {
                    mode_is_err++;

                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].base =
                        (u16)(m_hal_gpio_st_padmux_entry[pad_id].padmux[i].base >> 8);
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].offset =
                        m_hal_gpio_st_padmux_entry[pad_id].padmux[i].offset;
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].mask =
                        m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask;
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].val =
                        m_hal_gpio_st_padmux_entry[pad_id].padmux[i].val;
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].regval = reg_val;
                    m_hal_gpio_st_pad_checkVal.infocount++;
                }

                mode_is_find = 1;
#if (ENABLE_CHECK_ALL_PAD_CONFLICT == 0)
                break;
#endif
            }
            else if ((m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode >= PINMUX_FOR_PM_PAD_EXT_MODE0_1)
                     && (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode <= PINMUX_FOR_PM_PAD_EXT_MODE44_1))
            {
                ext_item_id = i;
            }
            else if ((m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode >= PINMUX_FOR_SPI_EXT_EN_MODE0_1)
                     && (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode <= PINMUX_FOR_SPI_EXT_EN_MODE5_1))
            {
                ext_item_id = i;
            }
            else
            {
                if ((pad_id < PAD_RGMII0_MCLK) || (pad_id > PAD_RGMII0_MDC))
                {
                    if ((mode == PINMUX_FOR_GPIO_MODE)
                        && (m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mode > PRIORITY_GREATER_GPIO))
                        continue;
                }
                reg_val = _GPIO_R_WORD_MASK(reg_addr, m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask);
                if (reg_val == m_hal_gpio_st_padmux_entry[pad_id].padmux[i].val)
                {
                    mode_is_err++;

                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].base =
                        (u16)(m_hal_gpio_st_padmux_entry[pad_id].padmux[i].base >> 8);
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].offset =
                        m_hal_gpio_st_padmux_entry[pad_id].padmux[i].offset;
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].mask =
                        m_hal_gpio_st_padmux_entry[pad_id].padmux[i].mask;
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].val =
                        m_hal_gpio_st_padmux_entry[pad_id].padmux[i].val;
                    m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].regval = reg_val;
                    m_hal_gpio_st_pad_checkVal.infocount++;
                }
            }
        }
    }

    if ((mode_is_find) && (ext_item_id))
    {
        // set external data mode
        reg_addr = _RIUA_16BIT(m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].base,
                               m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].offset);
        reg_val  = _GPIO_R_WORD_MASK(reg_addr, 0xFFFF);
        reg_val &= (m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].mask);
        if (reg_val != m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].val)
        {
            mode_is_err++;

            m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].base =
                (u16)(m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].base >> 8);
            m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].offset =
                m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].offset;
            m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].mask =
                m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].mask;
            m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].val =
                m_hal_gpio_st_padmux_entry[pad_id].padmux[ext_item_id].val;
            m_hal_gpio_st_pad_checkVal.infos[m_hal_gpio_st_pad_checkVal.infocount].regval = reg_val;
            m_hal_gpio_st_pad_checkVal.infocount++;
        }
    }

    return (mode_is_find && !mode_is_err) ? 0 : 1;
}

s32 hal_gpio_pad_check_val(u32 pad_id, u32 mode)
{
    if (FALSE == hal_gpio_check_pin(pad_id))
    {
        return 1;
    }

    memset(&m_hal_gpio_st_pad_checkVal, 0, sizeof(m_hal_gpio_st_pad_checkVal));

    return hal_gpio_pad_check_mode_general(pad_id, mode);
}

u8 hal_gpio_pad_check_info_count(void)
{
    return m_hal_gpio_st_pad_checkVal.infocount;
}

void* hal_gpio_pad_check_info_get(u8 index)
{
    return (void*)&m_hal_gpio_st_pad_checkVal.infos[index];
}

u8 hal_gpio_padmux_to_val(u8* p_mode, u32* mode_to_val)
{
    u32 index;
    for (index = 0; index < (sizeof(m_hal_gpio_st_padmode_info_tbl) / sizeof(m_hal_gpio_st_padmode_info_tbl[0]));
         index++)
    {
        if (!strcmp((const char*)m_hal_gpio_st_padmode_info_tbl[index].pad_name, (const char*)p_mode))
        {
            *mode_to_val = index;
            return 0;
        }
    }
    return 1;
}

static u32 pad_id_list[GPIO_NR + 1] = {0};

u32* hal_gpio_padmdoe_to_padindex(u32 mode)
{
    u16 k     = 0;
    u16 i     = 0;
    u16 Count = 0;

    for (k = 0; k < sizeof(m_hal_gpio_st_padmux_entry) / sizeof(struct hal_gpio_st_padmux_entry); k++)
    {
        for (i = 0; i < m_hal_gpio_st_padmux_entry[k].size; i++)
        {
            if (mode == m_hal_gpio_st_padmux_entry[k].padmux[i].mode)
            {
                pad_id_list[Count] = m_hal_gpio_st_padmux_entry[k].padmux[i].pad_id;
                Count += 1;
            }
        }
    }
    pad_id_list[Count] = PAD_UNKNOWN;
    return pad_id_list;
}
