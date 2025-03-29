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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include "mi_sys.h"
#include "st_common_mailbox.h"
#include "st_common.h"
#include "st_common_disp.h"
#include "mi_disp_datatype.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_check_picture.h"
// #include "st_sample_run_pmrtos.h"
#include "st_common_rtsp_video.h"




#define MI_VENC_WIDTH 3840
#define MI_VENC_HEIGHT 2160
#define MI_CLICMD_PRELOAD 0
#define IOCTL_PRELOAD_DATA_L2R _IOW('i', MI_CLICMD_PRELOAD, char *)
#define DEVICE_FILE "/dev/mi_cli"

#define DEBUG (1)

#define CHECK_L2R_RESULT(_func_, _ret_)      \
    {                                        \
        _ret_ = _func_;                      \
        if (_ret_ < MI_SUCCESS)              \
        {                                    \
            perror("IOCTL_DATA_L2R failed"); \
            close(fd);                       \
            return -1;                       \
        }                                    \
    }

#define MI_CLI_IOCTL(__fd, __cmd, __p)                                                       \
    ({                                                                                       \
        struct                                                                               \
        {                                                                                    \
            unsigned short     __socid;                                                      \
            int                __len;                                                        \
            unsigned long long __ptr;                                                        \
        } __tr     = {0, _IOC_SIZE(__cmd), (unsigned long)__p};                              \
        int __rval = ioctl(__fd, __cmd, &__tr);                                              \
        if (__rval == -1)                                                                    \
        {                                                                                    \
            printf("failed to ioctl 0x%08lx!(%s)\n", (unsigned long)__cmd, strerror(errno)); \
        }                                                                                    \
        __rval;                                                                              \
    })



struct picture_demo_info
{
    /* Image_Info virt/phy addr */
    void            *pImageInfoVirtAddrForNonPM;
    MI_PHY           pImageInfoPhyAddrForNonPM;
    bdma_img_desc_t  ImageInfo;
    // MI_U16          Bdma_Direct;

    /* Image virt/phy addr */
    MI_PHY PicturePhyAddrForNonPm;
    void  *pPictureVirtAddrForNonPm;

    MI_U32          u32PictureSizeFromFile;       // input file size
    FILE           *checkfile_fd;                 // input file fd
    pthread_mutex_t PictureAddrmutex;             // recive file mutex
    pthread_t       pReceivePicture_thread;       // image recive thread
    MI_U8           u8ReceivePicture_threadFlag;  // image recive thread stop flag
    MI_U8           u8SaveFileFlag;               // save file flag
    MI_U8           u8PanelDisplayFlag;           // show on panel flag
    MI_U8           u8CheckFlag;                  // image from file or CM4
    MI_U8           u8ReceivePicture_TimeoutFlag; // recive image timeout
    MI_U8           u8SendKillMeFlag;             // recive image timeout send kill me signal to CM4
    MI_U8           u8rtsp_flag;                   // enter str or not
    MI_U8           u8ImageNum;
    // MI_U8           bRtspWillStop;
    char            aPmRtosPath[256];
    /* image feed module info*/
    MI_SYS_ChnPort_t stChnInput;
    /* image output info */
    ST_Common_OutputFile_Attr_t stOutFileAttr;

    MI_U8 u8SaveMipiFile;
    MI_U16 u16MipiDumpNum;
};

struct picture_demo_info gstPictureDemoInfo = {0};
struct sigaction         sigAction;

MI_S32 ST_Molloc_Picture_Space(MI_PHY *PhyAddr, void **VirtAddr, MI_U32 lengh)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_SYS_MMA_Alloc(0, NULL, lengh, PhyAddr);
    if (s32Ret != MI_SUCCESS)
    {
        printf("err:MI_SYS_MMA_Alloc\n");
        goto init_allocerr;
    }
    s32Ret |= MI_SYS_Mmap(*PhyAddr, lengh, VirtAddr, FALSE);
    if (s32Ret != MI_SUCCESS)
    {
        printf("err:MI_SYS_Mmap\n");
        goto init_maperr;
    }
    return MI_SUCCESS;
