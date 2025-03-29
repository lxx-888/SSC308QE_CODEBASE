/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_vcm_common.h>

#ifdef __cplusplus
}
#endif

VCM_DRV_ENTRY_IMPL_BEGIN(LC898249);

// ============================================================================
#define VcmReg_Read(_reg, _data)    (pVcmCusHandle->i2c_bus->i2c_rx(pVcmCusHandle->i2c_bus, &(pVcmCusHandle->i2c_cfg), _reg, _data))
#define VcmReg_Write(_reg, _data)   (pVcmCusHandle->i2c_bus->i2c_tx(pVcmCusHandle->i2c_bus, &(pVcmCusHandle->i2c_cfg), _reg, _data))
#define VcmRegArrayR(_reg, _len)    (pVcmCusHandle->i2c_bus->i2c_array_rx(pVcmCusHandle->i2c_bus, &(pVcmCusHandle->i2c_cfg), (_reg), (_len)))
#define VcmRegArrayW(_reg, _len)    (pVcmCusHandle->i2c_bus->i2c_array_tx(pVcmCusHandle->i2c_bus, &(pVcmCusHandle->i2c_cfg), (_reg), (_len)))

#define VCM_I2C_LEGACY  I2C_NORMAL_MODE
#define VCM_I2C_FMT     I2C_FMT_A8D16
#define VCM_I2C_SPEED   200000
#define VCM_I2C_ADDR    0xE4

#define VCM_I2C_EEPROM_ADDR             0xE6
#define VCM_I2C_CALIBRATION_DX1_REG     0x38
#define VCM_I2C_CALIBRATION_DX2_REG     0x39
#define VCM_I2C_CALIBRATION_DX3_REG     0x3A
#define VCM_I2C_EEPROM_REG_BASE         0x0100
#define VCM_I2C_EEPROM_FOBDN_ADDR_BEGIN 0x0100
#define VCM_I2C_EEPROM_FOBDN_ADDR_END   0x013F

#define MAX_POS                         1023

#define COLOR_RED                       "\033[0;31m"
#define COLOR_NONE                      "\033[0m"

// ============================================================================

static CamOsMutex_t Lock;
bool gXUCmdMode = false;

static int pCus_vcm_GetEEPROMValue(ss_cus_vcm *pVcmCusHandle, u16 addr, u16 *value)
{
    CamOsMutexLock(&Lock);

    pVcmCusHandle->i2c_cfg.address = VCM_I2C_EEPROM_ADDR;
    pVcmCusHandle->i2c_cfg.fmt = I2C_FMT_A8D8;

    *value = 0x00;
    VcmReg_Read(addr, value);

    pVcmCusHandle->i2c_cfg.address = VCM_I2C_ADDR;
    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    CamOsMutexUnlock(&Lock);

    return SUCCESS;
}

#if 0
static int pCus_vcm_SetEEPROMValue(ss_cus_vcm *pVcmCusHandle, u16 addr, u16 value)
{
    u16 data = 0x00,backup = 0x00;

    if ((addr != VCM_I2C_CALIBRATION_DX1_REG) && (addr != VCM_I2C_CALIBRATION_DX2_REG) && (addr != VCM_I2C_CALIBRATION_DX3_REG) &&
        (addr != (VCM_I2C_CALIBRATION_DX1_REG + VCM_I2C_EEPROM_REG_BASE)) && (addr != (VCM_I2C_CALIBRATION_DX2_REG + VCM_I2C_EEPROM_REG_BASE)) &&
        (addr != (VCM_I2C_CALIBRATION_DX3_REG + VCM_I2C_EEPROM_REG_BASE)))
    {
        /* block write data to the EEPROM forbidden area */
        CamOsPrintf("\r\n");
        CamOsPrintf(COLOR_RED"LC898249 EEPROM write address (%04X), it's forbidden!\r\n"COLOR_NONE, addr);
        CamOsPrintf("\r\n");
        return SUCCESS;
    }

    CamOsMutexLock(&Lock);

    pVcmCusHandle->i2c_cfg.fmt = I2C_FMT_A8D8;

    VcmReg_Read(0xF0, &data);

    SENSOR_MDELAY(10);

    VcmReg_Read(0xE0, &data);

    VcmReg_Read(0x81, &backup);
    VcmReg_Write(0x81, backup & 0x7F);

    SENSOR_MDELAY(110);

    VcmReg_Write(0x98, 0xE2);
    VcmReg_Write(0x99, 0xA2);

    SENSOR_MDELAY(10);

    pVcmCusHandle->i2c_cfg.address = VCM_I2C_EEPROM_ADDR;

    VcmReg_Write(addr, value);

    SENSOR_MDELAY(10);

    pVcmCusHandle->i2c_cfg.address = VCM_I2C_ADDR;

    VcmReg_Read(0xE1, &data);

    VcmReg_Write(0x98, 0x00);
    VcmReg_Write(0x99, 0x00);

    VcmReg_Write(0x81, backup);

    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    CamOsMutexUnlock(&Lock);

    return SUCCESS;
}

