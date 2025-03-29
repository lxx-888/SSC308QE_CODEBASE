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
#include "mi_common_datatype.h"
#include "mi_pcie_datatype.h"

#ifndef __MI_PCIE_H__
#define __MI_PCIE_H__
#define PCIE_MAJOR_VERSION  3
#define PCIE_SUB_VERSION    0
#define MACRO_TO_STR(macro) #macro
#define PCIE_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                                \
        {                                                                                                            \
            char *tmp =                                                                                              \
                sub_version / 100  ? "mi_pcie_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_pcie_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_pcie_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                     \
        })
#define MI_PCIE_API_VERSION PCIE_VERSION_STR(PCIE_MAJOR_VERSION, PCIE_SUB_VERSION)
#ifdef __cplusplus
extern "C"
{
#endif

    /*************************************************************************
     Function: MI_PCIE_DmaTask

      @u16ChipId:   IN

      @pstDmaTask: IN
                pstDmaTask->u32BlockCount    DMA task count
                pstDmaTask->bReadFromRmt     Is read from remote.
                pstDmaTask->stDmaBlocks      Task block arrays.
      Return:
          MI_SUCCESS if success.
    *************************************************************************/
    MI_S32 MI_PCIE_DmaTask(MI_PCIE_DEV devId, MI_PCIE_DmaTask_t *pstDmaTask);

    /*************************************************************************
     Function: MI_PCIE_CreateDevice

      @devId:   IN

      @pstPcieDevAttr: IN
                pstPcieDevAttr->enMode       RC or EP
      Return:
          MI_SUCCESS if success.
    *************************************************************************/
    MI_S32 MI_PCIE_CreateDevice(MI_PCIE_DEV devId, MI_PCIE_DevAttr_t *pstPcieDevAttr);

    /*************************************************************************
     Function: MI_PCIE_DestroyDevice

      @devId:   IN

      Return:
          MI_SUCCESS if success.
    *************************************************************************/
    MI_S32 MI_PCIE_DestroyDevice(MI_PCIE_DEV devId);

    /*************************************************************************
     Function: MI_PCIE_CreateChannel

      @devId:   IN

      @chnId:   IN

      @pstPcieChnAttr IN
                pstPcieChnAttr->u16TargetChipId        Target Chip Id.
                pstPcieChnAttr->u32TargetChnId         Target Chip's Pcie Channel id.

      Return:
          MI_SUCCESS if success.
    *************************************************************************/
    MI_S32 MI_PCIE_CreateChannel(MI_PCIE_DEV devId, MI_PCIE_CHN chnId, MI_PCIE_ChnAttr_t *pstPcieChnAttr);

    /*************************************************************************
     Function: MI_PCIE_DestroyChannel

      @devId:   IN

      @chnId:   IN

      Return:
          MI_SUCCESS if success.
    *************************************************************************/
    MI_S32 MI_PCIE_DestroyChannel(MI_PCIE_DEV devId, MI_PCIE_CHN chnId);

    /*************************************************************************
     Function: MI_PCIE_StartChannel

      @devId:   IN

      @chnId:   IN

      Return:
          MI_SUCCESS if success.
    *************************************************************************/
    MI_S32 MI_PCIE_StartChannel(MI_PCIE_DEV devId, MI_PCIE_CHN chnId);

    /*************************************************************************
     Function: MI_PCIE_StopChannel

      @devId:   IN

      @chnId:   IN

      Return:
          MI_SUCCESS if success.
    *************************************************************************/
    MI_S32 MI_PCIE_StopChannel(MI_PCIE_DEV devId, MI_PCIE_CHN chnId);

#ifdef __cplusplus
}
#endif

#endif