init_maperr:
    if (VirtAddr != NULL)
    {
        MI_SYS_Munmap(VirtAddr, lengh);
        VirtAddr = NULL;
    }
init_allocerr:
    if (PhyAddr != 0)
    {
        MI_SYS_MMA_Free(0, *PhyAddr);
        PhyAddr = 0;
    }
    return s32Ret;
}

MI_S32 ST_Release_Picture_Space(MI_PHY *PhyAddr, void **VirtAddr, MI_U32 lengh)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_SYS_Munmap(VirtAddr, lengh);
    s32Ret |= MI_SYS_MMA_Free(0, *PhyAddr);
    PhyAddr  = NULL;
    VirtAddr = 0;
    return s32Ret;
}

MI_S32 ST_StartRtspServer(void)
{
    ST_VideoStreamInfo_t stStreamInfo;

    // STCHECKRESULT(MI_SYS_Init(0));
    MI_VENC_DupChn(0, 0);

    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = 0;
    stStreamInfo.VencChn      = 0;
    stStreamInfo.u32Width     = MI_VENC_WIDTH;
    stStreamInfo.u32Height    = MI_VENC_HEIGHT;
    stStreamInfo.u32FrameRate = 30;
    stStreamInfo.rtspIndex    = 0;

    stStreamInfo.bSaveFile    = gstPictureDemoInfo.u8SaveMipiFile;
    stStreamInfo.u16SaveNum = gstPictureDemoInfo.u16MipiDumpNum;
    sprintf(stStreamInfo.u8SaveFilePath, "/tmp/cm4_Mipi");

    printf("stStreamInfo.bSaveFile %u,stStreamInfo.u32SaveNum %u u8SaveFilePath %s,\n",stStreamInfo.bSaveFile,stStreamInfo.u16SaveNum,stStreamInfo.u8SaveFilePath);
    // start rtsp
    STCHECKRESULT(ST_Common_RtspServerStartVideo(&stStreamInfo));

    while (gstPictureDemoInfo.u8rtsp_flag)
    {
        usleep(100 * 1000);
    }

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    return MI_SUCCESS;
}

MI_S32 ST_MICLIRtosQuit(void)
{
    MI_U8 ret = 0;
    int   fd  = open(DEVICE_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("Failed to open the device");
        return -1;
    }

    CHECK_L2R_RESULT(MI_CLI_IOCTL(fd, IOCTL_PRELOAD_DATA_L2R, "quit"), ret);

    close(fd);
    return ret;
}

MI_S32 ST_GetMipiSteam(void)
{
    MI_VENC_DupChn(0, 0);
    ST_Common_OutputFile_Attr_t stOutFileAttr;
    memset(&stOutFileAttr,0x0,sizeof(ST_Common_OutputFile_Attr_t));
    MI_U32                      u32VencDevId = 0;
    MI_U32                      u32VencChnId = 0;
    // MI_VENC_ModType_e           eType        = E_MI_VENC_MODTYPE_H265E;
    sprintf(stOutFileAttr.FilePath, "/tmp/cm4_Mipi");
    stOutFileAttr.stModuleInfo.eModId    = E_MI_MODULE_ID_VENC;
    stOutFileAttr.stModuleInfo.u32DevId  = u32VencDevId;
    stOutFileAttr.stModuleInfo.u32ChnId  = u32VencChnId;
    stOutFileAttr.u16DumpBuffNum=gstPictureDemoInfo.u16MipiDumpNum;
    stOutFileAttr.bThreadExit = FALSE;
    ST_Common_CheckResult(&stOutFileAttr);
    ST_Common_WaitDumpFinsh(&stOutFileAttr);
    return 0;
}

