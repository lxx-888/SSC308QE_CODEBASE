/*
 * msb250x_udc_reg.h- Sigmastar
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

#ifndef _MSB250X_UDC_REG_H
#define _MSB250X_UDC_REG_H

#define GET_REG16_ADDR(x, y) ((x) + ((y) << 2))
#define GET_REG8_ADDR(x, y)  ((x) + ((y) << 1) - ((y)&1))

/* 00h ~ 0Fh */
#define MSB250X_OTG0_FADDR_REG(x)  GET_REG8_ADDR(x, 0x00)
#define MSB250X_OTG0_PWR_REG(x)    GET_REG8_ADDR(x, 0x01)
#define MSB250X_OTG0_INTRTX_REG(x) GET_REG8_ADDR(x, 0x02)
/* 03h reserved */
#define MSB250X_OTG0_INTRRX_REG(x) GET_REG8_ADDR(x, 0x04)
/* 05h reserved */
#define MSB250X_OTG0_INTRTXE_REG(x) GET_REG8_ADDR(x, 0x06)
/* 07h reserved */
#define MSB250X_OTG0_INTRRXE_REG(x) GET_REG8_ADDR(x, 0x08)
/* 09h reserved */
#define MSB250X_OTG0_INTRUSB_REG(x)  GET_REG8_ADDR(x, 0x0A)
#define MSB250X_OTG0_INTRUSBE_REG(x) GET_REG8_ADDR(x, 0x0B)
#define MSB250X_OTG0_FRAME_L_REG(x)  GET_REG8_ADDR(x, 0x0C)
#define MSB250X_OTG0_FRAME_H_REG(x)  GET_REG8_ADDR(x, 0x0D)
#define MSB250X_OTG0_INDEX_REG(x)    GET_REG8_ADDR(x, 0x0E)
#define MSB250X_OTG0_TESTMODE_REG(x) GET_REG8_ADDR(x, 0x0F)

/* 10h ~ 1Fh*/
#define MSB250X_OTG0_TXMAP_L_REG(x) GET_REG8_ADDR(x, 0x10)
#define MSB250X_OTG0_TXMAP_H_REG(x) GET_REG8_ADDR(x, 0x11)
/* 12h ~ 1Fh for EP_SEL = 0 */
#define MSB250X_OTG0_CSR0_REG(x)      GET_REG8_ADDR(x, 0x12)
#define MSB250X_OTG0_CSR0_FLSH_REG(x) GET_REG8_ADDR(x, 0x13)
/* 14h ~ 17h reserved */
#define MSB250X_OTG0_COUNT0_REG(x) GET_REG8_ADDR(x, 0x18)
/* 19h ~ 1Eh reserved */
#define MSB250X_OTG0_CONFDATA_REG(x) GET_REG8_ADDR(x, 0x1F)
/* 12h ~ 1Fh for EP_SEL != 0 (EP1 ~ EPx) */
#define MSB250X_OTG0_TXCSR1_REG(x)    GET_REG8_ADDR(x, 0x12)
#define MSB250X_OTG0_TXCSR2_REG(x)    GET_REG8_ADDR(x, 0x13)
#define MSB250X_OTG0_RXMAP_L_REG(x)   GET_REG8_ADDR(x, 0x14)
#define MSB250X_OTG0_RXMAP_H_REG(x)   GET_REG8_ADDR(x, 0x15)
#define MSB250X_OTG0_RXCSR1_REG(x)    GET_REG8_ADDR(x, 0x16)
#define MSB250X_OTG0_RXCSR2_REG(x)    GET_REG8_ADDR(x, 0x17)
#define MSB250X_OTG0_RXCOUNT_L_REG(x) GET_REG8_ADDR(x, 0x18)
#define MSB250X_OTG0_RXCOUNT_H_REG(x) GET_REG8_ADDR(x, 0x19)
/* 1Ah ~ 1Eh reserved */
#define MSB250X_OTG0_FIFOSIZE_REG(x) GET_REG8_ADDR(x, 0x1F)

/* 20h ~ 5Fh*/
#define OTG0_EP_FIFO_ACCESS_L(x, y) GET_REG8_ADDR(x, (0x20 + ((y) << 2)))
/* 40h ~ 5Fh reserved for infinity5 */
/* 30h ~ 5Fh reserved for infinity6 */

