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

#ifndef __AMIGOS_H__
#define __AMIGOS_H__
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <map>
#include <string>
#include <iostream>
#include <signal.h>
#include "amigos_log.h"
#include "amigos_instance.h"
#include "amigos_database_factory.h"

#ifdef CONFIG_MOD_CMD
#include "ss_cmd_base.h"
#include "ss_console.h"
static long console_transfer_init(const char *key)
{
    return 0;
}
static int console_transfer_deinit(long fd)
{
    return 0;
}
static int console_send_cmd_json(long fd, const char *cmd, unsigned int size, nlohmann::json &json_out)
{
    ss_result result(json_out);
    int ret = run_cmd_trans_log(result, cmd);
    result.to_json(ret);
    return 0;
}
static int console_grab_tab_list(long fd, string &str)
{
    nlohmann::json json_out;
    ss_result result(json_out);
    int ret = run_cmd_trans_log(result, "internal_get_tabs");
    str = result.out_tab().str();
    return ret;
}
#endif

class Amigos
{
public:
    explicit Amigos(unsigned int myChipId, AmigosDatabaseFactoryBase *factory)
    {
        instance = new AmigosInstance(myChipId, factory);
        assert(instance);
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        //In pcie interlink case, create amigos block objects on target chip.
        //If it used local chip only, there is not remote object in use
        unsigned int slaveCount = 0;
        //Pcie Rc do this
        slaveCount = instance->mapSlaveChipIdRef.size();
        AMIGOS_INFO("PCIE Slaves count is %d\n", slaveCount);
        if (slaveCount)
        {
            run_cmd(0, "start_rpmsg 0 %d", slaveCount);
        }
        std::string strIn = AmigosDatabaseFactory::GetInFile();
        CreateRemoteObj(strIn);
#endif
    }
    virtual ~Amigos()
    {
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        unsigned int slaveCount = instance->mapSlaveChipIdRef.size();
        //Destroy amigos block object on target chip
        DestroyRemoteObj();
        if (slaveCount)
        {
            //Pcie Rc do this
            run_cmd(0, "stop_rpmsg");
            run_cmd(0, "clear_rpmsg_client");
        }
#endif
        delete instance;
    }
    void TakeOff()
    {
        instance->InitAll();
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        //Init amigos blocks on other chip for pcie interlink case.
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "init_all_blocks");
        }
#endif
        instance->BindAll();
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        //Bind amigos blocks on other chip for pcie interlink case.
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "bind_all_blocks");
        }
#endif
        instance->StartAll();
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        //Start amigos blocks on other chip for pcie interlink case.
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "start_all_blocks");
        }
#endif
    }
    void DropDown()
    {
        instance->StopAll();
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        //Stop amigos blocks on other chip for pcie interlink case.
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "stop_all_blocks");
        }
#endif
        instance->UnbindAll();
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        //Unbind amigos blocks on other chip for pcie interlink case.
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "unbind_all_blocks");
        }
#endif
        instance->DeinitAll();
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
        //Deinit amigos blocks on other chip for pcie interlink case.
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "deinit_all_blocks");
        }
#endif
    }
#if defined(INTERFACE_PCIE) && defined(CONFIG_MOD_CMD)
    void CreateRemoteObj(std::string &strIniPath)
    {
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "create_all_obj %s %d", strIniPath.c_str(), it->first);
        }
    }
    void DestroyRemoteObj()
    {
        for (auto it = instance->mapSlaveChipIdRef.begin(); it != instance->mapSlaveChipIdRef.end(); ++it)
        {
            run_cmd(it->first, "destroy_all_obj");
            run_cmd(it->first, "clear_rpmsg_client");
        }
    }
#endif
    AmigosInstance *GetIns()
    {
        return instance;
    }
private:
    AmigosInstance *instance;
};

