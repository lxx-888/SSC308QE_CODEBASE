/*
 * pmsleep-irqs.h- Sigmastar
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
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/

/*  Not used in dtsi,
if need to get the interrupt number for request_irq(), use gpio_to_irq() to obtain irq number.
Or manual calculate the number is
GIC_SGI_NR+GIC_PPI_NR+GIC_SPI_ARM_INTERNAL_NR+GIC_SPI_MS_IRQ_NR+GIC_SPI_MS_FIQ_NR+X=160+X   */
/* MS_PM_SLEEP_FIQ 0-31 */
#define PMSLEEP_FIQ_START                  0
#define INT_PMSLEEP_PAD_PM_SR1_D0          (PMSLEEP_FIQ_START + 0)
#define INT_PMSLEEP_PAD_PM_SR1_D1          (PMSLEEP_FIQ_START + 1)
#define INT_PMSLEEP_PAD_PM_SR1_D2          (PMSLEEP_FIQ_START + 2)
#define INT_PMSLEEP_PAD_PM_SR1_D3          (PMSLEEP_FIQ_START + 3)
#define INT_PMSLEEP_PAD_PM_SR1_D4          (PMSLEEP_FIQ_START + 4)
#define INT_PMSLEEP_PAD_PM_DMIC0_CLK       (PMSLEEP_FIQ_START + 5)
#define INT_PMSLEEP_PAD_PM_DMIC0_D0        (PMSLEEP_FIQ_START + 6)
#define INT_PMSLEEP_PAD_PM_SPI_CK          (PMSLEEP_FIQ_START + 7)
#define INT_PMSLEEP_PAD_PM_SPI_DI          (PMSLEEP_FIQ_START + 8)
#define INT_PMSLEEP_PAD_PM_SPI_HLD         (PMSLEEP_FIQ_START + 9)
#define INT_PMSLEEP_PAD_PM_SPI_DO          (PMSLEEP_FIQ_START + 10)
#define INT_PMSLEEP_PAD_PM_SPI_WPZ         (PMSLEEP_FIQ_START + 11)
#define INT_PMSLEEP_PAD_PM_SPI_CZ          (PMSLEEP_FIQ_START + 12)
#define INT_PMSLEEP_PAD_PM_GPIO3           (PMSLEEP_FIQ_START + 13)
#define INT_PMSLEEP_PAD_PM_GPIO4           (PMSLEEP_FIQ_START + 14)
#define INT_PMSLEEP_PAD_PM_GPIO5           (PMSLEEP_FIQ_START + 15)
#define INT_PMSLEEP_PAD_PM_SR1_D5          (PMSLEEP_FIQ_START + 16)
#define INT_PMSLEEP_PAD_PM_SR1_D6          (PMSLEEP_FIQ_START + 17)
#define INT_PMSLEEP_PAD_PM_SR1_D7          (PMSLEEP_FIQ_START + 18)
#define INT_PMSLEEP_PAD_PM_SR1_VS          (PMSLEEP_FIQ_START + 19)
#define INT_PMSLEEP_PAD_PM_SR1_HS          (PMSLEEP_FIQ_START + 20)
#define INT_PMSLEEP_PAD_PM_SR1_PCLK        (PMSLEEP_FIQ_START + 21)
#define INT_PMSLEEP_PAD_PM_SR1_INT         (PMSLEEP_FIQ_START + 22)
#define INT_PMSLEEP_PAD_PM_UART_RX         (PMSLEEP_FIQ_START + 23)
#define INT_PMSLEEP_PAD_PM_UART_TX         (PMSLEEP_FIQ_START + 24)
#define INT_PMSLEEP_PAD_PM_RADAR_SAR_GPIO0 (PMSLEEP_FIQ_START + 25)
#define INT_PMSLEEP_PAD_PM_RADAR_SAR_GPIO1 (PMSLEEP_FIQ_START + 26)
#define INT_PMSLEEP_PAD_PM_RADAR_SAR_GPIO2 (PMSLEEP_FIQ_START + 27)
#define INT_PMSLEEP_PAD_PM_RADAR_SAR_GPIO3 (PMSLEEP_FIQ_START + 28)
#define INT_PMSLEEP_PAD_PM_IRIN            (PMSLEEP_FIQ_START + 29)
#define INT_PMSLEEP_PAD_PM_USB3_INT        (PMSLEEP_FIQ_START + 30)
#define INT_PMSLEEP_PAD_PM_SR1_RST         (PMSLEEP_FIQ_START + 31)
#define INT_PMSLEEP_PAD_PM_MI2C1_SCL       (PMSLEEP_FIQ_START + 32)
#define INT_PMSLEEP_PAD_PM_MI2C1_SDA       (PMSLEEP_FIQ_START + 33)
#define INT_PMSLEEP_PAD_PM_INTOUT          (PMSLEEP_FIQ_START + 34)
#define INT_PMSLEEP_PAD_PM_MSPI0_CK        (PMSLEEP_FIQ_START + 35)
#define INT_PMSLEEP_PAD_PM_MSPI0_CZ        (PMSLEEP_FIQ_START + 36)
#define INT_PMSLEEP_PAD_PM_MSPI0_DO        (PMSLEEP_FIQ_START + 37)
#define INT_PMSLEEP_PAD_PM_MSPI0_DI        (PMSLEEP_FIQ_START + 38)
#define INT_PMSLEEP_PAD_PM_FUART_RX        (PMSLEEP_FIQ_START + 39)
#define INT_PMSLEEP_PAD_PM_FUART_TX        (PMSLEEP_FIQ_START + 40)
#define INT_PMSLEEP_PAD_PM_GPIO0           (PMSLEEP_FIQ_START + 41)
#define INT_PMSLEEP_PAD_PM_GPIO1           (PMSLEEP_FIQ_START + 42)
#define INT_PMSLEEP_PAD_PM_GPIO2           (PMSLEEP_FIQ_START + 43)
#define INT_PMSLEEP_PAD_PM_USB3_ID         (PMSLEEP_FIQ_START + 44)
#define INT_PMSLEEP_PAD_RTC_IO0            (PMSLEEP_FIQ_START + 49)
#define INT_PMSLEEP_PAD_RTC_IO1            (PMSLEEP_FIQ_START + 50)
#define INT_PMSLEEP_PAD_RTC_IO2            (PMSLEEP_FIQ_START + 51)
#define INT_PMSLEEP_PAD_RTC_IO3            (PMSLEEP_FIQ_START + 52)
#define PMSLEEP_FIQ_END                    (PMSLEEP_FIQ_START + 53)
#define PMSLEEP_FIQ_NR                     (PMSLEEP_FIQ_END - PMSLEEP_FIQ_START)

#define INT_PMSLEEP_INVALID 0xFF

#define PMSLEEP_IRQ_START        PMSLEEP_FIQ_END
#define INT_PMSLEEP_IRQ_DUMMY_00 (PMSLEEP_IRQ_START + 0)
#define INT_PMSLEEP_IRQ_SAR      (PMSLEEP_IRQ_START + 1)
#define INT_PMSLEEP_IRQ_WOL      (PMSLEEP_IRQ_START + 2)
#define INT_PMSLEEP_IRQ_DUMMY_03 (PMSLEEP_IRQ_START + 3)
#define INT_PMSLEEP_IRQ_RTC      (PMSLEEP_IRQ_START + 4)
#define INT_PMSLEEP_IRQ_DUMMY_05 (PMSLEEP_IRQ_START + 5)
#define INT_PMSLEEP_IRQ_DUMMY_06 (PMSLEEP_IRQ_START + 6)
#define INT_PMSLEEP_IRQ_DUMMY_07 (PMSLEEP_IRQ_START + 7)
#define PMSLEEP_IRQ_END          (PMSLEEP_IRQ_START + 8)
#define PMSLEEP_IRQ_NR           (PMSLEEP_IRQ_END - PMSLEEP_IRQ_START)