MI_S32 ST_Mailbox_ReceivePicture(struct picture_demo_info *PicInfo, MI_U16 picIndex)
{
    MI_S32                s32Ret                = MI_SUCCESS;
    SS_Mbx_Msg_t          stMsg                 = {};
    MI_SYS_MemcpyDirect_e ImageInfo_Bdma_Direct = 0;
    MI_SYS_MemcpyDirect_e Picture_Bdma_Direct   = 0;
    MI_PHY                BdmaInfoPhyAddr_Pm    = 0;
    bdma_img_desc_t      *image_info            = NULL;

    if (NULL == PicInfo->pImageInfoVirtAddrForNonPM || 0 == PicInfo->pImageInfoPhyAddrForNonPM)
    {
        STCHECKRESULT(ST_Molloc_Picture_Space(&PicInfo->pImageInfoPhyAddrForNonPM, &PicInfo->pImageInfoVirtAddrForNonPM,
                                              sizeof(bdma_img_desc_t)));
    }

    stMsg.eDirect          = E_SS_MBX_DIRECT_ARM_TO_CM4;
    stMsg.u8MsgClass       = E_MBX_CLASS_IMG_QUERY;
    stMsg.u8ParameterCount = 1;
    stMsg.u16Parameters[0] = picIndex;
    ST_Common_Mailbox_SendMsg(&stMsg, 1000);
    s32Ret = ST_Common_Mailbox_ReceiveMsg(E_MBX_CLASS_IMG_QUERY_RESP, &stMsg, 100000);
    if (s32Ret != MI_SUCCESS)
    {
        printf(" ST_Common_Mailbox_ReceiveMsg failed: ret: %d \n", s32Ret);
        return -1;
    }

    BdmaInfoPhyAddr_Pm = (stMsg.u16Parameters[0] + (stMsg.u16Parameters[1] << 16));
    // Determine the image type in which the data is stored
    if (E_MBX_MEMTYPE_IMI == stMsg.u16Parameters[3])
    {
        ImageInfo_Bdma_Direct = E_MI_SYS_MEMCPY_PM_SRAM_TO_DRAM;
        printf(" PM BDMA INFO location: PM IMI \n");
    }
    else if (E_MBX_MEMTYPE_PSRAM == stMsg.u16Parameters[3])
    {
        ImageInfo_Bdma_Direct = E_MI_SYS_MEMCPY_PM_PSRAM_TO_DRAM;
        printf(" PM BDMA INFO location: PM PSRAM \n");
    }
    else
    {
        printf(" unknow PM picture location \n");
        return -2;
    }

    s32Ret = MI_SYS_MemcpyPaEx(0, ImageInfo_Bdma_Direct, PicInfo->pImageInfoPhyAddrForNonPM, BdmaInfoPhyAddr_Pm,
                               sizeof(bdma_img_desc_t));
    if (s32Ret != MI_SUCCESS)
    {
        printf(" MI_SYS_MemcpyPaEx failed: ret: %d \n", s32Ret);
        return -1;
    }


#if DEBUG
    // show image info
    image_info = (bdma_img_desc_t *)PicInfo->pImageInfoVirtAddrForNonPM;
    printf("image_info->u32Addr=%u\r\n", image_info->u32Addr);
    printf("image_info->u32Size=%u\r\n", image_info->u32Size);
    printf("image_info->u8MemType=%u\r\n", image_info->u8MemType);
    printf("image_info->u16Width=%u\r\n", image_info->u16Width);
    printf("image_info->u16Height=%u\r\n", image_info->u16Height);
    printf("image_info->u8PixelFmt=%u\r\n", image_info->u8PixelFmt);
#endif

    memcpy(&PicInfo->ImageInfo,PicInfo->pImageInfoVirtAddrForNonPM,sizeof(bdma_img_desc_t));
    if (0 == PicInfo->PicturePhyAddrForNonPm || NULL == PicInfo->pPictureVirtAddrForNonPm)
    {
        STCHECKRESULT(ST_Molloc_Picture_Space(&PicInfo->PicturePhyAddrForNonPm, &PicInfo->pPictureVirtAddrForNonPm,
                                              image_info->u32Size));
    }

    // Determine the image type in which the data is stored
    if (E_MBX_MEMTYPE_IMI == image_info->u8MemType)
    {
        Picture_Bdma_Direct = E_MI_SYS_MEMCPY_PM_SRAM_TO_DRAM;
        printf(" PM BDMA INFO location: PM IMI \n");
    }
    else if (E_MBX_MEMTYPE_PSRAM == image_info->u8MemType)
    {
        Picture_Bdma_Direct = E_MI_SYS_MEMCPY_PM_PSRAM_TO_DRAM;
        printf(" PM BDMA INFO location: PM PSRAM \n");
    }
    else
    {
        printf(" unknow PM picture location \n");
        return -2;
    }

    printf(" MI_SYS_MemcpyPaEx  before \n");
    s32Ret = MI_SYS_MemcpyPaEx(0, Picture_Bdma_Direct, PicInfo->PicturePhyAddrForNonPm, image_info->u32Addr,
                               image_info->u32Size);
    printf(" MI_SYS_MemcpyPaEx  after \n");
    if (s32Ret != MI_SUCCESS)
    {
        printf("MI_SYS_MemcpyPaEx failed ,s32Ret: %d\n", s32Ret);
        return s32Ret;
    }
    return 0;
}

