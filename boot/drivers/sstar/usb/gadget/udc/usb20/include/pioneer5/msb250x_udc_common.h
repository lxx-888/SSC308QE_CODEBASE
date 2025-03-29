/*
 * msb250x_udc_common.h - Sigmastar
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

#ifndef _MSB250X_UDC_COMMON_H_
#define _MSB250X_UDC_COMMON_H_
#define GET_REG16_ADDR(x, y) ((x) + ((y) << 2))
#define GET_REG8_ADDR(x, y)  ((x) + ((y) << 1) - ((y)&1))
#define RIU_BASE_ADDR        0x1F000000
#define UTMI_BASE_ADDR       GET_REG8_ADDR(RIU_BASE_ADDR, 0x142100)
#define USBC_BASE_ADDR       GET_REG8_ADDR(RIU_BASE_ADDR, 0x142300)
#define OTG0_BASE_ADDR       GET_REG8_ADDR(RIU_BASE_ADDR, 0x142500)
#define CHIPTOP_BASE_ADDR    GET_REG8_ADDR(RIU_BASE_ADDR, 0x101E00)

#define MIU0_BASE_ADDR 0x20000000

#define MIU0_SIZE ((unsigned long)0x10000000)

#define MIU0_BUS_BASE_ADDR ((unsigned long)0x00000000)
#define MIU1_BUS_BASE_ADDR ((unsigned long)0x80000000)

#define USB_MIU_SEL0 ((u8)0x70U)
#define USB_MIU_SEL1 ((u8)0xefU)
#define USB_MIU_SEL2 ((u8)0xefU)
#define USB_MIU_SEL3 ((u8)0xefU)

#define MSB250X_MAX_ENDPOINTS   7
#define MSB250X_USB_DMA_CHANNEL 3

#define ENABLE_OTG_USB_NEW_MIU_SLE 1

#define USB_SUPPORT_WAKEUP_XTAL 1

#define EP0_FIFO_SIZE 64
#define EP_FIFO_SIZE  512

#define MSB250X_EPS_CAP(_dev, _ep_op)              \
    /* control endpoint */                         \
    .ep[0] =                                       \
        {                                          \
            .ep =                                  \
                {                                  \
                    .name      = ep0name,          \
                    .ops       = _ep_op,           \
                    .maxpacket = EP0_FIFO_SIZE,    \
                },                                 \
            .dev    = _dev,                        \
            .ep_num = 0,                           \
    }, /* first group of endpoints */              \
        .ep[1] =                                   \
            {                                      \
                .ep =                              \
                    {                              \
                        .name      = "ep1",        \
                        .ops       = _ep_op,       \
                        .maxpacket = EP_FIFO_SIZE, \
                    },                             \
                .dev       = _dev,                 \
                .ep_num    = 1,                    \
                .fifo_size = 8192,                 \
    },                                             \
    .ep[2] =                                       \
        {                                          \
            .ep =                                  \
                {                                  \
                    .name      = "ep2",            \
                    .ops       = _ep_op,           \
                    .maxpacket = EP_FIFO_SIZE,     \
                },                                 \
            .dev       = _dev,                     \
            .ep_num    = 2,                        \
            .fifo_size = 1024,                     \
    },                                             \
    .ep[3] =                                       \
        {                                          \
            .ep =                                  \
                {                                  \
                    .name      = "ep3in-int",      \
                    .ops       = _ep_op,           \
                    .maxpacket = 64,               \
                },                                 \
            .dev       = _dev,                     \
            .ep_num    = 3,                        \
            .fifo_size = 64,                       \
    },                                             \
    .ep[4] =                                       \
        {                                          \
            .ep =                                  \
                {                                  \
                    .name      = "ep4",            \
                    .ops       = _ep_op,           \
                    .maxpacket = EP_FIFO_SIZE,     \
                },                                 \
            .dev       = _dev,                     \
            .ep_num    = 4,                        \
            .fifo_size = 512,                      \
    },                                             \
    .ep[5] =                                       \
        {                                          \
            .ep =                                  \
                {                                  \
                    .name      = "ep5",            \
                    .ops       = _ep_op,           \
                    .maxpacket = EP_FIFO_SIZE,     \
                },                                 \
            .dev       = _dev,                     \
            .ep_num    = 5,                        \
            .fifo_size = 512,                      \
    },                                             \
    .ep[6] = {                                     \
        .ep =                                      \
            {                                      \
                .name      = "ep6",                \
                .ops       = _ep_op,               \
                .maxpacket = 64,                   \
            },                                     \
        .dev       = _dev,                         \
        .ep_num    = 6,                            \
        .fifo_size = 64,                           \
    }

#endif /* _MSB250X_UDC_COMMON_H_ */