/* 60h */
#define MSB250X_OTG0_DEVCTL_REG(x) GET_REG8_ADDR(x, 0x60)

/* 80h ~ 95h */
#define MSB250X_OTG0_USB_CFG0_L(x) GET_REG8_ADDR(x, 0x80)
#define MSB250X_OTG0_USB_CFG0_H(x) GET_REG8_ADDR(x, 0x81)
#define MSB250X_OTG0_USB_CFG1_L(x) GET_REG8_ADDR(x, 0x82)
#define MSB250X_OTG0_USB_CFG1_H(x) GET_REG8_ADDR(x, 0x83)
#define MSB250X_OTG0_USB_CFG2_L(x) GET_REG8_ADDR(x, 0x84)
#define MSB250X_OTG0_USB_CFG2_H(x) GET_REG8_ADDR(x, 0x85)
#define MSB250X_OTG0_USB_CFG3_L(x) GET_REG8_ADDR(x, 0x86)
#define MSB250X_OTG0_USB_CFG3_H(x) GET_REG8_ADDR(x, 0x87)
#define MSB250X_OTG0_USB_CFG4_L(x) GET_REG8_ADDR(x, 0x88)
#define MSB250X_OTG0_USB_CFG4_H(x) GET_REG8_ADDR(x, 0x89)
#define MSB250X_OTG0_USB_CFG5_L(x) GET_REG8_ADDR(x, 0x8A)
#define MSB250X_OTG0_USB_CFG5_H(x) GET_REG8_ADDR(x, 0x8B)
#define MSB250X_OTG0_USB_CFG6_L(x) GET_REG8_ADDR(x, 0x8C)
#define MSB250X_OTG0_USB_CFG6_H(x) GET_REG8_ADDR(x, 0x8D)
#define MSB250X_OTG0_USB_CFG7_L(x) GET_REG8_ADDR(x, 0x8E)
#define MSB250X_OTG0_USB_CFG7_H(x) GET_REG8_ADDR(x, 0x8F)
#define MSB250X_OTG0_USB_CFG8_L(x) GET_REG8_ADDR(x, 0x90)
#define MSB250X_OTG0_USB_CFG8_H(x) GET_REG8_ADDR(x, 0x91)
#define MSB250X_OTG0_USB_CFG9_L(x) GET_REG8_ADDR(x, 0x92)
#define MSB250X_OTG0_USB_CFG9_H(x) GET_REG8_ADDR(x, 0x93)
#define MSB250X_OTG0_USB_CFGA_L(x) GET_REG8_ADDR(x, 0x94)
#define MSB250X_OTG0_USB_CFGA_H(x) GET_REG8_ADDR(x, 0x95)

/* 100h ~ 180h */
#define MSB250X_OTG0_EP_TXMAP_L_REG(x, y) GET_REG8_ADDR(x, 0x100 + ((y) << 4))
#define MSB250X_OTG0_EP_TXMAP_H_REG(x, y) GET_REG8_ADDR(x, 0x101 + ((y) << 4))
/* for EP_SEL = 0, 102h ~ 10Fh (EP1 ~ EPx) */
#define MSB250X_OTG0_EP0_CSR0_REG(x)      GET_REG8_ADDR(x, 0x102)
#define MSB250X_OTG0_EP0_CSR0_FLSH_REG(x) GET_REG8_ADDR(x, 0x103)
/* 104h ~ 107h reserved */
#define MSB250X_OTG0_EP0_COUNT0_REG(x) GET_REG8_ADDR(x, 0x108)
/* 109h ~ 10Eh reserved */
#define MSB250X_OTG0_EP0_CONFDATA_REG(x) GET_REG8_ADDR(x, 0x10F)
/* for EP_SEL != 0, 112h ~ 11Fh (EP1 ~ EPx) */
#define MSB250X_OTG0_EP_TXCSR1_REG(x, y)    GET_REG8_ADDR(x, 0x102 + ((y) << 4))
#define MSB250X_OTG0_EP_TXCSR2_REG(x, y)    GET_REG8_ADDR(x, 0x103 + ((y) << 4))
#define MSB250X_OTG0_EP_RXMAP_L_REG(x, y)   GET_REG8_ADDR(x, 0x104 + ((y) << 4))
#define MSB250X_OTG0_EP_RXMAP_H_REG(x, y)   GET_REG8_ADDR(x, 0x105 + ((y) << 4))
#define MSB250X_OTG0_EP_RXCSR1_REG(x, y)    GET_REG8_ADDR(x, 0x106 + ((y) << 4))
#define MSB250X_OTG0_EP_RXCSR2_REG(x, y)    GET_REG8_ADDR(x, 0x107 + ((y) << 4))
#define MSB250X_OTG0_EP_RXCOUNT_L_REG(x, y) GET_REG8_ADDR(x, 0x108 + ((y) << 4))
#define MSB250X_OTG0_EP_RXCOUNT_H_REG(x, y) GET_REG8_ADDR(x, 0x109 + ((y) << 4))
#define MSB250X_OTG0_EP_FIFOSIZE_REG(x, y)  GET_REG8_ADDR(x, 0x10F + ((y) << 4))