void *ST_NONPM_Receive_Picture_Thread(void *args)
{
    MI_S32           s32Ret     = MI_SUCCESS;
    MI_U32                    picIndex       = 0;
    MI_S32                    u32Ret         = 0;
    time_t           stTime     = 0;
    bdma_img_desc_t image_info = {0};
    char                      dump_name[256] = {0};
    FILE                     *dumpPictureOutfp;
    MI_U16                    u16DumpBuffNum    = 1;
    // MI_U32                    u32GetPictureTime = 0;
    MI_U32           u32Size    = 0;
    MI_U16           u16Width   = 0;
    MI_U16           u16Height  = 0;
    MI_SYS_BUF_HANDLE hHandle = 0;
    MI_SYS_BufConf_t  stBufConf;
    MI_SYS_BufInfo_t  stBufInfo;


    struct picture_demo_info *pdemo_info        = (struct picture_demo_info *)args;
    // u32GetPictureTime                           = ST_Common_GetMs();
    if (pdemo_info->u8CheckFlag != 0)
    {
        ST_Molloc_Picture_Space(&pdemo_info->PicturePhyAddrForNonPm,
                                            &pdemo_info->pPictureVirtAddrForNonPm, raw_640x360_len);
        memcpy(pdemo_info->pPictureVirtAddrForNonPm, raw_640x360, raw_640x360_len);
        u16Width  = 640;
        u16Height = 360;
        u32Size   = raw_640x360_len;
    }
    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
    stBufConf.eBufType           = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.u64TargetPts       = (MI_U64)time(&stTime);
    stBufConf.stFrameCfg.eFormat=RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP,E_MI_SYS_PIXEL_BAYERID_RG);
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stBufConf.stFrameCfg.eCompressMode  = E_MI_SYS_COMPRESS_MODE_NONE;
    stBufConf.stFrameCfg.u16Width       = u16Width;
    stBufConf.stFrameCfg.u16Height      = u16Height;

    while (pdemo_info->u8ReceivePicture_threadFlag && picIndex < pdemo_info->u8ImageNum)
    {
        pthread_mutex_lock(&pdemo_info->PictureAddrmutex);
        u32Ret = ST_Mailbox_ReceivePicture(pdemo_info, picIndex);
        pthread_mutex_unlock(&pdemo_info->PictureAddrmutex);

        if (MI_SUCCESS == u32Ret)
        {
            // u32GetPictureTime = ST_Common_GetMs();
            // show jpeg on panel
            if (gstPictureDemoInfo.u8SaveFileFlag == 1)
            {
                sprintf(dump_name, "/tmp/CM4_%d_%d_%d.raw", pdemo_info->ImageInfo.u16Width, pdemo_info->ImageInfo.u16Height, picIndex);
                printf("write file %s\n", dump_name);
                u16DumpBuffNum   = 1;
                dumpPictureOutfp = NULL;
                pthread_mutex_lock(&pdemo_info->PictureAddrmutex);
                ST_Common_WriteFile(dump_name, &dumpPictureOutfp, (MI_U8 *)pdemo_info->pPictureVirtAddrForNonPm,
                                    pdemo_info->ImageInfo.u32Size, &u16DumpBuffNum, true);
                pthread_mutex_unlock(&pdemo_info->PictureAddrmutex);
            }
            picIndex++;
            printf("receive picture %d\n", picIndex);
            gstPictureDemoInfo.u8ReceivePicture_TimeoutFlag = FALSE;
        }

        if (pdemo_info->u8PanelDisplayFlag)
        {
            // This is where the thread ends up when it exits
            image_info = pdemo_info->ImageInfo;
            stBufConf.stFrameCfg.u16Width   = image_info.u16Width?image_info.u16Width:640;
            stBufConf.stFrameCfg.u16Height  = image_info.u16Height?image_info.u16Height:360;
            u32Size    = image_info.u32Size?image_info.u32Size:stBufConf.stFrameCfg.u16Width*stBufConf.stFrameCfg.u16Height;
            printf("u16Width =%d.u16Height= %d,u32Size=%d\n",u16Width,u16Height,u32Size);
            if (MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&pdemo_info->stChnInput, &stBufConf, &stBufInfo, &hHandle, 0))
            {
                MI_SYS_MemcpyPa(0, stBufInfo.stFrameData.phyAddr[0], pdemo_info->PicturePhyAddrForNonPm, u32Size);
                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
                if (s32Ret != MI_SUCCESS)
                {
                    printf("%s:%d MI_SYS_ChnInputPortPutBuf ret:%d\n", __FUNCTION__, __LINE__, s32Ret);
                }
            }
            else
            {
                printf("%s:%d MI_SYS_ChnInputPortGetBuf failed\n", __FUNCTION__, __LINE__);
            }
        }
        usleep(80 * 1000);
    }
    ST_Common_WaitDumpFinsh(&pdemo_info->stOutFileAttr);
    printf("exit recive picture thread\r\n");
    return NULL;
}

