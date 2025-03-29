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

#ifndef __MI_PCIE_DATATYPE__
#define __MI_PCIE_DATATYPE__

#include "mi_common_datatype.h"

#define MI_PCIE_DMA_BLOCK_MAX 8

typedef enum
{
    E_MI_PCIE_WORKMODE_RC,
    E_MI_PCIE_WORKMODE_EP
} MI_PCIE_WorkMode_e;

typedef struct MI_PCIE_DevAttr_s
{
    MI_PCIE_WorkMode_e enMode;
} MI_PCIE_DevAttr_t;

typedef struct MI_PCIE_DmaBlocks_s
{
    MI_PHY phyLocalAddr;
    MI_PHY phyRemoteAddr;
    MI_U32 u32BlkSize;
} MI_PCIE_DmaBlocks_t;

typedef struct MI_PCIE_DmaTask_s
{
    MI_U32              u32BlockCount;
    MI_BOOL             bReadFromRmt;
    MI_PCIE_DmaBlocks_t stDmaBlocks[MI_PCIE_DMA_BLOCK_MAX];
} MI_PCIE_DmaTask_t;

typedef struct MI_PCIE_ChnAttr_s
{
    MI_U16 u16TargetChipId;
    MI_U32 u32TargetChnId;
} MI_PCIE_ChnAttr_t;

#define MI_DEF_PCIE_ERR(err) MI_DEF_ERR(E_MI_MODULE_ID_PCIE, E_MI_ERR_LEVEL_ERROR, err)

/* invalid device ID */
#define MI_ERR_PCIE_INVALID_DEVID MI_DEF_PCIE_ERR(E_MI_ERR_INVALID_DEVID)
/* invalid channel ID */
#define MI_ERR_PCIE_INVALID_CHNID MI_DEF_PCIE_ERR(E_MI_ERR_INVALID_CHNID)
/* at lease one parameter is illegal, e.g, an illegal enumeration value  */
#define MI_ERR_PCIE_ILLEGAL_PARAM MI_DEF_PCIE_ERR(E_MI_ERR_ILLEGAL_PARAM)
/* channel exists */
#define MI_ERR_PCIE_EXIST MI_DEF_PCIE_ERR(E_MI_ERR_EXIST)
/*UN exist*/
#define MI_ERR_PCIE_UNEXIST MI_DEF_PCIE_ERR(E_MI_ERR_UNEXIST)
/* using a NULL point */
#define MI_ERR_PCIE_NULL_PTR MI_DEF_PCIE_ERR(E_MI_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configuring attribute */
#define MI_ERR_PCIE_NOT_CONFIG MI_DEF_PCIE_ERR(E_MI_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define MI_ERR_PCIE_NOT_SUPPORT MI_DEF_PCIE_ERR(E_MI_ERR_NOT_SUPPORT)
/* operation is not permitted, e.g, try to change static attribute */
#define MI_ERR_PCIE_NOT_PERM MI_DEF_PCIE_ERR(E_MI_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define MI_ERR_PCIE_NOMEM MI_DEF_PCIE_ERR(E_MI_ERR_NOMEM)
/* failure caused by malloc buffer */
#define MI_ERR_PCIE_NOBUF MI_DEF_PCIE_ERR(E_MI_ERR_NOBUF)
/* no data in buffer */
#define MI_ERR_PCIE_BUF_EMPTY MI_DEF_PCIE_ERR(E_MI_ERR_BUF_EMPTY)
/* no buffer for new data */
#define MI_ERR_PCIE_BUF_FULL MI_DEF_PCIE_ERR(E_MI_ERR_BUF_FULL)
/* System is not ready,maybe not initialed or loaded.
 * Returning the error code when opening a device file failed.
 */
#define MI_ERR_PCIE_NOTREADY MI_DEF_PCIE_ERR(E_MI_ERR_SYS_NOTREADY)

/* bad address, e.g. used for copy_from_user & copy_to_user */
#define MI_ERR_PCIE_BADADDR MI_DEF_PCIE_ERR(E_MI_ERR_BADADDR)
/* resource is busy, e.g. destroy a PCIE channel without unregistering it */
#define MI_ERR_PCIE_BUSY MI_DEF_PCIE_ERR(E_MI_ERR_BUSY)

/* channel not start*/
#define MI_ERR_PCIE_CHN_NOT_STARTED MI_DEF_PCIE_ERR(E_MI_ERR_CHN_NOT_STARTED)
/* channel not stop*/
#define MI_ERR_PCIE_CHN_NOT_STOPPED MI_DEF_PCIE_ERR(E_MI_ERR_CHN_NOT_STOPED)
/* to be removed later */
#define MI_ERR_PCIE_UNDEFINED MI_DEF_PCIE_ERR(E_MI_ERR_FAILED)

#endif //__MI_PCIE_DATATYPE__