/* 200h */
#define MSB250X_OTG0_DMA_INTR(x)        GET_REG8_ADDR(x, 0x200)
#define MSB250X_OTG0_DMA_CNTL(x, y)     GET_REG8_ADDR(x, 0x204 + ((y - 1) << 4))
#define MSB250X_OTG0_DMA_ADDR(x, y)     GET_REG8_ADDR(x, 0x208 + ((y - 1) << 4))
#define MSB250X_OTG0_DMA_ADDR_LW(x, y)  GET_REG8_ADDR(x, 0x208 + ((y - 1) << 4))
#define MSB250X_OTG0_DMA_ADDR_HW(x, y)  GET_REG8_ADDR(x, 0x20A + ((y - 1) << 4))
#define MSB250X_OTG0_DMA_COUNT(x, y)    GET_REG8_ADDR(x, 0x20C + ((y - 1) << 4))
#define MSB250X_OTG0_DMA_COUNT_LW(x, y) GET_REG8_ADDR(x, 0x20C + ((y - 1) << 4))
#define MSB250X_OTG0_DMA_COUNT_HW(x, y) GET_REG8_ADDR(x, 0x20E + ((y - 1) << 4))

/* custom */
#define MSB250X_OTG0_INTR_EP(x) (1 << x)

#define MSB250X_OTG0_AUTONAK0_EP_BULKOUT(x) MSB250X_OTG0_USB_CFG3_L(x)
#define MSB250X_OTG0_AUTONAK1_EP_BULKOUT(x) MSB250X_OTG0_USB_CFG0_H(x)
#define MSB250X_OTG0_AUTONAK2_EP_BULKOUT(x) MSB250X_OTG0_USB_CFG8_L(x)

#define MSB250X_OTG0_AUTONAK0_RX_PKT_CNT(x) MSB250X_OTG0_USB_CFG5_L(x)
#define MSB250X_OTG0_AUTONAK1_RX_PKT_CNT(x) MSB250X_OTG0_USB_CFG1_L(x)
#define MSB250X_OTG0_AUTONAK2_RX_PKT_CNT(x) MSB250X_OTG0_USB_CFG9_L(x)

//#define MSB250X_OTG0_AUTONAK0_CTRL                  MSB250X_OTG0_USB_CFG5_H
#define MSB250X_OTG0_AUTONAK0_CTRL(x) MSB250X_OTG0_USB_CFG5_L(x)
#define MSB250X_OTG0_AUTONAK1_CTRL(x) MSB250X_OTG0_USB_CFG0_H(x)
#define MSB250X_OTG0_AUTONAK2_CTRL(x) MSB250X_OTG0_USB_CFG8_L(x)

#define MSB250X_OTG0_AUTONAK0_EN 0x2000
#define MSB250X_OTG0_AUTONAK1_EN 0x10
#define MSB250X_OTG0_AUTONAK2_EN 0x0100

#define MSB250X_OTG0_AUTONAK0_OK2Rcv 0x8000
#define MSB250X_OTG0_AUTONAK1_OK2Rcv 0x40
#define MSB250X_OTG0_AUTONAK2_OK2Rcv 0x0400