static MI_S32 ST_NONPM_PipeInit()
{
    /************************************************
    step1 :init sys and mailbox
    *************************************************/
    STCHECKRESULT(MI_SYS_Init(0));
    STCHECKRESULT(ST_Common_Mailbox_Enable());
    STCHECKRESULT(ST_Common_set_NonPM_StateMachine(E_MBX_STATEMCHN_ALIVE));
    gstPictureDemoInfo.u8ReceivePicture_threadFlag = TRUE;
    // gstPictureDemoInfo.u8ReceivePicture_TimeoutFlag = FALSE;

    if (gstPictureDemoInfo.u8PanelDisplayFlag == 1 || gstPictureDemoInfo.u8CheckFlag == 1)
    {
        enum en_mailbox_statemachine pm_state=E_MBX_STATEMCHN_DEAD;
        SS_Mbx_Msg_t              stMsg             = {};
        pm_state=ST_Common_CkeckPM_State();
        if(pm_state == E_MBX_STATEMCHN_DEAD)
        {
            ST_MICLIRtosQuit();
            STCHECKRESULT(RunPmrtos("/customer/sample_code/bin/pm_rtos"));
            ST_Common_Mailbox_KillMe();
        }

        if (MI_SUCCESS == ST_Common_Mailbox_ReceiveMsg(E_MBX_CLASS_RAW_IMG, &stMsg, 100000))
        {
            gstPictureDemoInfo.u8ImageNum = stMsg.u16Parameters[0];
            printf("u8ImageNum%u\r\n",  gstPictureDemoInfo.u8ImageNum);
            gstPictureDemoInfo.stOutFileAttr.u16DumpBuffNum=gstPictureDemoInfo.u8ImageNum;
        }else{
            printf("recive raw image num failed");
            return -1;
        }
        /************************************************
        step2 :init isp
        *************************************************/
        MI_U32 u32IspDevId  = 0;
        MI_U32 u32IspChnId  = 1;
        MI_U32 u32IspPortId = 0;

        MI_U32            u32SrcFrmrate;
        MI_U32            u32DstFrmrate;
        MI_SYS_BindType_e eBindType;
        MI_U32            u32BindParam;
        MI_SYS_ChnPort_t  stSrcChnPort;
        MI_SYS_ChnPort_t  stDstChnPort;

        MI_ISP_DevAttr_t      stIspDevAttr;
        MI_ISP_ChannelAttr_t  stIspChnAttr;
        MI_ISP_ChnParam_t     stIspChnParam;
        MasterEarlyInitParam_t *pstEarlyInitParam;
        MI_SYS_WindowRect_t   stIspInputCrop;
        MI_ISP_OutPortParam_t stIspOutPortParam;
        ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
        STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));
        ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
        pstEarlyInitParam = (MasterEarlyInitParam_t*) &stIspChnAttr.stIspCustIqParam.stVersion.u8Data[0];
        memset(pstEarlyInitParam,0x0,sizeof(MasterEarlyInitParam_t));
        stIspChnAttr.u32SensorBindId = MI_ISP_SNR_FLAG_3A_STATS_ONLY;
        pstEarlyInitParam->u16SnrEarlyAwbRGain = 1405;
        pstEarlyInitParam->u16SnrEarlyAwbGGain = 1024;
        pstEarlyInitParam->u16SnrEarlyAwbBGain = 2317;
        stIspChnAttr.stIspCustIqParam.stVersion.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
        stIspChnAttr.stIspCustIqParam.stVersion.u32Size = sizeof(MasterEarlyInitParam_t);
        ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam);
        MI_ISP_ApiCmdLoadBinFile(u32IspDevId, u32IspChnId, "/misc/isp_api0.bin", 1234);
        ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
        STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, u32IspPortId, &stIspOutPortParam));

        gstPictureDemoInfo.stChnInput.eModId    = E_MI_MODULE_ID_ISP;
        gstPictureDemoInfo.stChnInput.u32DevId  = u32IspDevId;
        gstPictureDemoInfo.stChnInput.u32ChnId  = u32IspChnId;
        gstPictureDemoInfo.stChnInput.u32PortId = u32IspPortId;

        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = u32IspDevId;
        stSrcChnPort.u32ChnId  = u32IspChnId;
        stSrcChnPort.u32PortId = u32IspPortId;

        // MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 4, 8);

        /************************************************
        step3 :init scl
        *************************************************/
        MI_U32                u32SclDevId  = 0;
        MI_U32                u32SclChnId  = 1;
        MI_U32                u32SclPortId = 0;
        MI_SCL_DevAttr_t      stSclDevAttr;
        MI_SCL_ChannelAttr_t  stSclChnAttr;
        MI_SCL_ChnParam_t     stSclChnParam;
        MI_SYS_WindowRect_t   stSclInputCrop;
        MI_SCL_OutPortParam_t stSclOutPortParam;

        ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
        stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL1;
        STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

        ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
        STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

        ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);
        stSclOutPortParam.stSCLOutputSize.u16Width  = 640;
        stSclOutPortParam.stSCLOutputSize.u16Height = 360;
        STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));

        /************************************************
        step4 :bind isp->scl
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = u32IspDevId;
        stSrcChnPort.u32ChnId  = u32IspChnId;
        stSrcChnPort.u32PortId = u32IspPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = u32SclDevId;
        stDstChnPort.u32ChnId  = u32SclChnId;
        stDstChnPort.u32PortId = u32SclPortId;
        u32SrcFrmrate          = 30;
        u32DstFrmrate          = 30;
        eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));

        MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 4, 8);

        /************************************************
        step5 :init venc
        *************************************************/

        MI_U32                      u32VencDevId = 0;
        MI_U32                      u32VencChnId = 1;
        MI_VENC_ModType_e           eType        = E_MI_VENC_MODTYPE_H265E;
        MI_VENC_InitParam_t         stVencInitParam;
        MI_VENC_ChnAttr_t           stVencChnAttr;
        MI_VENC_InputSourceConfig_t stVencSourceCfg;

        ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
        STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));
        ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = 640;
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 360;
        STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

        /************************************************
        step6 :bind scl->venc
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId;
        stSrcChnPort.u32PortId = u32SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 30;
        u32DstFrmrate          = 30;
        eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));

        /************************************************
        step7 :creat get stream thread
        *************************************************/
        sprintf(gstPictureDemoInfo.stOutFileAttr.FilePath, "/tmp/cm4_demo_num%u", gstPictureDemoInfo.u8ImageNum);
        gstPictureDemoInfo.stOutFileAttr.stModuleInfo.eModId    = E_MI_MODULE_ID_VENC;
        gstPictureDemoInfo.stOutFileAttr.stModuleInfo.u32DevId  = u32VencDevId;
        gstPictureDemoInfo.stOutFileAttr.stModuleInfo.u32ChnId  = u32VencChnId;
        gstPictureDemoInfo.stOutFileAttr.bThreadExit = FALSE;
        ST_Common_CheckResult(&gstPictureDemoInfo.stOutFileAttr);
        pthread_create(&gstPictureDemoInfo.pReceivePicture_thread, NULL, ST_NONPM_Receive_Picture_Thread,
                    &gstPictureDemoInfo);
    }

    return MI_SUCCESS;
}

