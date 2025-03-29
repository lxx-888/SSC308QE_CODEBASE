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
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <map>
#include "ss_cmd_base.h"
#include "amigos.h"
#include "amigos_env.h"
#include "amigos_command.h"
#include "amigos_database_io.hpp"

#define BLOCK_DISPLAY_WIDTH  36
#define BLOCK_DISPLAY_GAP    1
#define CURSOR_MOVE_LEFT(__x)  "\033[" << (__x) << 'D'
#define CURSOR_MOVE_RIGHT(__x) "\033[" << (__x) << 'C'
#define CURSOR_MOVE_UP(__x)    "\033[" << (__x) << 'A'
#define CURSOR_MOVE_DOWN(__x)  "\033[" << (__x) << 'B'

static Amigos *gp_amigosInstance = NULL;
static AmigosDatabaseFactory *gp_databaseFactory = NULL;
static int amigos_create_pipeline(vector<string> &in_strs)
{
    if (gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance was created!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_databaseFactory = new AmigosDatabaseFactory(in_strs[1]);
    assert(gp_databaseFactory);
    if (gp_databaseFactory->LoadFile() == -1)
    {
        delete gp_databaseFactory;
        gp_databaseFactory = NULL;
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Can not find the script file: "<< in_strs[1] << "!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance = new (std::nothrow)Amigos(ss_cmd_atoi(in_strs[2].c_str()), gp_databaseFactory);
    assert(gp_amigosInstance);
    return 0;
}
static int amigos_destroy_pipeline(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    delete gp_amigosInstance;
    gp_amigosInstance = NULL;
    gp_databaseFactory->UnloadFile();
    delete gp_databaseFactory;
    return 0;
}

static int amigos_start_pipeline(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->TakeOff();
    return 0;
}
static int amigos_stop_pipeline(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->DropDown();
    return 0;
}
static int amigos_init_all(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->InitAll();
    return 0;
}
static int amigos_deinit_all(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->DeinitAll();
    return 0;
}
static int amigos_bind_all(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->BindAll();
    return 0;
}
static int amigos_unbind_all(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->UnbindAll();
    return 0;
}
static int amigos_start_all(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->StartAll();
    return 0;
}
static int amigos_stop_all(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->StopAll();
    return 0;
}
static int amigos_init(vector<string> &in_strs)
{
    AmigosModuleBase *pCurClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }

    pCurClass->Init();
    return 0;
}
static int amigos_deinit(vector<string> &in_strs)
{
    AmigosModuleBase *pCurClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }

    pCurClass->Deinit();
    return 0;
}
static int amigos_prepare(vector<string> &in_strs)
{
    AmigosModuleBase *pCurClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }

    pCurClass->Prepare();
    return 0;
}
static int amigos_unprepare(vector<string> &in_strs)
{
    AmigosModuleBase *pCurClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }

    pCurClass->Unprepare();
    return 0;
}
static int amigos_start(vector<string> &in_strs)
{
    AmigosModuleBase *pCurClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }

    pCurClass->Start();
    return 0;
}
static int amigos_stop(vector<string> &in_strs)
{
    AmigosModuleBase *pCurClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }

    pCurClass->Stop();
    return 0;
}
static int amigos_bind(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    if (in_strs.size() == 2)
    {
        pDstClass->Bind();
        return 0;
    }
    pDstClass->BindBlock(ss_cmd_atoi(in_strs[2].c_str()));
    return 0;
}
static int amigos_unbind(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    if (in_strs.size() == 2)
    {
        pDstClass->UnBind();
        return 0;
    }
    pDstClass->UnBindBlock(ss_cmd_atoi(in_strs[2].c_str()));
    return 0;
}
static int amigos_start_in(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    if (in_strs.size() == 2)
    {
        pDstClass->StartIn();
        return 0;
    }
    unsigned int uintPort = (unsigned int)ss_cmd_atoi(in_strs[2].c_str());
    pDstClass->StartIn(uintPort);
    return 0;
}
static int amigos_stop_in(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    if (in_strs.size() == 2)
    {
        pDstClass->StopIn();
        return 0;
    }
    unsigned int uintPort = (unsigned int)ss_cmd_atoi(in_strs[2].c_str());
    pDstClass->StopIn(uintPort);
    return 0;
}
static int amigos_start_out(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    if (in_strs.size() == 2)
    {
        pDstClass->StartOut();
        return 0;
    }
    unsigned int uintPort = (unsigned int)ss_cmd_atoi(in_strs[2].c_str());
    pDstClass->StartOut(uintPort);
    return 0;
}
static int amigos_stop_out(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    if (in_strs.size() == 2)
    {
        pDstClass->StopOut();
        return 0;
    }
    unsigned int uintPort = (unsigned int)ss_cmd_atoi(in_strs[2].c_str());
    pDstClass->StopOut(uintPort);
    return 0;
}
static int amigos_link(vector<string> &in_strs)
{
    AmigosModuleBase *pPrevClass = NULL;
    AmigosModuleBase *pCurClass  = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pPrevClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pPrevClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[2]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[2].c_str());
        return -1;
    }
    unsigned int currInId  = ss_cmd_atoi(in_strs[3].c_str());
    AmigosSurfaceBase::ModPortInInfo stIn;
    pCurClass->GetSurface()->GetPortInInfo(currInId, stIn);
    unsigned int prevOutId = pPrevClass->GetSurface()->GetOutPortIdFromLoopId(stIn.stPrev.loopId);
    pCurClass->Link(currInId, prevOutId, pPrevClass);
    return 0;
}
static int amigos_unlink(vector<string> &in_strs)
{
    AmigosModuleBase *pCurClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pCurClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pCurClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int currInId  = ss_cmd_atoi(in_strs[2].c_str());
    pCurClass->Unlink(currInId);
    return 0;
}
static int amigos_refresh_db_file(vector<string> &in_strs)
{
    if (!gp_databaseFactory)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Factory did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_databaseFactory->UnloadFile();
    gp_databaseFactory->LoadFile();
    return 0;
}