static int pCus_vcm_GetMotionStatus(ss_cus_vcm *pVcmCusHandle, u16 *pos)
{
    u16 data = 0x00;

    CamOsMutexLock(&Lock);

    pVcmCusHandle->i2c_cfg.fmt = I2C_FMT_A8D8;

    VcmReg_Read(0xB0, &data);

    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    *pos = data & 0x80;

    CamOsMutexUnlock(&Lock);

    return SUCCESS;
}

static int pCus_vcm_SetPosition(ss_cus_vcm *pVcmCusHandle, u32 pos)
{
    CamOsMutexLock(&Lock);

    VcmReg_Write(0x84, pos & 0x03FF);

    CamOsMutexUnlock(&Lock);

    return SUCCESS;
}

static int pCus_vcm_GetPosition(ss_cus_vcm *pVcmCusHandle, u32 *pos)
{
    u16 data = 0x00;

    CamOsMutexLock(&Lock);

    *pos = 0;

    pVcmCusHandle->i2c_cfg.fmt = I2C_FMT_A8D8;

    VcmReg_Read(0xB0, &data);

    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    if((data & 0x80) != 0x00)
    {
        CamOsMutexUnlock(&Lock);

        return FAIL;
    }

    data = 0x00;

    VcmReg_Read(0x0A, &data);

    *pos = ((data + 16384) & 0x0000FFFF) * 1023 / 32768;

    CamOsMutexUnlock(&Lock);

    return SUCCESS;
}


static void pCus_vcm_SetI2CValue(ss_cus_vcm *pVcmCusHandle, u16 addr, u16 fmt, u16 reg, u16 value)
{
    CamOsMutexLock(&Lock);

    pVcmCusHandle->i2c_cfg.address = addr;
    pVcmCusHandle->i2c_cfg.fmt = fmt;

    VcmReg_Write(reg, value);

    pVcmCusHandle->i2c_cfg.address = VCM_I2C_ADDR;
    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    CamOsMutexUnlock(&Lock);
}

static void pCus_vcm_GetI2CValue(ss_cus_vcm *pVcmCusHandle, u16 addr, u16 fmt, u16 reg, u16 *value)
{
    CamOsMutexLock(&Lock);

    pVcmCusHandle->i2c_cfg.address = addr;
    pVcmCusHandle->i2c_cfg.fmt = fmt;

    *value = 0x00;
    VcmReg_Read(reg, value);

    pVcmCusHandle->i2c_cfg.address = VCM_I2C_ADDR;
    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    CamOsMutexUnlock(&Lock);
}
#endif

static int pCus_vcm_PowerOn(ss_cus_vcm *pVcmCusHandle, u32 idx)
{
    u16 data = 0x00;
    CamOsMutexInit(&Lock);

    SENSOR_MDELAY(10);
    pVcmCusHandle->i2c_cfg.fmt = I2C_FMT_A8D8;
    VcmReg_Read(0xF0, &data);
    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    return SUCCESS;
}

static int pCus_vcm_PowerOff(ss_cus_vcm *pVcmCusHandle, u32 idx)
{
    CamOsMutexDestroy(&Lock);

    return SUCCESS;
}
static int InitStat = FAIL;
static u32 MotorPos = 0;
s8 gCalibrationDX1 = 0,gCalibrationDX2 = 0,gCalibrationDX3 = 0;

ss_cus_vcm *pgVcmHandle = 0;

static int pCus_vcm_Init(ss_cus_vcm *pVcmCusHandle)
{
    int ret = SUCCESS;
    u16 data = 0;

    pVcmCusHandle->i2c_cfg.fmt = I2C_FMT_A8D8;
    ret = VcmReg_Write(0xE0, 0x01);
    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    SENSOR_MDELAY(50);

    pCus_vcm_GetEEPROMValue(pVcmCusHandle,VCM_I2C_CALIBRATION_DX1_REG,&data);
    gCalibrationDX1 = data;
    data = 0;
    pCus_vcm_GetEEPROMValue(pVcmCusHandle,VCM_I2C_CALIBRATION_DX3_REG,&data);
    gCalibrationDX3 = data;
    data = 0;
   /* pCus_vcm_GetEEPROMValue(pVcmCusHandle,VCM_I2C_CALIBRATION_DX2_REG,&data);
    gCalibrationDX2 = data;*/
    //gCalibrationDX2 = (int)((gCalibrationDX1 + (gCalibrationDX3 - gCalibrationDX1) * 46.0 / 176.0) + 0.5);

    InitStat = ret;

    if(InitStat)
        CamOsPrintf(COLOR_RED"LC898249 fail, ret: %d\r\n"COLOR_NONE, InitStat);
    else
        CamOsPrintf(COLOR_RED"LC898249 pass \r\n"COLOR_NONE);

    return ret;
}