static MI_S32 ST_NONPM_PipeUninit()
{

    if (gstPictureDemoInfo.u8PanelDisplayFlag == 1 || gstPictureDemoInfo.u8CheckFlag == 1)
    {
        MI_U32            u32IspDevId    = 0;
        MI_U32            u32IspChnId    = 1;
        MI_U32            u32IspPortId   = 0;
        MI_U32            u32SclDevId    = 0;
        MI_U32            u32SclChnId    = 1;
        MI_U32            u32SclPortId   = 0;

        MI_U32            u32VencDevId   = 0;
        MI_U32            u32VencChnId   = 1;
        MI_SYS_ChnPort_t stSrcChnPort;
        MI_SYS_ChnPort_t stDstChnPort;

        /************************************************
        step1 :unbind scl->venc
        *************************************************/

        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId;
        stSrcChnPort.u32PortId = u32SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step2 :deinit venc
        *************************************************/
        STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));
        STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

        /************************************************
        step3 :unbind isp->scl
        *************************************************/

        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = u32IspDevId;
        stSrcChnPort.u32ChnId  = u32IspChnId;
        stSrcChnPort.u32PortId = u32IspPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = u32SclDevId;
        stDstChnPort.u32ChnId  = u32SclChnId;
        stDstChnPort.u32PortId = u32SclPortId;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step4 :deinit scl
        *************************************************/
        STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId));
        STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));
        STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

        /************************************************
        step5 :deinit isp
        *************************************************/
        STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
        STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));
        STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));
    }
    /************************************************
    step5 :deinit sys and mailbox
    *************************************************/
    gstPictureDemoInfo.u8ReceivePicture_threadFlag = FALSE;
    pthread_join(gstPictureDemoInfo.pReceivePicture_thread, NULL);
    pthread_mutex_destroy(&gstPictureDemoInfo.PictureAddrmutex);
    return MI_SUCCESS;
}

