/*
 * gpio.h- Sigmastar
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
#ifndef ___GPIO_H
#define ___GPIO_H

#define PAD_I2C1_SDA        0
#define PAD_SR_RST0         1
#define PAD_SR_MCLK0        2
#define PAD_I2C1_SCL        3
#define PAD_SR_RST1         4
#define PAD_SR_MCLK1        5
#define PAD_I2C2_SCL        6
#define PAD_I2C2_SDA        7
#define PAD_GPIO0           8
#define PAD_GPIO1           9
#define PAD_GPIO2           10
#define PAD_GPIO3           11
#define PAD_GPIO4           12
#define PAD_GPIO5           13
#define PAD_GPIO6           14
#define PAD_GPIO7           15
#define PAD_GPIO8           16
#define PAD_GPIO9           17
#define PAD_GPIO10          18
#define PAD_GPIO11          19
#define PAD_SD0_CDZ         20
#define PAD_SD0_D1          21
#define PAD_SD0_D0          22
#define PAD_SD0_CLK         23
#define PAD_SD0_CMD         24
#define PAD_SD0_D3          25
#define PAD_SD0_D2          26
#define PAD_FUART_RTS       27
#define PAD_FUART_CTS       28
#define PAD_FUART_RX        29
#define PAD_FUART_TX        30
#define PAD_MSPI_CZ         31
#define PAD_MSPI_DO         32
#define PAD_MSPI_DI         33
#define PAD_MSPI_CK         34
#define PAD_SPI0_DO         35
#define PAD_SPI0_DI         36
#define PAD_SPI0_HLD        37
#define PAD_SPI0_WPZ        38
#define PAD_SPI0_CZ         39
#define PAD_SPI0_CK         40
#define PAD_I2C0_SDA        41
#define PAD_I2C0_SCL        42
#define PAD_PM_GPIO12       43
#define PAD_PM_GPIO11       44
#define PAD_PM_UART_TX      45
#define PAD_PM_UART_RX      46
#define PAD_PM_PSPI0_INT    47
#define PAD_PM_PSPI0_DI     48
#define PAD_PM_PSPI0_DO     49
#define PAD_PM_PSPI0_CK     50
#define PAD_PM_PSPI0_CZ     51
#define PAD_PM_GPIO10       52
#define PAD_PM_GPIO9        53
#define PAD_PM_GPIO8        54
#define PAD_PM_GPIO7        55
#define PAD_PM_PWM3         56
#define PAD_PM_PWM2         57
#define PAD_PM_PWM1         58
#define PAD_PM_PWM0         59
#define PAD_PM_GPIO6        60
#define PAD_PM_GPIO5        61
#define PAD_PM_GPIO4        62
#define PAD_PM_UART2_TX     63
#define PAD_PM_UART2_RX     64
#define PAD_PM_I2C_CLK      65
#define PAD_PM_I2C_SDA      66
#define PAD_PM_SDIO_INT     67
#define PAD_PM_GPIO3        68
#define PAD_PM_GPIO2        69
#define PAD_PM_GPIO1        70
#define PAD_PM_GPIO0        71
#define PAD_PM_SDIO_D1      72
#define PAD_PM_SDIO_D0      73
#define PAD_PM_SDIO_CLK     74
#define PAD_PM_SDIO_CMD     75
#define PAD_PM_SDIO_D3      76
#define PAD_PM_SDIO_D2      77
#define PAD_PM_FUART_RTS    78
#define PAD_PM_FUART_CTS    79
#define PAD_PM_FUART_RX     80
#define PAD_PM_FUART_TX     81
#define PAD_PM_HSRAM_GPIO0  82
#define PAD_PM_HSRAM_GPIO1  83
#define PAD_PM_HSRAM_GPIO2  84
#define PAD_PM_HSRAM_GPIO3  85
#define PAD_PM_HSRAM_GPIO4  86
#define PAD_PM_HSRAM_GPIO5  87
#define PAD_PM_HSRAM_GPIO6  88
#define PAD_PM_HSRAM_GPIO7  89
#define PAD_PM_HSRAM_GPIO8  90
#define PAD_PM_HSRAM_GPIO9  91
#define PAD_PM_HSRAM_GPIO10 92
#define PAD_PM_HSRAM_GPIO11 93
#define PAD_PM_HSRAM_GPIO12 94
#define PAD_PM_SAR_GPIO0    95
#define PAD_PM_SAR_GPIO1    96
#define PAD_PM_SAR_GPIO2    97
#define PAD_PM_SAR_GPIO3    98
#define PAD_PM_SAR_GPIO4    99
#define PAD_OUTN_RX0_CH5    100
#define PAD_OUTP_RX0_CH5    101
#define PAD_OUTN_RX0_CH4    102
#define PAD_OUTP_RX0_CH4    103
#define PAD_OUTN_RX0_CH3    104
#define PAD_OUTP_RX0_CH3    105
#define PAD_OUTN_RX0_CH2    106
#define PAD_OUTP_RX0_CH2    107
#define PAD_OUTN_RX0_CH1    108
#define PAD_OUTP_RX0_CH1    109
#define PAD_OUTN_RX0_CH0    110
#define PAD_OUTP_RX0_CH0    111
#define PAD_ENG_TEST0       112

#define GPIO_NR     113
#define PAD_UNKNOWN 0xFFFF

#endif // #ifndef ___GPIO_H