static inline int amigos_setup_ui(int argc, char **argv)
{
    std::vector<std::string> vectDbFiles;
    std::string strInput;
    std::string outFile;

    unsigned int i = 0, sel = 0, isDriect = 0;
    int result;

    while ((result = getopt(argc, argv, "ir:o:")) != -1)
    {
        switch (result)
        {
            case 'i':
            {
                for (i = 0; i < (unsigned int)argc - optind; i++)
                {
                    if (argv[optind + i][0] == '-')
                    {
                        break;
                    }
                    vectDbFiles.push_back(argv[optind + i]);
                }
            }
            break;
            case 'r':
            {
                vectDbFiles.push_back(optarg);
                isDriect = 1;
            }
            break;
            case 'o':
            {
                outFile = optarg;
            }
            break;
            default:
                break;
        }
    }
    if (vectDbFiles.size() == 0)
    {
        std::cout << "Build info : Commit " << GIT_COMMIT << ", Build by " << BUILD_OWNER << ", Date " << BUILD_DATE << '.' << std::endl;
        std::cout << "Run json/ini files by selection menu  : " << argv[0] << " -i ./aaa.json ./bbb.json" << std::endl;
        std::cout << "Run script file directly without menu : " << argv[0] << " -r aaa.json" << std::endl;
        std::cout << "Convert ini script file to json       : " << argv[0] << " -r aaa.ini -o xxx.json" << std::endl;
        return -1;
    }
    if (isDriect)
    {
        if (outFile.size())
        {
            std::map<unsigned int, unsigned int> mapChipIdRef;
            unsigned int uintChipId = 0;
            AmigosDatabaseFactory factory(vectDbFiles[0], outFile);
            factory.LoadFile();
            AmigosDatabase *pSecRoot = factory.Create("ROOT");
            for (unsigned int uintLoopId = pSecRoot->GetRootLoopId();
                 uintLoopId != LOOP_ID_END; uintLoopId = pSecRoot->GetRootLoopId(uintLoopId))
            {
                uintChipId = pSecRoot->GetRootChipId(uintLoopId);
                if (mapChipIdRef.find(uintLoopId) == mapChipIdRef.end())
                {
                    mapChipIdRef[uintChipId] = 0;
                }
                mapChipIdRef[uintChipId]++;
            }
            factory.Destroy(pSecRoot);
            for (auto it = mapChipIdRef.begin(); it != mapChipIdRef.end(); ++it)
            {
                Amigos amigos(it->first, &factory);
            }
            factory.UnloadFile();
            return 0;
        }
#ifdef CONFIG_MOD_CMD
        run_cmd("amigos_create_pipeline %s 0", vectDbFiles[0].c_str());
        run_cmd("amigos_start_pipeline");
        while (strInput != "q")
        {
            printf("Press 'q' to exit!\n");
            printf("Press 'c' to enter debug!\n");
            std::cin >> strInput;
            if (strInput == "c")
            {
                struct ss_console_attr_s console_attr;

                console_attr.trans_init = console_transfer_init;
                console_attr.trans_deinit = console_transfer_deinit;
                console_attr.send_cmd = console_send_cmd_json;
                console_attr.grab_tab_list = console_grab_tab_list;
                ss_console(&console_attr, "123");
            }
        }
        run_cmd("amigos_stop_pipeline");
        run_cmd("amigos_destroy_pipeline");
#else
        AmigosDatabaseFactory factory(vectDbFiles[0]);
        factory.LoadFile();
        if (!outFile.size())
        {
            Amigos amigos(0, &factory);
            amigos.TakeOff();
            while (strInput != "q")
            {
                printf("Press 'q' to exit!\n");
                std::cin >> strInput;
            }
            amigos.DropDown();
        }
        factory.UnloadFile();
#endif
        return 0;
    }
#ifdef CONFIG_MOD_CMD
    bool bTakeOff = false;
#else
    Amigos *insAmigos = NULL;
    AmigosDatabaseFactory *insFactory = NULL;
#endif
    do
    {
        for (i = 0; i < vectDbFiles.size(); i++)
        {
            printf("Press '%d' to run: %s\n", i, vectDbFiles[i].c_str());
        }
        printf("Press 'q' to exit!\n");
#ifdef CONFIG_MOD_CMD
        printf("Press 'c' to enter debug!\n");
#endif
        std::cin >> strInput;
        if (strInput == "q")
        {
#ifdef CONFIG_MOD_CMD
            if (bTakeOff)
            {
                run_cmd("amigos_stop_pipeline");
                run_cmd("amigos_destroy_pipeline");
            }
#else
            if (insAmigos)
            {
                insAmigos->DropDown();
                delete insAmigos;
                insAmigos = NULL;
            }
            if (insFactory)
            {
                insFactory->UnloadFile();
                delete insFactory;
                insFactory = NULL;
            }
#endif
            break;
        }
#ifdef CONFIG_MOD_CMD
        else if (strInput == "c")
        {
            struct ss_console_attr_s console_attr;

            console_attr.trans_init = console_transfer_init;
            console_attr.trans_deinit = console_transfer_deinit;
            console_attr.send_cmd = console_send_cmd_json;
            console_attr.grab_tab_list = console_grab_tab_list;
            ss_console(&console_attr, "123");
            continue;
        }
#endif
        else if (strInput == "p")
        {
            continue;
        }
        sel = atoi(strInput.c_str());
        if ( sel < vectDbFiles.size())
        {
#ifdef CONFIG_MOD_CMD
            if (bTakeOff)
            {
                run_cmd("amigos_stop_pipeline");
                run_cmd("amigos_destroy_pipeline");
            }
            run_cmd("amigos_create_pipeline %s 0", vectDbFiles[sel].c_str());
            run_cmd("amigos_start_pipeline");
            bTakeOff = true;
#else
            if (insAmigos)
            {
                insAmigos->DropDown();
                delete insAmigos;
                insAmigos = NULL;
            }
            if (insFactory)
            {
                insFactory->UnloadFile();
                delete insFactory;
                insFactory = NULL;
            }
            insFactory = new AmigosDatabaseFactory(0, outFile);
            assert(insFactory);
            insFactory->LoadFile();
            insAmigos = new (std::nothrow)Amigos(0, insFactory);
            assert(insAmigos);
            insAmigos->TakeOff();
#endif
        }
        else
        {
            printf("Input select %s error!\n", strInput.c_str());
        }
    } while (1);

    return 0;
}
#endif //__AMIGOS_H__