#define MSB250X_OTG0_AUTONAK0_AllowAck 0x4000
#define MSB250X_OTG0_AUTONAK1_AllowAck 0x20
#define MSB250X_OTG0_AUTONAK2_AllowAck 0x04

#define MSB250X_OTG0_DMA_MODE_CTL  MSB250X_OTG0_USB_CFG5_L
#define MSB250X_OTG0_DMA_MODE_CTL1 (MSB250X_OTG0_USB_CFG0_L + 1)

#define MSB250X_OTG0_CFG1_H_SHORT_ECO          0x40
#define MSB250X_OTG0_CFG6_H_SHORT_MODE         0x20
#define MSB250X_OTG0_CFG6_H_BUS_OP_FIX         0x40
#define MSB250X_OTG0_CFG6_H_REG_MI_WDFIFO_CTRL 0x80

/* new mode1 in Peripheral mode */
#define M_Mode1_P_BulkOut_EP   0x0002
#define M_Mode1_P_OK2Rcv       0x8000
#define M_Mode1_P_AllowAck     0x4000
#define M_Mode1_P_Enable       0x2000
#define M_Mode1_P_NAK_Enable   0x2000
#define M_Mode1_P_NAK_Enable_1 0x10
#define M_Mode1_P_AllowAck_1   0x20
#define M_Mode1_P_OK2Rcv_1     0x40

/* MSB250X_OTG0_PWR_REG */ /* RW */
#define MSB250X_OTG0_PWR_ISOUP     (1 << 7)
#define MSB250X_OTG0_PWR_SOFT_CONN (1 << 6)
#define MSB250X_OTG0_PWR_HS_EN     (1 << 5)
#define MSB250X_OTG0_PWR_HS_MODE   (1 << 4)
#define MSB250X_OTG0_PWR_RESET     (1 << 3)
#define MSB250X_OTG0_PWR_RESUME    (1 << 2)
#define MSB250X_OTG0_PWR_SUSPEND   (1 << 1)
#define MSB250X_OTG0_PWR_ENSUSPEND (1 << 0)

/* MSB250X_OTG0_INTRUSB_REG */ /* RO */
#define MSB250X_OTG0_INTRUSB_VBUS_ERR (1 << 7)
#define MSB250X_OTG0_INTRUSB_SESS_REQ (1 << 6)
#define MSB250X_OTG0_INTRUSB_DISCONN  (1 << 5)
#define MSB250X_OTG0_INTRUSB_CONN     (1 << 4)
#define MSB250X_OTG0_INTRUSB_SOF      (1 << 3)
#define MSB250X_OTG0_INTRUSB_RESET    (1 << 2)
#define MSB250X_OTG0_INTRUSB_RESUME   (1 << 1)
#define MSB250X_OTG0_INTRUSB_SUSPEND  (1 << 0)

/* MSB250X_OTG0_INTRUSBE_REG */ /* RW */
#define MSB250X_OTG0_INTRUSBE_VBUS_ERR (1 << 7)
#define MSB250X_OTG0_INTRUSBE_SESS_REQ (1 << 6)
#define MSB250X_OTG0_INTRUSBE_DISCONN  (1 << 5)
#define MSB250X_OTG0_INTRUSBE_CONN     (1 << 4)
#define MSB250X_OTG0_INTRUSBE_SOF      (1 << 3)
#define MSB250X_OTG0_INTRUSBE_RESET    (1 << 2)
#define MSB250X_OTG0_INTRUSBE_BABBLE   (1 << 2)
#define MSB250X_OTG0_INTRUSBE_RESUME   (1 << 1)
#define MSB250X_OTG0_INTRUSBE_SUSPEND  (1 << 0)

/* MSB250X_OTG0_TESTMODE_REG */ /* RW */
#define MSB250X_OTG0_TESTMODE_FORCE_HOST   (1 << 7)
#define MSB250X_OTG0_TESTMODE_FIFO_ACCESS  (1 << 6)
#define MSB250X_OTG0_TESTMODE_FORCE_FS     (1 << 5)
#define MSB250X_OTG0_TESTMODE_FORCE_HS     (1 << 4)
#define MSB250X_OTG0_TESTMODE_TEST_PACKET  (1 << 3)
#define MSB250X_OTG0_TESTMODE_TEST_K       (1 << 2)
#define MSB250X_OTG0_TESTMODE_TEST_J       (1 << 1)
#define MSB250X_OTG0_TESTMODE_TEST_SE0_NAK (1 << 0)

