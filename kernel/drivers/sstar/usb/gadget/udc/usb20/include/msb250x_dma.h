/*
 * msb250x_dma.h- Sigmastar
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

#ifndef __MSB250X_DMA_H
#define __MSB250X_DMA_H

u8   msb250x_dma_find_channel_by_ep(struct msb250x_udc *udc, u8 ep_num);
void msb250x_dma_release_channel(struct msb250x_udc *udc, s8 ch);
int  msb250x_dma_setup_control(struct usb_ep *ep, struct msb250x_request *mreq, u32 bytes);
void msb250x_dma_isr_handler(u8 ch, struct msb250x_udc *dev);

#endif
