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
#define PMSLEEP_FIQ_START           0
#define INT_PMSLEEP_TIMER0          (PMSLEEP_FIQ_START + 0)
#define INT_PMSLEEP_TIMER1          (PMSLEEP_FIQ_START + 1)
#define INT_PMSLEEP_WDT             (PMSLEEP_FIQ_START + 2)
#define INT_PMSLEEP_TIMER2          (PMSLEEP_FIQ_START + 3)
#define INT_PMSLEEP_IR_RC           (PMSLEEP_FIQ_START + 4)
#define INT_PMSLEEP_CPU0TO2         (PMSLEEP_FIQ_START + 5)
#define INT_PMSLEEP_PM_XIU_TIMEOUT  (PMSLEEP_FIQ_START + 6)
#define INT_PMSLEEP_SAR_GPIO3       (PMSLEEP_FIQ_START + 7)
#define INT_PMSLEEP_SAR_GPIO2       (PMSLEEP_FIQ_START + 8)
#define INT_PMSLEEP_SAR_GPIO1       (PMSLEEP_FIQ_START + 9)
#define INT_PMSLEEP_SAR_GPIO0       (PMSLEEP_FIQ_START + 10)
#define INT_PMSLEEP_IR              (PMSLEEP_FIQ_START + 11)
#define INT_PMSLEEP_POWER_NO_GOOD_0 (PMSLEEP_FIQ_START + 12)
#define INT_PMSLEEP_TIMER5          (PMSLEEP_FIQ_START + 13)
#define INT_PMSLEEP_TIMER3          (PMSLEEP_FIQ_START + 14)
#define INT_PMSLEEP_TIMER4          (PMSLEEP_FIQ_START + 15)
#define INT_PMSLEEP_TIMER6          (PMSLEEP_FIQ_START + 16)
#define INT_PMSLEEP_TIMER7          (PMSLEEP_FIQ_START + 17)
#define INT_PMSLEEP_PIR_TX          (PMSLEEP_FIQ_START + 18)
#define INT_PMSLEEP_PIR_RX          (PMSLEEP_FIQ_START + 19)
#define INT_PMSLEEP_DUMMY_20        (PMSLEEP_FIQ_START + 20)
#define INT_PMSLEEP_DUMMY_21        (PMSLEEP_FIQ_START + 21)
#define INT_PMSLEEP_SAR_GPIO4       (PMSLEEP_FIQ_START + 22)
#define INT_PMSLEEP_HST1TO0         (PMSLEEP_FIQ_START + 23)
#define INT_PMSLEEP_PM_MODE_WAKEUP  (PMSLEEP_FIQ_START + 24)
#define INT_PMSLEEP_ISO_FAIL        (PMSLEEP_FIQ_START + 25)
#define INT_PMSLEEP_PAD2SDIO_SD_CDZ (PMSLEEP_FIQ_START + 26)
#define INT_PMSLEEP_PIR_INTMODE_2   (PMSLEEP_FIQ_START + 27)
#define INT_PMSLEEP_PIR_INTMODE_1   (PMSLEEP_FIQ_START + 28)
#define INT_PMSLEEP_PIR_INTMODE_0   (PMSLEEP_FIQ_START + 29)
#define INT_PMSLEEP_DUMMY_30        (PMSLEEP_FIQ_START + 30)
#define INT_PMSLEEP_DUMMY_31        (PMSLEEP_FIQ_START + 31)
#define INT_PMSLEEP_DUMMY_32        (PMSLEEP_FIQ_START + 32)
#define INT_PMSLEEP_DUMMY_33        (PMSLEEP_FIQ_START + 33)
#define INT_PMSLEEP_DUMMY_34        (PMSLEEP_FIQ_START + 34)
#define INT_PMSLEEP_DUMMY_35        (PMSLEEP_FIQ_START + 35)
#define INT_PMSLEEP_DUMMY_36        (PMSLEEP_FIQ_START + 36)
#define INT_PMSLEEP_DUMMY_37        (PMSLEEP_FIQ_START + 37)
#define INT_PMSLEEP_DUMMY_38        (PMSLEEP_FIQ_START + 38)
#define INT_PMSLEEP_DUMMY_39        (PMSLEEP_FIQ_START + 39)
#define INT_PMSLEEP_DUMMY_40        (PMSLEEP_FIQ_START + 40)
#define INT_PMSLEEP_DUMMY_41        (PMSLEEP_FIQ_START + 41)
#define INT_PMSLEEP_DUMMY_42        (PMSLEEP_FIQ_START + 42)
#define INT_PMSLEEP_DUMMY_43        (PMSLEEP_FIQ_START + 43)
#define INT_PMSLEEP_DUMMY_44        (PMSLEEP_FIQ_START + 44)
#define INT_PMSLEEP_DUMMY_45        (PMSLEEP_FIQ_START + 45)
#define INT_PMSLEEP_DUMMY_46        (PMSLEEP_FIQ_START + 46)
#define INT_PMSLEEP_DUMMY_47        (PMSLEEP_FIQ_START + 47)
#define PMSLEEP_FIQ_END             (PMSLEEP_FIQ_START + 48)
#define PMSLEEP_FIQ_NR              (PMSLEEP_FIQ_END - PMSLEEP_FIQ_START)

#define INT_PMSLEEP_INVALID 0xFF