void ST_NONPM_Demo_Usage(void)
{
    printf("Usage:./prog_cm4_nonPM_demo file x cm4 x rtsp x\n");
    printf("Usage: file x: x=0/1,save raw file to /tmp \n");
    printf("Usage: cm4 x: x=0/1,image which reciver from CM4\n");
    // printf("Usage: Conflict between the check and panel\n");
}

MI_S32 ST_NONPM_GetCmdlineParam(int argc, char **argv)
{
    gstPictureDemoInfo.u8SaveFileFlag     = 0x0; // default user input
    gstPictureDemoInfo.u8PanelDisplayFlag = 0x0;
    gstPictureDemoInfo.u8CheckFlag        = 0x0;

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "file") && argc > (i + 1))
        {
            gstPictureDemoInfo.u8SaveFileFlag = atoi(argv[i + 1]);
            printf("u8SaveFileFlag=%d\n", gstPictureDemoInfo.u8SaveFileFlag);
        }
        else if (0 == strcmp(argv[i], "cm4") && argc > (i + 1))
        {
            gstPictureDemoInfo.u8PanelDisplayFlag = atoi(argv[i + 1]);
            printf("u8PanelDisplayFlag=%d\n", gstPictureDemoInfo.u8PanelDisplayFlag);
        }
        else if (0 == strcmp(argv[i], "check") && argc > (i + 1))
        {
            gstPictureDemoInfo.u8CheckFlag = atoi(argv[i + 1]);
            printf("u8CheckFlag=%d\n", gstPictureDemoInfo.u8CheckFlag);
        }
        else if (0 == strcmp(argv[i], "rtsp") && argc > (i + 1))
        {
            gstPictureDemoInfo.u8rtsp_flag = atoi(argv[i + 1]);
            printf("u8rtsp_flag=%d\n", gstPictureDemoInfo.u8rtsp_flag);
        }
        else if ((0 == strcmp(argv[i], "pmrtos_path")) && (argc > (i + 1)))
        {
            strcpy(gstPictureDemoInfo.aPmRtosPath, argv[i]);
            printf("aPmRtosPath=%s.\n", gstPictureDemoInfo.aPmRtosPath);
        }
        else if ((0 == strcmp(argv[i], "mipi")) && (argc > (i + 1)))
        {
            gstPictureDemoInfo.u8SaveMipiFile=atoi(argv[i + 1]);

            if(argc > (i + 2))
            {
                gstPictureDemoInfo.u16MipiDumpNum=atoi(argv[i + 2]);
            }else
            {
                gstPictureDemoInfo.u16MipiDumpNum=60;
            }
            printf("u16MipiDumpNum=%d.\n", gstPictureDemoInfo.u16MipiDumpNum);
        }
    }
    gstPictureDemoInfo.u8SendKillMeFlag = !gstPictureDemoInfo.u8rtsp_flag;
    if (gstPictureDemoInfo.u8CheckFlag != 0 && gstPictureDemoInfo.u8PanelDisplayFlag != 0)
        return -1;
    pthread_mutex_init(&gstPictureDemoInfo.PictureAddrmutex, NULL);
    return MI_SUCCESS;
}
void ST_NONPM_HandleSig(MI_S32 signo)
{
    if (signo == SIGUSR1)
    {
        printf("catch SIGUSR1(10), send killme message and exit normally\n");
        // ST_Common_Mailbox_KillMe();
        gstPictureDemoInfo.u8ReceivePicture_threadFlag = FALSE;
        gstPictureDemoInfo.u8rtsp_flag=FALSE;
        ST_MICLIRtosQuit();
        ST_NONPM_PipeUninit();
        printf("send kill me signal to CM4\n");
        ST_Common_Mailbox_KillMe();
    }
    else if (signo == SIGUSR2)
    {
        gstPictureDemoInfo.u8SendKillMeFlag = FALSE;
        printf("catch SIGUSR2(12),Disable sending kill me signals\n");
    }
    return;
}