/* MSB250X_OTG0_CSR0_REG */ /* RO, WO */
#define MSB250X_OTG0_CSR0_SSETUPEND (1 << 7)
#define MSB250X_OTG0_CSR0_SRXPKTRDY (1 << 6)
#define MSB250X_OTG0_CSR0_SENDSTALL (1 << 5)
#define MSB250X_OTG0_CSR0_SETUPEND  (1 << 4)
#define MSB250X_OTG0_CSR0_DATAEND   (1 << 3)
#define MSB250X_OTG0_CSR0_SENTSTALL (1 << 2)
#define MSB250X_OTG0_CSR0_TXPKTRDY  (1 << 1)
#define MSB250X_OTG0_CSR0_RXPKTRDY  (1 << 0)

/* CSR0 in host mode */
#define MSB250X_OTG0_CSR0_STATUSPACKET (1 << 6)
#define MSB250X_OTG0_CSR0_REQPACKET    (1 << 5)
#define MSB250X_OTG0_CSR0_SETUPPACKET  (1 << 3)
#define MSB250X_OTG0_CSR0_RXSTALL      (1 << 2)

/* MSB250X_OTG0_TXCSR1_REG */ /* RO, WO */
#define MSB250X_OTG0_TXCSR1_AUTOSET     (1 << 15)
#define MSB250X_OTG0_TXCSR1_MODE        (1 << 13)
#define MSB250X_OTG0_TXCSR1_DMAREQENAB  (1 << 12)
#define MSB250X_OTG0_TXCSR1_FRCDATAOG   (1 << 11)
#define MSB250X_OTG0_TXCSR1_DMAREQMODE  (1 << 10)
#define MSB250X_OTG0_TXCSR1_CLRDATAOTG  (1 << 6)
#define MSB250X_OTG0_TXCSR1_SENTSTALL   (1 << 5)
#define MSB250X_OTG0_TXCSR1_SENDSTALL   (1 << 4)
#define MSB250X_OTG0_TXCSR1_FLUSHFIFO   (1 << 3)
#define MSB250X_OTG0_TXCSR1_UNDERRUN    (1 << 2)
#define MSB250X_OTG0_TXCSR1_FIFONOEMPTY (1 << 1)
#define MSB250X_OTG0_TXCSR1_TXPKTRDY    (1 << 0)

/* host mode */
#define MSB250X_OTG0_TXCSR1_RXSTALL (1 << 5)

/* MSB250X_OTG0_TXCSR2_REG */ /* RW */
#define MSB250X_OTG0_TXCSR2_AUTOSET    (1 << 7)
#define MSB250X_OTG0_TXCSR2_ISOC       (1 << 6)
#define MSB250X_OTG0_TXCSR2_MODE       (1 << 5)
#define MSB250X_OTG0_TXCSR2_DMAREQENAB (1 << 4)
#define MSB250X_OTG0_TXCSR2_FRCDATAOG  (1 << 3)
#define MSB250X_OTG0_TXCSR2_DMAREQMODE (1 << 2)

/* MSB250X_OTG0_RXCSR1_REG */ /* RW, RO */
#define MSB250X_OTG0_RXCSR1_CLRDATATOG (1 << 7)
#define MSB250X_OTG0_RXCSR1_SENTSTALL  (1 << 6)
#define MSB250X_OTG0_RXCSR1_SENDSTALL  (1 << 5)
#define MSB250X_OTG0_RXCSR1_FLUSHFIFO  (1 << 4)
#define MSB250X_OTG0_RXCSR1_DATAERROR  (1 << 3)
#define MSB250X_OTG0_RXCSR1_OVERRUN    (1 << 2)
#define MSB250X_OTG0_RXCSR1_FIFOFULL   (1 << 1)
#define MSB250X_OTG0_RXCSR1_RXPKTRDY   (1 << 0)

/* host mode */
#define MSB250X_OTG0_RXCSR1_RXSTALL (1 << 6)
#define MSB250X_OTG0_RXCSR1_REQPKT  (1 << 5)

