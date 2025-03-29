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
    return RGN_RET_SUCCESS;
}

bool DrvRgnOsSemDeInit(DrvRgnOsTsemConfig_t *pCfg)
{
    return RGN_RET_SUCCESS;
}

bool DrvRgnOsSemUp(DrvRgnOsTsemConfig_t *pCfg)
{
    return RGN_RET_SUCCESS;
}

bool DrvRgnOsSemDown(DrvRgnOsTsemConfig_t *pCfg)
{
    return RGN_RET_SUCCESS;
}

void *DrvRgnOsMemAlloc(u32 u32Size)
{
    return malloc(u32Size);
}

void DrvRgnOsMemRelease(void *pPtr)
{
    free(pPtr);
}

void DrvRgnOsSetGopClkEnable(void *pCtx) {}

void DrvRgnOsSetGopClkDisable(void *pCtx) {}

u32 DrvRgnOsGetChipRevision(void)
{
    return 0;
}