static int amigos_refresh_db_all(vector<string> &in_strs)
{
    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->UnloadDbAll();
    gp_amigosInstance->GetIns()->LoadDbAll();
    return 0;
}
static int amigos_refresh_db(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    pDstClass->GetSurface()->UnloadDb();
    pDstClass->GetSurface()->LoadDb();
    return 0;
}
static int amigos_load_db(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    pDstClass->GetSurface()->LoadDb();
    return 0;
}
static int amigos_unload_db(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    pDstClass->GetSurface()->UnloadDb();
    return 0;
}
static int amigos_reset_stream(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    pDstClass->ResetStream(ss_cmd_atoi(in_strs[2].c_str()), ss_cmd_atoi(in_strs[3].c_str()), ss_cmd_atoi(in_strs[4].c_str()));
    return 0;
}
static int amigos_create_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->CreateDelayPass(inPortId);
}
static int amigos_destroy_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->DestroyDelayPass(inPortId);
}
static int amigos_init_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->InitDelayPass(inPortId);
}
static int amigos_deinit_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->DeinitDelayPass(inPortId);
}
static int amigos_bind_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->BindDelayPass(inPortId);
}
static int amigos_unbind_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->UnbindDelayPass(inPortId);
}
static int amigos_start_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->StartDelayPass(inPortId);
}
static int amigos_stop_delay_pass(std::vector<std::string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    unsigned int inPortId = ss_cmd_atoi(in_strs[2].c_str());
    return pDstClass->StopDelayPass(inPortId);
}
static int amigos_set_attr(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    return amigos_set_val(pDstClass->GetSurface()->GetDbIns(), in_strs, 2);
}
static int amigos_set_in_attr(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    AmigosSurfaceBase::ModPortInInfo stIn;
    bool ret = pDstClass->GetSurface()->GetPortInInfo(ss_cmd_atoi(in_strs[2].c_str()), stIn);
    if (!ret)
    {
        AMIGOS_ERR("Get surface in failed, key:%s port%d\n", in_strs[1].c_str(), ss_cmd_atoi(in_strs[2].c_str()));
        return -1;
    }
    return amigos_set_in_val(pDstClass->GetSurface()->GetDbIns(), stIn.curLoopId, in_strs, 3);
}
static int amigos_set_out_attr(vector<string> &in_strs)
{
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    AmigosSurfaceBase::ModPortOutInfo stOut;
    bool ret = pDstClass->GetSurface()->GetPortOutInfo(ss_cmd_atoi(in_strs[2].c_str()), stOut);
    if (!ret)
    {
        AMIGOS_ERR("Get surface out failed, key:%s port%d\n", in_strs[1].c_str(), ss_cmd_atoi(in_strs[2].c_str()));
        return -1;
    }
    return amigos_set_out_val(pDstClass->GetSurface()->GetDbIns(), stOut.curLoopId, in_strs, 3);
}
#ifdef CONFIG_MOD_CMD
static int amigos_access_obj(vector<string> &in_strs)
{
    std::string modName;
    AmigosModuleBase *pDstClass = NULL;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if(pDstClass == NULL)
    {
        AMIGOS_ERR("GetInstance failed, not found strKey [%s]\n", in_strs[1].c_str());
        return -1;
    }
    modName = pDstClass->GetSurface()->GetModInfo().modName;
    AmigosCommand *pDstClassCmd = AmigosCommand::GetObj(modName);
    if (!pDstClassCmd)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Mod ["<< modName << "] cmd did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    ss_cmd_base::set_ext_data(&pDstClassCmd->GetCmdData(), pDstClass);
    for (auto it = ss_cmd_base::get_data().begin(); it != ss_cmd_base::get_data().end(); ++it)
    {
        sslog.store_tab() << it->first;
        sslog.store_tab() << "/";
    }
    return 0;
}
#endif
static void _draw_input_port(unsigned int i, const std::vector<unsigned int> &ioPort)
{
    if (ioPort.size() > i)
    {
        sslog << std::setw(2) << std::left << ioPort[i];
    }
    else
    {
        sslog << std::setw(2) << std::left << "\033[" << 2 << 'C';
    }
}
static void _draw_output_port(unsigned int i, const std::vector<unsigned int> &ioPort)
{
    if (ioPort.size() > i)
    {
        sslog << std::setw(2) << std::right << ioPort[i];
    }
    else
    {
        sslog << std::setw(2) << std::right << "\033[" << 2 << 'C';
    }
}
static void _draw_block(unsigned int x, unsigned int y, unsigned int height, const char *str,
                      const std::vector<unsigned int> ioPort[2])
{
    unsigned int width;     // width of the block
    unsigned int i = 0;

    width = BLOCK_DISPLAY_WIDTH;    // calculate the width of the block based on the length of the string
    if (strlen(str) > width - 8)
    {
        sslog << "Disp Name: " << str << " exceed the limitation." << std::endl;
        return;
    }
    if (height == 0)
    {
        sslog << "Height is 0" << std::endl;
        return;
    }
    for (i = 0; i < y; i++)
    {
        sslog << std::endl;
    }
    i = 0;
    if (height > 1)
    {
        sslog << CURSOR_MOVE_RIGHT(x) << '|';
        _draw_input_port(i, ioPort[0]);
        sslog << std::string(BLOCK_DISPLAY_WIDTH - 6, '-');
        _draw_output_port(i, ioPort[1]);
        sslog << '|' << std::endl;
        i++;
    }

    // print the middle lines of the block
    for (; i < height / 2; i++)
    {
        sslog << CURSOR_MOVE_RIGHT(x) << '|';
        _draw_input_port(i, ioPort[0]);
        sslog << CURSOR_MOVE_RIGHT(BLOCK_DISPLAY_WIDTH - 6);
        _draw_output_port(i, ioPort[1]);
        sslog << '|' << std::endl;
    }
    unsigned int gap = (BLOCK_DISPLAY_WIDTH - strlen(str)) / 2 - 3;
    sslog << CURSOR_MOVE_RIGHT(x) << '|';
    _draw_input_port(i, ioPort[0]);
    sslog << CURSOR_MOVE_RIGHT(gap);
    sslog << str << CURSOR_MOVE_RIGHT(BLOCK_DISPLAY_WIDTH - gap - 6 - strlen(str));
    _draw_output_port(i, ioPort[1]);
    sslog << '|' << std::endl;
    i++;
    for (; i < height - 1; i++)
    {
        sslog << CURSOR_MOVE_RIGHT(x) << '|';
        _draw_input_port(i, ioPort[0]);
        sslog << CURSOR_MOVE_RIGHT(BLOCK_DISPLAY_WIDTH - 6);
        _draw_output_port(i, ioPort[1]);
        sslog << '|' << std::endl;
    }
    if (height > 2)
    {
        sslog << CURSOR_MOVE_RIGHT(x) << '|';
        _draw_input_port(i, ioPort[0]);
        sslog << std::string(BLOCK_DISPLAY_WIDTH - 6, '-');
        _draw_output_port(i, ioPort[1]);
        sslog << '|' << std::endl;
        i++;
    }
    sslog << CURSOR_MOVE_UP(y + height);
}
static int amigos_display_pipeline(vector<string> &in_strs)
{
    std::vector<AmigosInstance::AmigosBlockMap> vectBlockMap;
    unsigned int maxHeight = 0;

    if (!gp_amigosInstance)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Instance did not create!"<< std::endl << PRINT_COLOR_END;
        return -1;
    }
    gp_amigosInstance->GetIns()->GetMapAll(vectBlockMap);