/* MSB250X_OTG0_RXCSR2_REG */ /* RW */
#define MSB250X_OTG0_RXCSR2_AUTOCLR  (1 << 7)
#define MSB250X_OTG0_RXCSR2_ISOC     (1 << 6)
#define MSB250X_OTG0_RXCSR2_DMAREQEN (1 << 5)
#define MSB250X_OTG0_RXCSR2_DISNYET  (1 << 4)
#define MSB250X_OTG0_RXCSR2_DMAREQMD (1 << 3)

/* MSB250X_OTG0_DEVCTL_REG */
#define MSB250X_OTG0_B_DEVIC   (1 << 7)
#define MSB250X_OTG0_FS_DEVIC  (1 << 6)
#define MSB250X_OTG0_LS_DEVIC  (1 << 5)
#define MSB250X_OTG0_HOST_MODE (1 << 2)
#define MSB250X_OTG0_HOST_REQ  (1 << 1)
#define MSB250X_OTG0_SESSION   (1 << 0)

/* CH_DMA_CNTL */
#define MSB250X_OTG0_DMA_BURST_MODE (3 << 9)
#define MSB250X_OTG0_DMA_INT_EN     (1 << 3)
#define MSB250X_OTG0_DMA_AUTO       (1 << 2)
#define MSB250X_OTG0_DMA_TX         (1 << 1)
#define MSB250X_OTG0_EN_DMA         (1 << 0)

/* USB_CFG0_L */
#define MSB250X_OTG0_CFG0_SRST_N (1 << 0)

#define RXCSR2_MODE1 (MSB250X_OTG0_RXCSR2_AUTOCLR | MSB250X_OTG0_RXCSR2_DMAREQEN | MSB250X_OTG0_RXCSR2_DMAREQMD)
#define TXCSR2_MODE1 (MSB250X_OTG0_TXCSR2_DMAREQENAB | MSB250X_OTG0_TXCSR2_AUTOSET | MSB250X_OTG0_TXCSR2_DMAREQMODE)

struct otg0_ep_txcsr_h
{
    __u8 bUnused : 2;
    __u8 bDMAReqMode : 1;
    __u8 bFrcDataTog : 1;
    __u8 bDMAReqEnab : 1;
    __u8 bMode : 1;
    __u8 bISO : 1;
    __u8 bAutoSet : 1;
} __attribute__((packed));

struct otg0_ep_txcsr_l
{
    __u8 bTxPktRdy : 1;
    __u8 bFIFONotEmpty : 1;
    __u8 bUnderRun : 1;
    __u8 bFlushFIFO : 1;
    __u8 bSendStall : 1;
    __u8 bSentStall : 1;
    __u8 bClrDataTog : 1;
    __u8 bIncompTx : 1;
} __attribute__((packed));

struct otg0_ep_rxcsr_h
{
    __u8 bIncompRx : 1;
    __u8 bUnused : 2;
    __u8 bDMAReqMode : 1;
    __u8 bDisNyet : 1;
    __u8 bDMAReqEnab : 1;
    __u8 bISO : 1;
    __u8 bAutoClear : 1;
} __attribute__((packed));

struct otg0_ep_rxcsr_l
{
    __u8 bRxPktRdy : 1;
    __u8 bFIFOFull : 1;
    __u8 bOverRun : 1;
    __u8 bDataError : 1;
    __u8 bFlushFIFO : 1;
    __u8 bSendStall : 1;
    __u8 bSentStall : 1;
    __u8 bClrDataTog : 1;
} __attribute__((packed));

struct otg0_ep0_csr_h
{
    __u8 bFlushFIF0 : 1;
    __u8 bUnused : 7;
} __attribute__((packed));

struct otg0_ep0_csr_l
{
    __u8 bRxPktRdy : 1;
    __u8 bTxPktRdy : 1;
    __u8 bSentStall : 1;
    __u8 bDataEnd : 1;
    __u8 bSetupEnd : 1;
    __u8 bSendStall : 1;
    __u8 bServicedRxPktRdy : 1;
    __u8 bServicedSetupEnd : 1;
} __attribute__((packed));