static int pCus_vcm_SetPos(ss_cus_vcm *pVcmCusHandle, u32 pos)
{
    if(gXUCmdMode == true)
        return SUCCESS;

    MotorPos = pos;

    if(pos > MAX_POS)
        return FAIL;
    else if (InitStat == FAIL)
        return SUCCESS;

    CamOsMutexLock(&Lock);

    VcmReg_Write(0x84, pos & 0x03FF);

    SENSOR_MDELAY(5);

    CamOsMutexUnlock(&Lock);

    return SUCCESS;
}

static int pCus_vcm_GetCurPos(ss_cus_vcm *pVcmCusHandle, u32 *cur_pos)
{
    u16 data = 0x00;

    if (InitStat == FAIL || gXUCmdMode == true)
        return SUCCESS;

    CamOsMutexLock(&Lock);

    pVcmCusHandle->i2c_cfg.fmt = I2C_FMT_A8D8;
    VcmReg_Read(0xB0, &data);
    pVcmCusHandle->i2c_cfg.fmt = VCM_I2C_FMT;

    if((data & 0x80) == 0x00)
        *cur_pos = MotorPos;
    else
    {
        data = 0x00;
        VcmReg_Read(0x0A, &data);
        *cur_pos = ((data + 16384) & 0x0000FFFF) * 1023 / 32768;
    }
    CamOsMutexUnlock(&Lock);
    CamOsPrintf(COLOR_RED"[pCus_vcm_GetCurPos] *cur_pos %d\r\n"COLOR_NONE, *cur_pos);

    return SUCCESS;
}

static int pCus_vcm_GetMinMaxPos(ss_cus_vcm *pVcmCusHandle, u32 *min_pos, u32 *max_pos)
{
    *min_pos = 0;
    *max_pos = MAX_POS;
    return SUCCESS;
}

int cus_vcm_init_handle(ss_cus_vcm* drv_handle)
{
    ss_cus_vcm *pVcmCusHandle = drv_handle;

    if (!pVcmCusHandle) {
        VCM_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    VCM_EMSG("[%s]", __FUNCTION__);

    // i2c
    pVcmCusHandle->i2c_cfg.mode    = VCM_I2C_LEGACY;
    pVcmCusHandle->i2c_cfg.fmt     = VCM_I2C_FMT;
    pVcmCusHandle->i2c_cfg.speed   = VCM_I2C_SPEED;
    pVcmCusHandle->i2c_cfg.address = VCM_I2C_ADDR;

    pVcmCusHandle->pCus_vcm_PowerOn      = pCus_vcm_PowerOn;
    pVcmCusHandle->pCus_vcm_PowerOff     = pCus_vcm_PowerOff;
    pVcmCusHandle->pCus_vcm_Init         = pCus_vcm_Init;
    pVcmCusHandle->pCus_vcm_SetPos       = pCus_vcm_SetPos;
    pVcmCusHandle->pCus_vcm_GetCurPos    = pCus_vcm_GetCurPos;
    pVcmCusHandle->pCus_vcm_GetMinMaxPos = pCus_vcm_GetMinMaxPos;

#if 0
    pVcmCusHandle->pCus_vcm_SetEEPROMValue = pCus_vcm_SetEEPROMValue;
    pVcmCusHandle->pCus_vcm_GetEEPROMValue = pCus_vcm_GetEEPROMValue;
    pVcmCusHandle->pCus_vcm_GetMotionStatus = pCus_vcm_GetMotionStatus;
    pVcmCusHandle->pCus_vcm_SetPosition = pCus_vcm_SetPosition;
    pVcmCusHandle->pCus_vcm_GetPosition = pCus_vcm_GetPosition;
    pVcmCusHandle->pCus_vcm_SetI2CValue = pCus_vcm_SetI2CValue;
    pVcmCusHandle->pCus_vcm_GetI2CValue = pCus_vcm_GetI2CValue;
#endif
    pgVcmHandle = pVcmCusHandle;

    return SUCCESS;
}

VCM_DRV_ENTRY_IMPL_END(LC898249,cus_vcm_init_handle);


