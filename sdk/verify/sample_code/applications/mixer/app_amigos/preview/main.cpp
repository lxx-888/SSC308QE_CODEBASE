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
#include "amigos.h"
#ifdef CONFIG_MOD_CMD
#include "ss_cmd_base.h"
#include "amigos_command.h"
INIT_CMD_BASE(amicmd);
#else
#include "amigos_module_init.h"
#endif

static void preload_module()
{
#ifdef CONFIG_MOD_CMD
    MOD_SETUP_IN(base);
    MOD_SETUP_IN(amigos);
    MOD_SETUP_IN(common);
#ifdef CONFIG_DB_UT
    MOD_SETUP_IN(amidb_ut);
#endif
#endif
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
    MOD_SETUP(pcie_ring_internal);
    AMIGOS_SETUP("PCIE", Pcie, MiBase);
#endif
#ifdef INTERFACE_VDEC
    AMIGOS_SETUP("VDEC", Vdec, MiBase);
#endif
#ifdef INTERFACE_DISP
    AMIGOS_SETUP("DISP", Disp, MiBase);
    AMIGOS_SETUP("WBC", Wbc, MiBase);
#endif
    AMIGOS_SETUP("RTSP", Rtsp);
    AMIGOS_MODULE_SETUP(Exp);
#ifdef INTERFACE_VENC
    AMIGOS_SETUP("VENC", Venc, MiBase);
#endif
#ifdef INTERFACE_ISP
    AMIGOS_SETUP("ISP", Isp, MiBase);
    AMIGOS_MODULE_SETUP(Gae);
    AMIGOS_MODULE_SETUP(Sae);
#endif
#ifdef INTERFACE_VIF
    AMIGOS_SETUP("VIF", Vif, MiBase);
#endif
#ifdef INTERFACE_SCL
    AMIGOS_SETUP("SCL", Scl, MiBase);
    AMIGOS_SETUP("SCL_STRETCH", SclStretch, MiBase);
#endif
#ifdef INTERFACE_IQSERVER
    AMIGOS_SETUP("IQ", Iq);
#endif
    AMIGOS_SETUP("FILE", File);
    AMIGOS_MODULE_SETUP(Timer);
    AMIGOS_MODULE_SETUP(EnvMonitor);
    AMIGOS_SETUP("SYNC", Sync);
    AMIGOS_SETUP("SWITCH", Switch);
    AMIGOS_SETUP("TICK", Tick);
    AMIGOS_SETUP("PASS", Pass);
    AMIGOS_SETUP("EMPTY", Empty);
#ifdef INTERFACE_VDISP
    AMIGOS_SETUP("VDISP", Vdisp, MiBase);
#endif
#ifdef INTERFACE_AI
    AMIGOS_SETUP("AI", Ai, MiBase);
    AMIGOS_SETUP("AEC", Aec);
#endif
#ifdef INTERFACE_AO
    AMIGOS_SETUP("AO", Ao, MiBase);
#endif
#if (defined(INTERFACE_AI ) || defined(INTERFACE_AO))
    AMIGOS_SETUP("AUDIOALGO", AudioAlgo);
    AMIGOS_MODULE_SETUP(AudioDecode);
    AMIGOS_MODULE_SETUP(AudioEncode);
#endif
#ifdef INTERFACE_SENSOR
    AMIGOS_SETUP("SNR", Snr, MiBase);
#endif
#ifdef INTERFACE_SYS
    AMIGOS_SETUP("OSD", Osd);
    AMIGOS_SETUP("UAC", Uac);
    AMIGOS_SETUP("UVC", Uvc);
    AMIGOS_SETUP("SLOT", Slot);
    AMIGOS_SETUP("POOL", Pool);
    AMIGOS_MODULE_SETUP(Pares);
#endif
#ifdef CUSTOMER_SNR9931
    AMIGOS_SETUP("SNR9931", Snr9931, MiBase);
#endif
#ifdef INTERFACE_LDC
    AMIGOS_SETUP("LDC", Ldc, MiBase);
#endif
#ifdef INTERFACE_NIR
    AMIGOS_SETUP("NIR", Nir, MiBase);
#endif
#ifdef INTERFACE_JPD
    AMIGOS_SETUP("JPD", Jpd, MiBase);
#endif
#ifdef INTERFACE_VDF
    AMIGOS_SETUP("VDF", Vdf, MiBase);
#endif
#ifdef INTERFACE_IVE
    AMIGOS_SETUP("IVE", Ive, MiBase);
#endif
#ifdef INTERFACE_RGN
    AMIGOS_SETUP("RGN", Rgn, MiBase);
#endif
#ifdef INTERFACE_SHADOW
#ifdef INTERFACE_IPU
    AMIGOS_SETUP("HSEG", Hseg, MiBase);
#endif
#endif
#ifdef INTERFACE_IPU
    AMIGOS_SETUP("DET", Det, MiBase);
#endif
//#ifdef GLES_LDC_ENABLE
    //AMIGOS_SETUP("GPU", Gpu);
//#endif
//#ifdef DRM_ENABLE
    //AMIGOS_SETUP("DRM", Drm);
//#endif
#ifdef INTERFACE_HDMIRX
    AMIGOS_SETUP("HDMIRX", Hdmirx);
#endif
#ifdef INTERFACE_HVP
    AMIGOS_SETUP("HVP", Hvp);
    AMIGOS_SETUP("SIGNAL_MONITOR", SignalMonitor);
#endif
}

int main(int argc, char **argv)
{
    preload_module();
#ifdef CONFIG_MOD_CMD
    if (strlen(argv[0]) >= strlen("amicmd")
        && !strncmp(argv[0] + strlen(argv[0]) - strlen("amicmd"), "amicmd", strlen("amicmd")))
    {
        return setup_ui(argc, argv);
    }
#endif
    return amigos_setup_ui(argc, argv);
}
