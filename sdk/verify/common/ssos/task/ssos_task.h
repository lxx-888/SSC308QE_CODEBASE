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
#ifndef __SSOS_TASK__
#define __SSOS_TASK__

#include "ssos_thread.h"

/*
 * Thread event manager from SigmaStar.
 */

#define SSOS_TASK_GET_PART_BUFFER(__h, __type, __member, __value)                               \
    do                                                                                          \
    {                                                                                           \
        type __tmp;                                                                             \
        SSOS_TASK_GetPartBufData(__h, &__tmp, &(__tmp.member), sizeof(__tmp.member), &__value); \
    } while (0);
#define SSOS_TASK_SET_PART_BUFFER(__h, __type, __member, __value)                               \
    do                                                                                          \
    {                                                                                           \
        type tmp;                                                                               \
        SSOS_TASK_SetPartBufData(__h, &__tmp, &(__tmp.member), sizeof(__tmp.member), &__value); \
    } while (0);

typedef struct SSOS_TASK_UserData_s
{
    /*
     * It is because that 'userData' and 'size' only describe for the data struct,
     * and user wish count the size that do not related to "userData" which point to.
     * So the 'realSize' is for user to count the real memory size of system memory in some usecase.
     */
    void *       data;
    unsigned int size;
    unsigned int realSize;
} SSOS_TASK_UserData_t;
typedef struct SSOS_TASK_Buffer_s
{
    /*
     * Declare for 'ssos_task' internal bufffer.
     */
    void *       buf;
    unsigned int size;
} SSOS_TASK_Buffer_t;

typedef void *(*SSOS_TASK_Signal_t)(struct SSOS_TASK_Buffer_s *, struct SSOS_TASK_UserData_s *);
typedef void *(*SSOS_TASK_Monitor_t)(struct SSOS_TASK_Buffer_s *);

typedef struct SSOS_TASK_Attr_s
{
    long                monitorCycleSec;  /* Monitor wait timeout sec. */
    long                monitorCycleNsec; /* Monitor wait timeout nsec. */
    SSOS_TASK_Monitor_t doMonitor;        /* Callback monitor main function. */
    SSOS_TASK_Signal_t  doSignal;         /* Do event. */
    SSOS_TASK_Signal_t  doSignalDrop;     /* Do drop event if set 'doSignalDrop' or 'isDropEvent'. */
    SSOS_THREAD_Attr_t  threadAttr;       /* Thread basic attribution. */

    struct SSOS_TASK_Buffer_s inBuf; /* Use for internal buffer. */

    unsigned char isDropData;
    unsigned char isDropEvent;
    unsigned int  maxData;
    unsigned int  maxEvent;

    unsigned char isResetTimer; /* Reset timer after get signal. */
} SSOS_TASK_Attr_t;

void *SSOS_TASK_Open(struct SSOS_TASK_Attr_s *attr);
int   SSOS_TASK_Close(void *handle);
int   SSOS_TASK_StartMonitor(void *handle);
int   SSOS_TASK_OneShot(void *handle);
int   SSOS_TASK_ConfigTimer(void *handle, long timeOutSec, long timeOutNsec,
                            unsigned char isResetTimer); // if timeOut is 0, use default setting from SSOS_TASK_Open
int   SSOS_TASK_Stop(void *handle);
int   SSOS_TASK_Send(void *handle, struct SSOS_TASK_UserData_s *userData);
int   SSOS_TASK_SetBuffer(void *handle, void *buf);
int   SSOS_TASK_GetBuffer(void *handle, void *buf);
int   SSOS_TASK_SetPartBufData(void *handle, void *head, void *part, unsigned int size, void *inBuf);
int   SSOS_TASK_GetPartBufData(void *handle, void *head, void *part, unsigned int size, void *outBuf);

#endif