#if 0
    for (auto it = vectBlockMap.begin(); it != vectBlockMap.end(); it++)
    {
        std::cout << it->modSectionName << " " << it->uintX << " " << it->uintY << " " << it->uintH << std::endl;
    }
#endif
    for (auto it = vectBlockMap.begin(); it != vectBlockMap.end(); it++)
    {
        _draw_block(it->uintX * (BLOCK_DISPLAY_WIDTH + BLOCK_DISPLAY_GAP + 1),
                    it->uintY, it->uintH, it->modSectionName.c_str(), it->ioPorts);
        maxHeight = maxHeight > it->uintY + it->uintH ? maxHeight : it->uintY + it->uintH;
    }
    sslog << CURSOR_MOVE_DOWN(maxHeight);
    return 0;
}
static void output_msg_from_env(AmigosEnv &env)
{
    std::map<std::string, std::string> mapEnv;
    env.Dump(mapEnv);
    for (const auto &item : mapEnv)
    {
        if (item.second != "")
        {
            sslog << item.first << ':' << item.second << std::endl;
        }
    }
}
static void output_msg_from_env(const char *modName)
{
    AmigosEnv env(modName);
    output_msg_from_env(env);
}
static int amigos_get_preview_windows(std::vector<std::string> &in_strs)
{
    output_msg_from_env("EXP_PREVIEW_WINDOWS");
    output_msg_from_env("RTSP_PREVIEW_WINDOWS");
    output_msg_from_env("UVC_PREVIEW_WINDOWS");
    output_msg_from_env("UAC_PREVIEW_WINDOWS");
    return 0;
}
static int amigos_dump_env(std::vector<std::string> &in_strs)
{
    AmigosEnv env(in_strs[1]);
    output_msg_from_env(env);
    return 0;
}
static int amigos_dump_in_env(std::vector<std::string> &in_strs)
{
    AmigosEnv env(in_strs[1]);
    output_msg_from_env(env.In(ss_cmd_atoi(in_strs[2].c_str())));
    return 0;
}
static int amigos_dump_out_env(std::vector<std::string> &in_strs)
{
    AmigosEnv env(in_strs[1]);
    output_msg_from_env(env.Out(ss_cmd_atoi(in_strs[2].c_str())));
    return 0;
}