#define PMSLEEP_IRQ_START                               PMSLEEP_FIQ_END
#define INT_PMSLEEP_IRQ_IRQ2HST0                        (PMSLEEP_IRQ_START + 0)
#define INT_PMSLEEP_IRQ_FIQ_OUT_PM                      (PMSLEEP_IRQ_START + 1)
#define INT_PMSLEEP_IRQ_PM_SLEEP                        (PMSLEEP_IRQ_START + 2)
#define INT_PMSLEEP_IRQ_SDIO                            (PMSLEEP_IRQ_START + 3)
#define INT_PMSLEEP_IRQ_DUMMY_04                        (PMSLEEP_IRQ_START + 4)
#define INT_PMSLEEP_IRQ_PM_UART                         (PMSLEEP_IRQ_START + 5)
#define INT_PMSLEEP_IRQ_SAR1                            (PMSLEEP_IRQ_START + 6)
#define INT_PMSLEEP_IRQ_RTC0                            (PMSLEEP_IRQ_START + 7)
#define INT_PMSLEEP_IRQ_FALG_POC                        (PMSLEEP_IRQ_START + 8)
#define INT_PMSLEEP_IRQ_ISP_VIF                         (PMSLEEP_IRQ_START + 9)
#define INT_PMSLEEP_IRQ_PSPI02HOST                      (PMSLEEP_IRQ_START + 10)
#define INT_PMSLEEP_IRQ_MIIC0                           (PMSLEEP_IRQ_START + 11)
#define INT_PMSLEEP_IRQ_RTC_GEN_ISO_FAIL                (PMSLEEP_IRQ_START + 12)
#define INT_PMSLEEP_IRQ_PM_FUART                        (PMSLEEP_IRQ_START + 13)
#define INT_PMSLEEP_IRQ_PM_BDMA                         (PMSLEEP_IRQ_START + 14)
#define INT_PMSLEEP_IRQ_PM_ARB                          (PMSLEEP_IRQ_START + 15)
#define INT_PMSLEEP_IRQ_PM_FUART_MERGE                  (PMSLEEP_IRQ_START + 16)
#define INT_PMSLEEP_IRQ_CHIP_TOP_POWERGOOD_DEGLITCH_MUX (PMSLEEP_IRQ_START + 17)
#define INT_PMSLEEP_IRQ_DUMMY_18                        (PMSLEEP_IRQ_START + 18)
#define INT_PMSLEEP_IRQ_SAR_KP                          (PMSLEEP_IRQ_START + 19)
#define INT_PMSLEEP_IRQ_SAR_GPIO_WK                     (PMSLEEP_IRQ_START + 20)
#define INT_PMSLEEP_IRQ_PM_ERROR_RESP                   (PMSLEEP_IRQ_START + 21)
#define INT_PMSLEEP_IRQ_DUMMY_22                        (PMSLEEP_IRQ_START + 22)
#define INT_PMSLEEP_IRQ_PM_FUART1                       (PMSLEEP_IRQ_START + 23)
#define INT_PMSLEEP_IRQ_PM_FUART1_MERGE                 (PMSLEEP_IRQ_START + 24)
#define INT_PMSLEEP_IRQ_PIR_RX                          (PMSLEEP_IRQ_START + 25)
#define INT_PMSLEEP_IRQ_PIR_TX                          (PMSLEEP_IRQ_START + 26)
#define INT_PMSLEEP_IRQ_SEC                             (PMSLEEP_IRQ_START + 27)
#define INT_PMSLEEP_IRQ_NSEC                            (PMSLEEP_IRQ_START + 28)
#define INT_PMSLEEP_IRQ_DUMMY_29                        (PMSLEEP_IRQ_START + 29)
#define INT_PMSLEEP_IRQ_DUMMY_30                        (PMSLEEP_IRQ_START + 30)
#define INT_PMSLEEP_IRQ_DUMMY_31                        (PMSLEEP_IRQ_START + 31)
#define INT_PMSLEEP_IRQ_DUMMY_32                        (PMSLEEP_IRQ_START + 32)
#define INT_PMSLEEP_IRQ_DUMMY_33                        (PMSLEEP_IRQ_START + 33)
#define INT_PMSLEEP_IRQ_DUMMY_34                        (PMSLEEP_IRQ_START + 34)
#define INT_PMSLEEP_IRQ_DUMMY_35                        (PMSLEEP_IRQ_START + 35)
#define INT_PMSLEEP_IRQ_DUMMY_36                        (PMSLEEP_IRQ_START + 36)
#define INT_PMSLEEP_IRQ_DUMMY_37                        (PMSLEEP_IRQ_START + 37)
#define INT_PMSLEEP_IRQ_DUMMY_38                        (PMSLEEP_IRQ_START + 38)
#define INT_PMSLEEP_IRQ_DUMMY_39                        (PMSLEEP_IRQ_START + 39)
#define INT_PMSLEEP_IRQ_DUMMY_40                        (PMSLEEP_IRQ_START + 40)
#define INT_PMSLEEP_IRQ_DUMMY_41                        (PMSLEEP_IRQ_START + 41)
#define INT_PMSLEEP_IRQ_DUMMY_42                        (PMSLEEP_IRQ_START + 42)
#define INT_PMSLEEP_IRQ_DUMMY_43                        (PMSLEEP_IRQ_START + 43)
#define INT_PMSLEEP_IRQ_DUMMY_44                        (PMSLEEP_IRQ_START + 44)
#define INT_PMSLEEP_IRQ_DUMMY_45                        (PMSLEEP_IRQ_START + 45)
#define INT_PMSLEEP_IRQ_DUMMY_46                        (PMSLEEP_IRQ_START + 46)
#define INT_PMSLEEP_IRQ_DUMMY_47                        (PMSLEEP_IRQ_START + 47)
#define PMSLEEP_IRQ_END                                 (PMSLEEP_IRQ_START + 48)
#define PMSLEEP_IRQ_NR                                  (PMSLEEP_IRQ_END - PMSLEEP_IRQ_START)
