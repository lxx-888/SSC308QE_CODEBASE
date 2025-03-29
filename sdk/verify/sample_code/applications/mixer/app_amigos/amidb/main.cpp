/*SigmaStar trade secret */
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

#include <iostream>
#include "amigos_database.h"
#include "amigos_database_factory.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        return -1;
    }
    AmigosDatabaseFactory factory(argv[1]);
    factory.LoadFile();
    AmigosDatabase *pSecRoot = factory.Create("ROOT");
    for (unsigned int uintLoopId = pSecRoot->GetRootLoopId();
         uintLoopId != LOOP_ID_END; uintLoopId = pSecRoot->GetRootLoopId(uintLoopId))
    {
        std::cout << "ENTRY BLOCK: " << pSecRoot->GetRootEntryBlock(uintLoopId) << std::endl;
        std::cout << "CHIP: " << pSecRoot->GetRootChipId(uintLoopId) << std::endl;
    }
    factory.Destroy(pSecRoot);
    AmigosDatabase *pSec = factory.Create(argv[2]);
    std::cout << "MOD: " << pSec->GetMod<std::string>("MOD") << std::endl;
    std::cout << "DEV: " << pSec->GetMod<int>("DEV") << std::endl;
    std::cout << "CHN: " << pSec->GetMod<unsigned int>("CHN") << std::endl;
    for (unsigned int uintLoopId = pSec->GetInLoopId();
         uintLoopId != LOOP_ID_END; uintLoopId = pSec->GetInLoopId(uintLoopId))
    {
        std::cout << "Input Port: " << uintLoopId << " >>>>>>>>>>" << std::endl;
        std::cout << "    Prev: " << pSec->GetIn<std::string>(uintLoopId, "PREV") << std::endl;
        std::cout << "    Fps: " << pSec->GetIn<std::string >(uintLoopId, "FPS") << std::endl;
        std::cout << "    Bind Type: " << pSec->GetIn<unsigned int>(uintLoopId, "BIND_TYPE") << std::endl;
        std::cout << "    Bind Para: " << pSec->GetIn<int>(uintLoopId, "BIND_PARAM") << std::endl;
    }
    for (unsigned int uintLoopId = pSec->GetOutLoopId();
         uintLoopId != LOOP_ID_END; uintLoopId = pSec->GetOutLoopId(uintLoopId))
    {
        std::cout << "Output Port: " << uintLoopId << " >>>>>>>>>>" << std::endl;
        std::cout << "    Fps: " << pSec->GetOut<unsigned int>(uintLoopId, "FPS") << std::endl;
    }
    factory.Destroy(pSec);
    factory.UnloadFile();
    return 0;
}