void ST_NONPM_RegisterSignalFunction(void)
{
    sigAction.sa_handler = ST_NONPM_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGUSR1, &sigAction, NULL); //-10
    sigaction(SIGUSR2, &sigAction, NULL); //-12
    printf("Signal processing function was registered successfully\n");
    return;
}


MI_S32 main(int argc, char **argv)
{

    if (ST_NONPM_GetCmdlineParam(argc, argv) != MI_SUCCESS)
    {
        ST_NONPM_Demo_Usage();
        return -1;
    }

    STCHECKRESULT(ST_NONPM_PipeInit());
    ST_NONPM_RegisterSignalFunction();
    if (gstPictureDemoInfo.u8rtsp_flag)
    {
        ST_StartRtspServer();
    }else if (gstPictureDemoInfo.u8SaveMipiFile)
    {
        ST_GetMipiSteam();
    }

    while (gstPictureDemoInfo.u8ReceivePicture_threadFlag)
    {
        usleep(1 * 1000 * 1000);
    }
    if(NULL!=gstPictureDemoInfo.pPictureVirtAddrForNonPm || NULL!=gstPictureDemoInfo.pImageInfoVirtAddrForNonPM)
    {
        ST_Release_Picture_Space(&gstPictureDemoInfo.PicturePhyAddrForNonPm,&gstPictureDemoInfo.pPictureVirtAddrForNonPm,gstPictureDemoInfo.ImageInfo.u32Size);
        ST_Release_Picture_Space(&gstPictureDemoInfo.pImageInfoPhyAddrForNonPM,&gstPictureDemoInfo.pImageInfoVirtAddrForNonPM,sizeof(bdma_img_desc_t));
    }

    memset(&gstPictureDemoInfo,0x0,sizeof(struct picture_demo_info));
    return 0;
}
