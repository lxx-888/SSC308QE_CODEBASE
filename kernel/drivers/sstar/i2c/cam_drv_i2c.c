/*
 * cam_drv_i2c.c- Sigmastar
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

#include <cam_drv_i2c.h>
#include <linux/i2c.h>
#include <drv_iic.h>

s32 CamI2cOpen(tI2cHandle *pHandle, u8 nPortNum)
{
#if defined(__RTK_OS__)
    pHandle->nPortNum = nPortNum;

#elif defined(__KERNEL__)
    pHandle->pAdapter = (void *)i2c_get_adapter(nPortNum);

#endif
    return 0;
}

s32 CamI2cTransfer(tI2cHandle *pHandle, tI2cMsg *pMsg, u32 nMsgNum)
{
#if defined(__RTK_OS__)
    return drv_i2c_master_xfer(pHandle->nPortNum, pMsg, nMsgNum);

#elif defined(__KERNEL__)
    return i2c_transfer((struct i2c_adapter *)pHandle->pAdapter, (struct i2c_msg *)pMsg, nMsgNum);

#endif
}

s32 CamI2cAsyncTransfer(tI2cHandle *pHandle, tI2cMsg *pMsg, u32 nMsgNum)
{
#if defined(__RTK_OS__)

#elif defined(__KERNEL__)
    return sstar_i2c_master_async_xfer((struct i2c_adapter *)pHandle->pAdapter, (struct i2c_msg *)pMsg, nMsgNum);
#endif
}

s32 CamI2cClose(tI2cHandle *pHandle)
{
#if defined(__RTK_OS__)
    pHandle->nPortNum = (-1);

#elif defined(__KERNEL__)

#endif
    return 0;
}

s32 CamI2cSetAsyncCb(tI2cHandle *pHandle, sstar_i2c_async_calbck CalBck, void *pReserved)
{
    s32 s32Ret = 0;

    sstar_i2c_async_cb_set((struct i2c_adapter *)pHandle->pAdapter, CalBck, pReserved);

    return s32Ret;
}

s32 CamI2cWnWrite(tI2cHandle *pHandle, tI2cMsg *pMsg, u32 nMsgNum, u32 WnLen, u16 WaitCnt)
{
#if defined(__RTK_OS__)

#elif defined(__KERNEL__)
    s32 s32Ret = 0;

    s32Ret |= sstar_i2c_wnwrite_xfer((struct i2c_adapter *)pHandle->pAdapter, (struct i2c_msg *)pMsg, nMsgNum, WnLen,
                                     WaitCnt);

    return s32Ret;
#endif
}
s32 CamI2cWnAsyncWrite(tI2cHandle *pHandle, tI2cMsg *pMsg, u32 nMsgNum, u32 WnLen, u16 WaitCnt)
{
#if defined(__RTK_OS__)

#elif defined(__KERNEL__)
    s32 s32Ret = 0;

    s32Ret |= sstar_i2c_wnwrite_async_xfer((struct i2c_adapter *)pHandle->pAdapter, (struct i2c_msg *)pMsg, nMsgNum,
                                           WnLen, WaitCnt);

    return s32Ret;
#endif
}

s32 CamI2cSetClk(void *pHandle, u32 pClk)
{
    // ToDo
    return 0;
}

#if defined(__KERNEL__)
EXPORT_SYMBOL(CamI2cOpen);
EXPORT_SYMBOL(CamI2cTransfer);
EXPORT_SYMBOL(CamI2cAsyncTransfer);
EXPORT_SYMBOL(CamI2cSetAsyncCb);
EXPORT_SYMBOL(CamI2cWnWrite);
EXPORT_SYMBOL(CamI2cWnAsyncWrite);
EXPORT_SYMBOL(CamI2cClose);
#endif
