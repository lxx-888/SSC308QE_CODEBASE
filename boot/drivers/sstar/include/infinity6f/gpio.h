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

#define PAD_PWM_OUT0           0
#define PAD_PWM_OUT1           1
#define PAD_PWM_OUT2           2
#define PAD_PWM_OUT3           3
#define PAD_PWM_OUT4           4
#define PAD_PWM_OUT5           5
#define PAD_PWM_OUT6           6
#define PAD_PWM_OUT7           7
#define PAD_SD0_GPIO0          8
#define PAD_SD0_VCTRL          9
#define PAD_SD0_CDZ            10
#define PAD_SD0_D1             11
#define PAD_SD0_D0             12
#define PAD_SD0_CLK            13
#define PAD_SD0_CMD            14
#define PAD_SD0_D3             15
#define PAD_SD0_D2             16
#define PAD_I2C0_SCL           17
#define PAD_I2C0_SDA           18
#define PAD_SR_RST0            19
#define PAD_SR_MCLK0           20
#define PAD_I2C1_SCL           21
#define PAD_I2C1_SDA           22
#define PAD_SR_RST1            23
#define PAD_SR_MCLK1           24
#define PAD_OUTP_RX0_CH0       25
#define PAD_OUTN_RX0_CH0       26
#define PAD_OUTP_RX0_CH1       27
#define PAD_OUTN_RX0_CH1       28
#define PAD_OUTP_RX0_CH2       29
#define PAD_OUTN_RX0_CH2       30
#define PAD_OUTP_RX0_CH3       31
#define PAD_OUTN_RX0_CH3       32
#define PAD_OUTP_RX0_CH4       33
#define PAD_OUTN_RX0_CH4       34
#define PAD_OUTP_RX0_CH5       35
#define PAD_OUTN_RX0_CH5       36
#define PAD_OUTP_RX1_CH0       37
#define PAD_OUTN_RX1_CH0       38
#define PAD_OUTP_RX1_CH1       39
#define PAD_OUTN_RX1_CH1       40
#define PAD_OUTP_RX1_CH2       41
#define PAD_OUTN_RX1_CH2       42
#define PAD_OUTP_RX1_CH3       43
#define PAD_OUTN_RX1_CH3       44
#define PAD_OUTP_RX1_CH4       45
#define PAD_OUTN_RX1_CH4       46
#define PAD_OUTP_RX2_CH0       47
#define PAD_OUTN_RX2_CH0       48
#define PAD_OUTP_RX2_CH1       49
#define PAD_OUTN_RX2_CH1       50
#define PAD_OUTP_RX2_CH2       51
#define PAD_OUTN_RX2_CH2       52
#define PAD_I2C2_SCL           53
#define PAD_I2C2_SDA           54
#define PAD_SR_RST2            55
#define PAD_SR_MCLK2           56
#define PAD_I2C3_SCL           57
#define PAD_I2C3_SDA           58
#define PAD_SR_RST3            59
#define PAD_SR_MCLK3           60
#define PAD_ISP0_XVS           61
#define PAD_ISP0_XHS           62
#define PAD_ISP0_XTRIG         63
#define PAD_PM_SR1_D0          64
#define PAD_PM_SR1_D4          65
#define PAD_PM_SR1_PCLK        66
#define PAD_PM_SR1_RST         67
#define PAD_PM_SR1_INT         68
#define PAD_PM_MI2C1_SCL       69
#define PAD_PM_MI2C1_SDA       70
#define PAD_PM_SR1_D1          71
#define PAD_PM_SR1_D2          72
#define PAD_PM_SR1_D3          73
#define PAD_PM_SR1_D5          74
#define PAD_PM_SR1_D6          75
#define PAD_PM_SR1_D7          76
#define PAD_PM_SR1_VS          77
#define PAD_PM_SR1_HS          78
#define PAD_PM_DMIC0_CLK       79
#define PAD_PM_DMIC0_D0        80
#define PAD_PM_SPI_CK          81
#define PAD_PM_SPI_DI          82
#define PAD_PM_SPI_HLD         83
#define PAD_PM_SPI_DO          84
#define PAD_PM_SPI_WPZ         85
#define PAD_PM_SPI_CZ          86
#define PAD_PM_RADAR_SAR_GPIO0 87
#define PAD_PM_RADAR_SAR_GPIO1 88
#define PAD_PM_RADAR_SAR_GPIO2 89
#define PAD_PM_RADAR_SAR_GPIO3 90
#define PAD_PM_UART_RX         91
#define PAD_PM_UART_TX         92
#define PAD_PM_INTOUT          93
#define PAD_PM_GPIO0           94
#define PAD_PM_GPIO1           95
#define PAD_PM_GPIO2           96
#define PAD_PM_GPIO3           97
#define PAD_PM_GPIO4           98
#define PAD_PM_GPIO5           99
#define PAD_PM_FUART_RX        100
#define PAD_PM_FUART_TX        101
#define PAD_PM_IRIN            102
#define PAD_PM_MSPI0_DO        103
#define PAD_PM_MSPI0_DI        104
#define PAD_PM_MSPI0_CK        105
#define PAD_PM_MSPI0_CZ        106
#define PAD_PM_USB3_INT        107
#define PAD_PM_USB3_ID         108
#define PAD_SAR_GPIO0          109
#define PAD_SAR_GPIO1          110
#define PAD_RGMII0_MCLK        111
#define PAD_RGMII0_RSTN        112
#define PAD_RGMII0_RXCLK       113
#define PAD_RGMII0_RXCTL       114
#define PAD_RGMII0_RXD0        115
#define PAD_RGMII0_RXD1        116
#define PAD_RGMII0_RXD2        117
#define PAD_RGMII0_RXD3        118
#define PAD_RGMII0_TXCLK       119
#define PAD_RGMII0_TXCTL       120
#define PAD_RGMII0_TXD0        121
#define PAD_RGMII0_TXD1        122
#define PAD_RGMII0_TXD2        123
#define PAD_RGMII0_TXD3        124
#define PAD_RGMII0_MDIO        125
#define PAD_RGMII0_MDC         126
#define PAD_SD1_GPIO0          127
#define PAD_SD1_GPIO1          128
#define PAD_SD1_CDZ            129
#define PAD_SD1_D1             130
#define PAD_SD1_D0             131
#define PAD_SD1_CLK            132
#define PAD_SD1_CMD            133
#define PAD_SD1_D3             134
#define PAD_SD1_D2             135
#define PAD_OUTP_TX0_CH0       136
#define PAD_OUTN_TX0_CH0       137
#define PAD_OUTP_TX0_CH1       138
#define PAD_OUTN_TX0_CH1       139
#define PAD_OUTP_TX0_CH2       140
#define PAD_OUTN_TX0_CH2       141
#define PAD_OUTP_TX0_CH3       142
#define PAD_OUTN_TX0_CH3       143
#define PAD_OUTP_TX0_CH4       144
#define PAD_OUTN_TX0_CH4       145
#define PAD_EMMC_RSTN          146
#define PAD_EMMC_CLK           147
#define PAD_EMMC_CMD           148
#define PAD_EMMC_DS            149
#define PAD_EMMC_D3            150
#define PAD_EMMC_D4            151
#define PAD_EMMC_D0            152
#define PAD_EMMC_D5            153
#define PAD_EMMC_D1            154
#define PAD_EMMC_D6            155
#define PAD_EMMC_D2            156
#define PAD_EMMC_D7            157
#define PAD_ETH_LED0           158
#define PAD_ETH_LED1           159
#define PAD_I2C4_SCL           160
#define PAD_I2C4_SDA           161
#define PAD_SPI_CK             162
#define PAD_SPI_DI             163
#define PAD_SPI_HLD            164
#define PAD_SPI_DO             165
#define PAD_SPI_WPZ            166
#define PAD_SPI_CZ             167
#define PAD_UART_RX            168
#define PAD_UART_TX            169

#define GPIO_NR     170
#define PAD_UNKNOWN 0xFFFF

#endif // #ifndef ___GPIO_H