static int amigos_dump_all_env(std::vector<std::string> &in_strs)
{
    sslog << "<Module " << in_strs[1] << " Env>"<< ':' << std::endl;
    AmigosEnv env(in_strs[1]);
    output_msg_from_env(env);
    AmigosModuleBase *pDstClass = AmigosModuleBase::GetModule(in_strs[1]);
    if (pDstClass)
    {
        auto inMap = pDstClass->GetSurface()->GetPortInMap();
        for (auto &it : inMap)
        {
            sslog << "<In" << it.first << ">:" << std::endl;
            output_msg_from_env(env.In(it.first));
        }
        auto outMap = pDstClass->GetSurface()->GetPortOutMap();
        for (auto &it : outMap)
        {
            sslog << "<Out" << it.first << ">:" << std::endl;
            output_msg_from_env(env.Out(it.first));
        }
    }
    return 0;
}
static int amigos_get_env(std::vector<std::string> &in_strs)
{
    if (!in_strs[1].size())
    {
        AMIGOS_ERR("Env GetIns failed, key is empty.\n");
        return -1;
    }
    AmigosEnv env(in_strs[1]);
    sslog << env[in_strs[2]] << std::endl;
    return 0;
}
static int amigos_get_in_env(std::vector<std::string> &in_strs)
{
    if (!in_strs[1].size())
    {
        AMIGOS_ERR("Env GetIns failed, key is empty.\n");
        return -1;
    }
    AmigosEnv env(in_strs[1]);
    unsigned int port = ss_cmd_atoi(in_strs[2].c_str());
    sslog << env.In(port)[in_strs[3]] << std::endl;
    return 0;
}
static int amigos_get_out_env(std::vector<std::string> &in_strs)
{
    if (!in_strs[1].size())
    {
        AMIGOS_ERR("Env GetIns failed, key is empty.\n");
        return -1;
    }
    AmigosEnv env(in_strs[1]);
    unsigned int port = ss_cmd_atoi(in_strs[2].c_str());
    sslog << env.Out(port)[in_strs[3]] << std::endl;
    return 0;
}
static int amigos_set_env(std::vector<std::string> &in_strs)
{
    if (!in_strs[1].size())
    {
        AMIGOS_ERR("Env GetIns failed, key is empty.\n");
        return -1;
    }
    AmigosEnv env(in_strs[1]);
    env[in_strs[2]] = in_strs[3];
    return 0;
}
static int amigos_set_in_env(std::vector<std::string> &in_strs)
{
    if (!in_strs[1].size())
    {
        AMIGOS_ERR("Env GetIns failed, key is empty.\n");
        return -1;
    }
    AmigosEnv env(in_strs[1]);
    unsigned int port = ss_cmd_atoi(in_strs[2].c_str());
    env.In(port)[in_strs[3]] = in_strs[4];
    return 0;
}
static int amigos_set_out_env(std::vector<std::string> &in_strs)
{
    if (!in_strs[1].size())
    {
        AMIGOS_ERR("Env GetIns failed, key is empty.\n");
        return -1;
    }
    AmigosEnv env(in_strs[1]);
    unsigned int port = ss_cmd_atoi(in_strs[2].c_str());
    env.Out(port)[in_strs[3]] = in_strs[4];
    return 0;
}
static int amigos_enable_shm_env(std::vector<std::string> &in_strs)
{
    bool isFather = ss_cmd_atoi(in_strs[1].c_str());
    return AmigosEnv::EnableShm(isFather);
}
static int amigos_disable_shm_env(std::vector<std::string> &in_strs)
{
    return AmigosEnv::DisableShm();
}
static int amigos_set_log_lv(std::vector<std::string> &in_strs)
{
    if (in_strs[1] == "trace")
    {
        amilog.Out.lv_mask(PRINT_LV_TRACE);
        return 0;
    }
    if (in_strs[1] == "debug")
    {
        amilog.Out.lv_mask(PRINT_LV_DEBUG);
        return 0;
    }
    if (in_strs[1] == "warn")
    {
        amilog.Out.lv_mask(PRINT_LV_WARN);
        return 0;
    }
    if (in_strs[1] == "error")
    {
        amilog.Out.lv_mask(PRINT_LV_ERROR);
        return 0;
    }
    return 0;
}
MOD_CMDS(amigos){
    ADD_CMD("amigos_create_pipeline", amigos_create_pipeline, 2);
    ADD_CMD_HELP("amigos_create_pipeline", "[scriptFilePath] [myChipId]", "Load ini and create block's object, myChipId default is 0.");
    ADD_CMD("amigos_destroy_pipeline", amigos_destroy_pipeline, 0);
    ADD_CMD_HELP("amigos_destroy_pipeline", "No argument", "Destroy all blocks object.");
    ADD_CMD("amigos_start_pipeline", amigos_start_pipeline, 0);
    ADD_CMD_HELP("amigos_start_pipeline", "No argument", "Start pipeline.");
    ADD_CMD("amigos_stop_pipeline", amigos_stop_pipeline, 0);
    ADD_CMD_HELP("amigos_stop_pipeline", "No argument", "Stop pipeline.");
    ADD_CMD("amigos_init_all", amigos_init_all, 0);
    ADD_CMD_HELP("amigos_init_all", "No argument", "Do init for all modules.");
    ADD_CMD("amigos_deinit_all", amigos_deinit_all, 0);
    ADD_CMD_HELP("amigos_deinit_all", "No argument", "Do deinit for all modules.");
    ADD_CMD("amigos_bind_all", amigos_bind_all, 0);
    ADD_CMD_HELP("amigos_bind_all", "No argument", "Do bind for all modules.");
    ADD_CMD("amigos_unbind_all", amigos_unbind_all, 0);
    ADD_CMD_HELP("amigos_unbind_all", "No argument", "Do unbind for all modules.");
    ADD_CMD("amigos_start_all", amigos_start_all, 0);
    ADD_CMD_HELP("amigos_start_all", "No argument", "Do start for all modules.");
    ADD_CMD("amigos_stop_all", amigos_stop_all, 0);
    ADD_CMD_HELP("amigos_stop_all", "No argument", "Do stop for all modules.");
    ADD_CMD("amigos_display_pipeline", amigos_display_pipeline, 0);
    ADD_CMD_HELP("amigos_display_pipeline", "No argument", "Display pipeline after create.");
    ADD_CMD("amigos_init", amigos_init, 1);
    ADD_CMD_HELP("amigos_init", "[strKey]", "init the mod");
    ADD_CMD("amigos_deinit", amigos_deinit, 1);
    ADD_CMD_HELP("amigos_deinit", "[strKey]", "deinit the mod");
    ADD_CMD("amigos_prepare", amigos_prepare, 1);
    ADD_CMD_HELP("amigos_prepare", "[strKey]", "prepare the mod");
    ADD_CMD("amigos_unprepare", amigos_unprepare, 1);
    ADD_CMD_HELP("amigos_unprepare", "[strKey]", "unprepare the mod");
    ADD_CMD("amigos_start", amigos_start, 1);
    ADD_CMD_HELP("amigos_start", "[strKey]", "start the mod");
    ADD_CMD("amigos_stop", amigos_stop, 1);
    ADD_CMD_HELP("amigos_stop", "[strKey]", "stop the mod");
    ADD_CMD("amigos_link", amigos_link, 3);
    ADD_CMD_HELP("amigos_link", "[prev key] [cur key] [cur input port]", "Link current module and previous from input port.");
    ADD_CMD("amigos_unlink", amigos_unlink, 2);
    ADD_CMD_HELP("amigos_unlink", "[cur key] [cur input port]", "Unlink current module and previous from input port.");
    ADD_CMD_VAR_ARG("amigos_bind", amigos_bind, 1);
    ADD_CMD_HELP("amigos_bind", "[dstKey] ...",
                 "Bind the dstKey's mod and its prev mod, if port id don't mention, which means it bind all input ports");
    ADD_CMD_VAR_ARG("amigos_unbind", amigos_unbind, 1);
    ADD_CMD_HELP("amigos_unbind", "[dstKey] ...",
                 "Unbind the dstKey's mod and its prev mod, if port id don't mention, which means it unbind all input ports");
    ADD_CMD_VAR_ARG("amigos_start_in", amigos_start_in, 1);
    ADD_CMD_HELP("amigos_start_in", "[strKey] ...",
                 "Start amigos modules's input port, if port id don't mention, which means start all ports.",
                 "[strKey]: Module section name", "[port]: Port id");
    ADD_CMD_VAR_ARG("amigos_stop_in", amigos_stop_in, 1);
    ADD_CMD_HELP("amigos_stop_in", "[strKey] ...",
                 "Stop amigos modules's input port, if port don't mention, which means stop all ports.",
                 "[strKey]: Module section name", "[port]: Port id");
    ADD_CMD_VAR_ARG("amigos_start_out", amigos_start_out, 1);
    ADD_CMD_HELP("amigos_start_out", "[strKey] ...",
                 "Start amigos modules's output port, if port don't mention, which means start all ports.",
                 "[strKey]: Module section name", "[port]: Port id");
    ADD_CMD_VAR_ARG("amigos_stop_out", amigos_stop_out, 1);
    ADD_CMD_HELP("amigos_stop_out", "[strKey] ...",
                 "Stop amigos modules's output port, if port don't mention, which means stop all ports.",
                 "[strKey]: Module section name", "[port]: Port id");
    ADD_CMD("amigos_refresh_db_file", amigos_refresh_db_file, 0);
    ADD_CMD_HELP("amigos_refresh_db_file", "No argument", "Unload and load module's database from target file.");
    ADD_CMD("amigos_refresh_db_all", amigos_refresh_db_all, 0);
    ADD_CMD_HELP("amigos_refresh_db_all", "No argument", "Unload and load module's surface from database.");
    ADD_CMD("amigos_refresh_db", amigos_refresh_db, 1);
    ADD_CMD_HELP("amigos_refresh_db", "[strKey]", "Unload and load module's database.");
    ADD_CMD("amigos_load_db", amigos_load_db, 1);
    ADD_CMD_HELP("amigos_load_db", "[strKey]", "Load module's database.");
    ADD_CMD("amigos_unload_db", amigos_unload_db, 1);
    ADD_CMD_HELP("amigos_unload_db", "[strKey]", "Unload module's database.");
    ADD_CMD("amigos_reset_stream", amigos_reset_stream, 4);
    ADD_CMD_HELP("amigos_reset_stream", "[cur input port] [width] [height]", "Reset stream start from currnet module's input port by traverse.");
    ADD_CMD("amigos_create_delay_pass", amigos_create_delay_pass, 2);
    ADD_CMD_HELP("amigos_create_delay_pass", "[strKey] [port]", "Create delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD("amigos_destroy_delay_pass", amigos_destroy_delay_pass, 2);
    ADD_CMD_HELP("amigos_destroy_delay_pass", "[strKey] [port]", "Destroy delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD("amigos_init_delay_pass", amigos_init_delay_pass, 2);
    ADD_CMD_HELP("amigos_init_delay_pass", "[strKey] [port]", "Initialize delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD("amigos_deinit_delay_pass", amigos_deinit_delay_pass, 2);
    ADD_CMD_HELP("amigos_deinit_delay_pass", "[strKey] [port]", "Deinitialize delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD("amigos_bind_delay_pass", amigos_bind_delay_pass, 2);
    ADD_CMD_HELP("amigos_bind_delay_pass", "[strKey] [port]", "Bind delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD("amigos_unbind_delay_pass", amigos_unbind_delay_pass, 2);
    ADD_CMD_HELP("amigos_unbind_delay_pass", "[strKey] [port]", "Unbind delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD("amigos_start_delay_pass", amigos_start_delay_pass, 2);
    ADD_CMD_HELP("amigos_start_delay_pass", "[strKey] [port]", "Start delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD("amigos_stop_delay_pass", amigos_stop_delay_pass, 2);
    ADD_CMD_HELP("amigos_stop_delay_pass", "[strKey] [port]", "Stop delay pass modules if its input have.",
                 "[strKey]: Module section name", "[port]: Input port id");
    ADD_CMD_VAR_ARG("amigos_set_attr", amigos_set_attr, 3);
    ADD_CMD_HELP("amigos_set_attr", "[strKey] [key] [...] [value]", "Set 'key' & 'value' of module in database.");
    ADD_CMD_VAR_ARG("amigos_set_in_attr", amigos_set_in_attr, 4);
    ADD_CMD_HELP("amigos_set_in_attr", "[strKey] [port] [key] [...] [value]", "Set 'key' & 'value' module's input in database.");
    ADD_CMD_VAR_ARG("amigos_set_out_attr", amigos_set_out_attr, 4);
    ADD_CMD_HELP("amigos_set_out_attr", "[strKey] [port] [key] [...] [value]", "Set 'key' & 'value' of module's output in database.");
    ADD_CMD("amigos_get_preview_windows", amigos_get_preview_windows, 0);
    ADD_CMD_HELP("amigos_get_preview_windows", "No argument.", "Get preview windows from current pipelie.");
    ADD_CMD("amigos_dump_env", amigos_dump_env, 1);
    ADD_CMD_HELP("amigos_dump_env", "[obj]", "Dump all environment message from module's object.");
    ADD_CMD("amigos_dump_in_env", amigos_dump_in_env, 2);
    ADD_CMD_HELP("amigos_dump_in_env", "[obj] [port]", "Dump all environment message from module's object input.");
    ADD_CMD("amigos_dump_out_env", amigos_dump_out_env, 2);
    ADD_CMD_HELP("amigos_dump_out_env", "[obj] [port]", "Dump all environment message from module's object output.");
    ADD_CMD("amigos_dump_all_env", amigos_dump_all_env, 1);
    ADD_CMD_HELP("amigos_dump_all_env", "[obj]", "Dump all environment message from module's object, input and output.");
    ADD_CMD("amigos_set_log_lv", amigos_set_log_lv, 1);
    ADD_CMD_HELP("amigos_set_log_lv", "[level]", "To mask log in amilog.", "[level]: 'trace', 'debug', 'warn', 'error'");
    ADD_CMD("amigos_get_env", amigos_get_env, 2);
    ADD_CMD_HELP("amigos_get_env", "[obj] [key]", "Get amigos env object's value by key.");
    ADD_CMD("amigos_get_in_env", amigos_get_in_env, 3);
    ADD_CMD_HELP("amigos_get_in_env", "[obj] [port] [key]", "Get amigos env object's input value by key.");
    ADD_CMD("amigos_get_out_env", amigos_get_out_env, 3);
    ADD_CMD_HELP("amigos_get_out_env", "[obj] [port] [key]", "Get amigos env object's output value by key.");
    ADD_CMD("amigos_set_env", amigos_set_env, 3);
    ADD_CMD_HELP("amigos_set_env", "[obj] [key] [val]", "Set amigos env object's value by key.");
    ADD_CMD("amigos_set_in_env", amigos_set_in_env, 4);
    ADD_CMD_HELP("amigos_set_in_env", "[obj] [port] [key] [val]", "Set amigos env object's input value by key.");
    ADD_CMD("amigos_set_out_env", amigos_set_out_env, 4);
    ADD_CMD_HELP("amigos_set_out_env", "[obj] [port] [key] [val]", "Set amigos env object's output value by key.");
    ADD_CMD("amigos_enable_shm_env", amigos_enable_shm_env, 1);
    ADD_CMD_HELP("amigos_enable_shm_env", "[is_father]","Enable shared memory for amigos module env buffer");
    ADD_CMD("amigos_disable_shm_env", amigos_disable_shm_env, 0);
    ADD_CMD_HELP("amigos_disable_shm_env", "No argument","Disable shared memory for amigos module env buffer");
#ifdef CONFIG_MOD_CMD
    ADD_CMD("amigos_access_obj", amigos_access_obj, 1);
    ADD_CMD_HELP("amigos_access_obj", "[dstKey]", "Switch current client env to dst amigos module's obj.");
#endif
}