struct otg0_usb_power
{
    __u8 bEnableSuspendM : 1;
    __u8 bSuspendMode : 1;
    __u8 bResume : 1;
    __u8 bReset : 1;
    __u8 bHSMode : 1;
    __u8 bHSEnab : 1;
    __u8 bSoftConn : 1;
    __u8 bISOUpdate : 1;
} __attribute__((packed));

struct otg0_usb_intr
{
    __u8 bSuspend : 1;
    __u8 bResume : 1;
    __u8 bReset : 1;
    __u8 bSOF : 1;
    __u8 bConn : 1;
    __u8 bDiscon : 1;
    __u8 bSessReq : 1;
    __u8 bVBusError : 1;
} __attribute__((packed));

struct otg0_usbe_intr
{
    __u8 bSuspend : 1;
    __u8 bResume : 1;
    __u8 bReset : 1;
    __u8 bSOF : 1;
    __u8 bConn : 1;
    __u8 bDiscon : 1;
    __u8 bSessReq : 1;
    __u8 bVBusError : 1;
} __attribute__((packed));
struct otg0_usb_cfg0_l
{
    __u8 bSRST_N : 1;
    __u8 bOTG_TM_1 : 1;
    __u8 bDebugSel : 4;
    __u8 bUSBOTG : 1;
    __u8 bMIUPriority : 1;
} __attribute__((packed));

struct otg0_usb_cfg0_h
{
    __u8 bEP_BULKOUT_1 : 4;
    __u8 bECO4NAK_EN_1 : 1;
    __u8 bSetAllow_ACK_1 : 1;
    __u8 bSet_OK2Rcv_1 : 1;
    __u8 bDMPullDown : 1;
} __attribute__((packed));

struct otg0_usb_cfg6_h
{
    __u8 bDMABugFix : 1;
    __u8 bDMAMCU_RD_Fix : 1;
    __u8 bDMAMCU_WR_Fix : 1;
    __u8 bINT_WR_CLR_EN : 1;
    __u8 bMCU_HLT_DMA_EN : 1;
    __u8 bShortMode : 1;
    __u8 bBusOPFix : 1;
    __u8 bREG_MI_WDFIFO_CTRL : 1;
} __attribute__((packed));

struct utmi_signal_status_l
{
    __u8 bVBUSVALID : 1;
    __u8 bAVALID : 1;
    __u8 bBVALID : 1;
    __u8 bIDDIG : 1;
    __u8 bHOSTDISCON : 1;
    __u8 bSESSEND : 1;
    __u8 bLINESTATE : 2;
} __attribute__((packed));

struct usbc0_rst_ctrl_l
{
    __u8 bUSB_RST : 1;
    __u8 bUHC_RST : 1;
    __u8 bOTG_RST : 1;
    __u8 bREG_SUSPEND : 1;
    __u8 bReserved0 : 1;
    __u8 bUHC_XIU_ENABLE : 1;
    __u8 bOTG_XIU_ENABLE : 1;
    __u8 bReserved1 : 1;
} __attribute__((packed));

struct usbc0_pwr_mng_eve_enable_l
{
    __u8 bWAKEUP_INTEN : 1;
    __u8 bDEV_DET_INTEN : 1;
    __u8 bRESUME_INTEN : 1;
    __u8 bRESET_INTEN : 1;
    __u8 bCONN_VBUS_INTEN : 1;
    __u8 bCONN_AVAL_INTEN : 1;
    __u8 bREG_PV2MI_WAIT : 1;
    __u8 bUHC_DBUS_MASK : 1;
} __attribute__((packed));
struct otg0_dma_ctrlrequest
{
    __u8 bEnableDMA : 1;
    __u8 bDirection : 1;
    __u8 bDMAMode : 1;
    __u8 bInterruptEnable : 1;
    __u8 bEndpointNumber : 4;
    __u8 bBusError : 1;
    __u8 bRurstMode : 2;
    __u8 bReserved : 5;
} __attribute__((packed));

struct otg0_ep_tx_maxp
{
    __u16 wMaximumPayload : 11;
    __u8  bMult : 2;
    __u8  bUnused : 3;
} __attribute__((packed));

#endif /* _MSB250X_OTG0_REG_H */
