/*
 * cam_os_condition.h- Sigmastar
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

#ifndef __CAM_OS_CONDITION_H__
#define __CAM_OS_CONDITION_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cam_os_wrapper.h"

typedef void *CamOsCondition_t;
typedef void *CamOsConditionToken_t;

void CamOsConditionInit(CamOsCondition_t *ptCondition);
void CamOsConditionDeinit(CamOsCondition_t *ptCondition);
void CamOsConditionWakeUpAll(CamOsCondition_t *ptCondition);

CamOsConditionToken_t _CamOsConditionTokenAlloc(CamOsCondition_t *ptCondition);
void                  _CamOsConditionTokenWait(CamOsConditionToken_t tToken);
s32                   _CamOsConditionTokenTimedWait(CamOsConditionToken_t tToken, u32 nMsec);
void                  _CamOsConditionTokenFree(CamOsCondition_t *ptCondition, CamOsConditionToken_t tToken);

#define CamOsConditionWait(ptCondition, condition)           \
    (                                                        \
        {                                                    \
            CamOsRet_e            __eRet = CAM_OS_OK;        \
            CamOsConditionToken_t tToken;                    \
            tToken = _CamOsConditionTokenAlloc(ptCondition); \
            while (!(condition))                             \
                _CamOsConditionTokenWait(tToken);            \
            _CamOsConditionTokenFree(ptCondition, tToken);   \
            __eRet;                                          \
        })

#define __CamOsConditionTimedWait(ptCondition, condition, timeout_ms)              \
    (                                                                              \
        {                                                                          \
            unsigned long         __ret         = timeout_ms;                      \
            unsigned long long    __target_time = CamOsGetTimeInMs() + timeout_ms; \
            CamOsConditionToken_t tToken;                                          \
            tToken = _CamOsConditionTokenAlloc(ptCondition);                       \
            while (!(condition))                                                   \
            {                                                                      \
                if (0 != _CamOsConditionTokenTimedWait(tToken, __ret))             \
                {                                                                  \
                    __ret = (condition);                                           \
                    break;                                                         \
                }                                                                  \
                __ret = (unsigned long)(__target_time - CamOsGetTimeInMs());       \
                if (__ret > timeout_ms)                                            \
                {                                                                  \
                    __ret = (condition);                                           \
                    break;                                                         \
                }                                                                  \
            }                                                                      \
            _CamOsConditionTokenFree(ptCondition, tToken);                         \
            __ret;                                                                 \
        })

#define CamOsConditionTimedWait(ptCondition, condition, nMsec)                            \
    (                                                                                     \
        {                                                                                 \
            CamOsRet_e __eRet = CAM_OS_OK;                                                \
            if (!__CamOsConditionTimedWait(ptCondition, condition, (nMsec & 0xFFFFFFFF))) \
            {                                                                             \
                __eRet = CAM_OS_TIMEOUT;                                                  \
            }                                                                             \
            __eRet;                                                                       \
        })

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_CONDITION_H__
