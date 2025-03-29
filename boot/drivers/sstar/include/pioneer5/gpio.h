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

#define PAD_I2C5_SCL      0
#define PAD_I2C5_SDA      1
#define PAD_SD0_GPIO0     2
#define PAD_SD0_VCTRL     3
#define PAD_SD0_CDZ       4
#define PAD_SD0_D1        5
#define PAD_SD0_D0        6
#define PAD_SD0_CLK       7
#define PAD_SD0_CMD       8
#define PAD_SD0_D3        9
#define PAD_SD0_D2        10
#define PAD_KEY0          11
#define PAD_KEY1          12
#define PAD_KEY2          13
#define PAD_KEY3          14
#define PAD_KEY4          15
#define PAD_KEY5          16
#define PAD_KEY6          17
#define PAD_KEY7          18
#define PAD_KEY8          19
#define PAD_KEY9          20
#define PAD_KEY10         21
#define PAD_KEY11         22
#define PAD_KEY12         23
#define PAD_KEY13         24
#define PAD_PM_PWM0       25
#define PAD_PM_PWM1       26
#define PAD_PM_I2CM_SCL   27
#define PAD_PM_I2CM_SDA   28
#define PAD_PM_UART_RX0   29
#define PAD_PM_UART_TX0   30
#define PAD_PM_IR_RX      31
#define PAD_PM_GPIO0      32
#define PAD_PM_GPIO1      33
#define PAD_PM_GPIO2      34
#define PAD_PM_GPIO3      35
#define PAD_PM_GPIO4      36
#define PAD_PM_GPIO5      37
#define PAD_PM_SPI_WPZ    38
#define PAD_PM_SPI_DO     39
#define PAD_PM_SPI_CZ     40
#define PAD_PM_SPI_HLD    41
#define PAD_PM_SPI_CK     42
#define PAD_PM_SPI_DI     43
#define PAD_SAR_GPIO0     44
#define PAD_SAR_GPIO1     45
#define PAD_EMMC_RSTN     46
#define PAD_EMMC_CLK      47
#define PAD_EMMC_CMD      48
#define PAD_EMMC_DS       49
#define PAD_EMMC_D3       50
#define PAD_EMMC_D4       51
#define PAD_EMMC_D0       52
#define PAD_EMMC_D5       53
#define PAD_EMMC_D1       54
#define PAD_EMMC_D6       55
#define PAD_EMMC_D2       56
#define PAD_EMMC_D7       57
#define PAD_OUTP_RX1_CH_0 58
#define PAD_OUTN_RX1_CH_0 59
#define PAD_OUTP_RX1_CH_1 60
#define PAD_OUTN_RX1_CH_1 61
#define PAD_OUTP_RX1_CH_2 62
#define PAD_OUTN_RX1_CH_2 63
#define PAD_OUTP_RX1_CH_3 64
#define PAD_OUTN_RX1_CH_3 65
#define PAD_OUTP_RX0_CH_0 66
#define PAD_OUTN_RX0_CH_0 67
#define PAD_OUTP_RX0_CH_1 68
#define PAD_OUTN_RX0_CH_1 69
#define PAD_OUTP_RX0_CH_2 70
#define PAD_OUTN_RX0_CH_2 71
#define PAD_OUTP_RX0_CH_3 72
#define PAD_OUTN_RX0_CH_3 73
#define PAD_OUTP_RX0_CH_4 74
#define PAD_OUTN_RX0_CH_4 75
#define PAD_OUTP_RX0_CH_5 76
#define PAD_OUTN_RX0_CH_5 77
#define PAD_SPDIF_TX      78
#define PAD_SR_IO3        79
#define PAD_SR_IO2        80
#define PAD_SR_IO1        81
#define PAD_SR_IO0        82
#define PAD_SR_PDN1       83
#define PAD_SR_MCLK1      84
#define PAD_SR_RST1       85
#define PAD_I2C1_SDA      86
#define PAD_I2C1_SCL      87
#define PAD_SR_PDN0       88
#define PAD_SR_MCLK0      89
#define PAD_SR_RST0       90
#define PAD_I2C0_SDA      91
#define PAD_I2C0_SCL      92
#define PAD_SR_PDN2       93
#define PAD_SR_MCLK2      94
#define PAD_SR_RST2       95
#define PAD_I2C2_SDA      96
#define PAD_I2C2_SCL      97
#define PAD_SR_PDN3       98
#define PAD_SR_MCLK3      99
#define PAD_SR_RST3       100
#define PAD_I2C3_SDA      101
#define PAD_I2C3_SCL      102
#define PAD_PWM_OUT0      103
#define PAD_PWM_OUT1      104
#define PAD_PWM_OUT2      105
#define PAD_PWM_OUT3      106
#define PAD_PWM_OUT4      107
#define PAD_PWM_OUT5      108
#define PAD_PWM_OUT6      109
#define PAD_PWM_OUT7      110
#define PAD_PWM_OUT8      111
#define PAD_PWM_OUT9      112
#define PAD_PWM_OUT10     113
#define PAD_PWM_OUT11     114
#define PAD_OUTP_TX0_CH_0 115
#define PAD_OUTN_TX0_CH_0 116
#define PAD_OUTP_TX0_CH_1 117
#define PAD_OUTN_TX0_CH_1 118
#define PAD_OUTP_TX0_CH_2 119
#define PAD_OUTN_TX0_CH_2 120
#define PAD_OUTP_TX0_CH_3 121
#define PAD_OUTN_TX0_CH_3 122
#define PAD_OUTP_TX0_CH_4 123
#define PAD_OUTN_TX0_CH_4 124
#define PAD_OUTP_TX1_CH_0 125
#define PAD_OUTN_TX1_CH_0 126
#define PAD_OUTP_TX1_CH_1 127
#define PAD_OUTN_TX1_CH_1 128
#define PAD_OUTP_TX1_CH_2 129
#define PAD_OUTN_TX1_CH_2 130
#define PAD_OUTP_TX1_CH_3 131
#define PAD_OUTN_TX1_CH_3 132
#define PAD_OUTP_TX1_CH_4 133
#define PAD_OUTN_TX1_CH_4 134
#define PAD_SAR_ADC_0     135
#define PAD_SAR_ADC_1     136
#define PAD_SAR_ADC_2     137
#define PAD_SAR_ADC_3     138
#define PAD_SAR_ADC_4     139
#define PAD_SAR_ADC_5     140
#define PAD_SAR_ADC_6     141
#define PAD_SAR_ADC_7     142
#define PAD_SAR_ADC_8     143
#define PAD_SAR_ADC_9     144
#define PAD_SAR_ADC_10    145
#define PAD_SAR_ADC_11    146
#define PAD_SAR_ADC_12    147
#define PAD_SAR_ADC_13    148
#define PAD_SAR_ADC_14    149
#define PAD_SAR_ADC_15    150
#define PAD_SAR_ADC_16    151
#define PAD_SAR_ADC_17    152
#define PAD_SAR_ADC_18    153
#define PAD_SAR_ADC_19    154
#define PAD_SAR_ADC_20    155
#define PAD_SAR_ADC_21    156
#define PAD_SAR_ADC_22    157
#define PAD_SAR_ADC_23    158
#define PAD_SR_IO4        159
#define PAD_RGMII0_MCLK   160
#define PAD_RGMII0_RSTN   161
#define PAD_RGMII0_RXCLK  162
#define PAD_RGMII0_RXCTL  163
#define PAD_RGMII0_RXD0   164
#define PAD_RGMII0_RXD1   165
#define PAD_RGMII0_RXD2   166
#define PAD_RGMII0_RXD3   167
#define PAD_RGMII0_TXCLK  168
#define PAD_RGMII0_TXCTL  169
#define PAD_RGMII0_TXD0   170
#define PAD_RGMII0_TXD1   171
#define PAD_RGMII0_TXD2   172
#define PAD_RGMII0_TXD3   173
#define PAD_RGMII0_MDIO   174
#define PAD_RGMII0_MDC    175
#define PAD_UART_RX2      176
#define PAD_UART_TX2      177
#define PAD_UART_RX3      178
#define PAD_UART_TX3      179
#define PAD_UART_RX4      180
#define PAD_UART_TX4      181
#define PAD_UART_RX1      182
#define PAD_UART_TX1      183
#define PAD_FUART_RX      184
#define PAD_FUART_TX      185
#define PAD_FUART_RTS     186
#define PAD_FUART_CTS     187

#define GPIO_NR     188
#define PAD_UNKNOWN 0xFFFF

#define PAD_PM_START PAD_PM_PWM0
#define PAD_PM_END   PAD_SAR_GPIO1
#define PAD_PM_NR    (PAD_PM_END - PAD_PM_START + 1)

#endif // #ifndef ___GPIO_H
