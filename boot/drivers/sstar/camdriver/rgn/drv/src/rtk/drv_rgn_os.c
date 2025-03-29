/*
 * drv_rgn_os.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#define __DRV_RGN_OS_C__
#include "drv_rgn_os.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
bool DrvRgnOsSemInit(DrvRgnOsTsemConfig_t *pCfg)
{
    bool bRet;
    if (CamOsTsemInit(&pCfg->tSemCfg) != CAM_OS_OK)
    {
        bRet = RGN_RET_FAILURE;
    }
    else
    {
        bRet = RGN_RET_SUCCESS;
    }
    return bRet;
}

bool DrvRgnOsSemDeInit(DrvRgnOsTsemConfig_t *pCfg)
{
    bool bRet;
    if (CamOsTsemDeinit(&pCfg->tSemCfg) != CAM_OS_OK)
    {
        bRet = RGN_RET_FAILURE;
    }
    else
    {
        bRet = RGN_RET_SUCCESS;
    }

    return bRet
}

bool DrvRgnOsSemUp(DrvRgnOsTsemConfig_t *pCfg)
{
    bool bRet;
    if (CamOsTsemUp(&pCfg->tSemCfg) != CAM_OS_OK)
    {
        bRet = RGN_RET_FAILURE;
    }
    else
    {
        bRet = RGN_RET_SUCCESS;
    }

    return bRet
}

bool DrvRgnOsSemDown(DrvRgnOsTsemConfig_t *pCfg)
{
    bool bRet;
    if (CamOsTsemDown(&pCfg->tSemCfg) != CAM_OS_OK)
    {
        bRet = RGN_RET_FAILURE;
    }
    else
    {
        bRet = RGN_RET_SUCCESS;
    }
    return bRet;
}

void *DrvRgnOsMemAlloc(u32 u32Size)
{
    return CamOsMemAlloc(u32Size);
}

void DrvRgnOsMemRelease(void *pPtr)
{
    CamOsMemRelease(pPtr);
}

void DrvRgnOsSetGopClkEnable(void *pCtx) {}

void DrvRgnOsSetGopClkDisable(void *pCtx) {}

u32 DrvRgnOsGetChipRevision(void)
{
    return CamOsChipRevision();
}
