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

#ifndef __SS_DATATYPE_H
#define __SS_DATATYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
    typedef unsigned char BOOL;
#endif

#define ERR_LOG_TITLE  "MCU_ERR_LOG "
#define DEBUG_LOG_TILE "MCU_INF_LOG "

#define DEBUG_LOG_LEVEL DEBUG_LOG_ERR

#define DEBUG_LOG_EMERG   0
#define DEBUG_LOG_ALERTRT 1
#define DEBUG_LOG_CRIT    2
#define DEBUG_LOG_ERR     3
#define DEBUG_LOG_WARNING 4
#define DEBUG_LOG_NOTICE  5
#define DEBUG_LOG_INFO    6
#define DEBUG_LOG_DEBUG   7

#if DEBUG_LOG_LEVEL >= DEBUG_LOG_INFO
#define DEBUG_INFO(fmt, ...)                          \
    do                                                \
    {                                                 \
        printf(DEBUG_LOG_TILE "" fmt, ##__VA_ARGS__); \
    } while (0);
#else
#define DEBUG_INFO(fmt, ...)
#endif

#if DEBUG_LOG_LEVEL >= DEBUG_LOG_ERR
#define DEBUG_ERROR(fmt, ...)                        \
    do                                               \
    {                                                \
        printf(ERR_LOG_TITLE "" fmt, ##__VA_ARGS__); \
    } while (0);
#else
#define DEBUG_ERROR(fmt, ...)
#endif

#define SS_TASK_CMD_PROTRCOL 14
    //________________________________________________
    // title				byte				mean
    //________________________________________________
    // Header				4				0x5A5A5A5A
    // CmdType				1				0:REQ 1:ACK  SS_CMD_TYPE_e
    // Cmd					1				current cmd  SS_TASK_e
    // Accept				1				in ack 0:accept 1:reject
    // Status				1				soc ststus   SS_STATE_e
    // User					2				user define
    // Tail					4				0xA5A5A5A5
    //________________________________________________
    // Total				14				SS_TASK_CMD_PROTRCOL
    //________________________________________________

    typedef enum
    {
        E_TASK_NONE,        // there is no task to do                     (mcu to soc)
        E_TASK_PHOTO,       // take photo                                 (mcu to soc)
        E_TASK_START_REC,   // start recording pictures                   (mcu to soc)
        E_TASK_STOP_REC,    // stop recording pictures                    (mcu to soc)
        E_TASK_TRANS,       // wifi ftp transmit                          (mcu to soc)
        E_TASK_STOP_TRANS,  // stop wifi ftp                              (mcu to soc)
        E_TASK_POWEROFF,    // power off when mcu check no other task     (mcu to soc)
        E_TASK_POWEROFF_OK, // mcu can pulldown soc power                 (soc to mcu)
        E_TASK_REQUEST,     // soc requests a task from mcu               (soc to mcu)
        E_TASK_DONE,        // soc notify to mcu when task done           (soc to mcu)
        E_TASK_HEARTBEAT,   // heart beat to notify alive state           (soc to mcu)

        E_TASK_ERR
    } SS_TASK_e;

    typedef enum
    {
        E_STATE_RESUMING = 0,
        E_STATE_IDEL,
        E_STATE_CAP_PIC,
        E_STATE_REC,
        E_STATE_TRANS,
        E_STATE_SUSPENDING,
        E_STATE_SUSPENDED,

        E_STATE_UNKOWN
    } SS_STATE_e;

    typedef enum
    {
        E_CMD_TYPE_REQ,
        E_CMD_TYPE_ACK,

        E_CMD_TYPE_ERR
    } SS_CMD_TYPE_e;

#ifdef __cplusplus
}
#endif
#endif
