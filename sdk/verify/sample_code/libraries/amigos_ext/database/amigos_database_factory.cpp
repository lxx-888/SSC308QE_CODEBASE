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
#include <fstream>
#include "amigos_log.h"
#ifdef CONFIG_DB_INI
#include "amigos_database_ini.h"
#endif
#ifdef CONFIG_DB_JSON
#include "amigos_database_json.h"
#include "amigos_database_json_out.h"
#endif
#include "amigos_database_factory.h"

#ifdef CONFIG_DB_JSON
template <class T>
nlohmann::ordered_json AmigosDatabaseJsonOut<T>::JsonOut;
#endif

AmigosDatabaseFactory::AmigosDatabaseFactory(const std::string &strIn)
{
    strInputFile = strIn;
}

AmigosDatabaseFactory::AmigosDatabaseFactory(const std::string &strIn, const std::string &strOut)
{
    strInputFile = strIn;
    strOutputFile = strOut;
}

const std::string &AmigosDatabaseFactory::GetInFile()
{
    return strInputFile;
}
const std::string &AmigosDatabaseFactory::GetOutFile()
{
    return strOutputFile;
}
int AmigosDatabaseFactory::LoadFile()
{
    size_t findOffset = 0;
    if (!strInputFile.size())
    {
        amilog.Error() << CODE_TRACE << "File empty!" << COLOR_ENDL;
        return -1;
    }
    findOffset = strInputFile.find_last_of('.');
    if (findOffset == std::string::npos)
    {
        amilog.Error() << CODE_TRACE << "File name: " << strInputFile << " error!" << COLOR_ENDL;
        return -1;
    }
#ifdef CONFIG_INI
    if (AmigosDatabaseIni::pDict)
    {
        amilog.Error() << CODE_TRACE << "Only one for load." << COLOR_ENDL;
        return -1;
    }
    if (findOffset + 3 == strInputFile.size()
        && !strInputFile.compare(findOffset, 4, ".ini"))
    {
        if (AmigosDatabaseIni::pDict)
        {
            return -1;
        }
        AmigosDatabaseIni::pDict = iniparser_load(strInputFile.c_str());
        if (!AmigosDatabaseIni::pDict)
        {
            amilog.Error() << CODE_TRACE << "Script file: " << strInputFile << " open error!" << COLOR_ENDL;
            return -1;
        }
        amilog.Info() << CODE_TRACE << "Load Ini File: " << strInputFile << " success!" << COLOR_ENDL;
        return 0;
    }
#endif
#ifdef CONFIG_DB_JSON
    if (AmigosDatabaseJson::Json.size())
    {
        amilog.Error() << CODE_TRACE << "Only one for load." << COLOR_ENDL;
        return -1;
    }
    if (findOffset + 5 == strInputFile.size()
        && !strInputFile.compare(findOffset, 5, ".json"))
    {
        if (AmigosDatabaseJson::Json.size())
        {
            return -1;
        }
        std::ifstream fin(strInputFile.c_str());
        if (!fin.is_open())
        {
            amilog.Error() << CODE_TRACE << "Can not open: " << strInputFile<< COLOR_ENDL;
            return -1;
        }
        fin >> AmigosDatabaseJson::Json;
        amilog.Info() << CODE_TRACE << "Load Json File: " << strInputFile << " success!" << COLOR_ENDL;
        return 0;
    }
#endif
    amilog.Error() << CODE_TRACE << "Not support: " << strInputFile << COLOR_ENDL;
    return -1;
}
int AmigosDatabaseFactory::UnloadFile()
{
#ifdef CONFIG_DB_INI
    if (AmigosDatabaseIni::pDict)
    {
        iniparser_freedict(AmigosDatabaseIni::pDict);
        AmigosDatabaseIni::pDict = NULL;
        if (strOutputFile.size())
        {
#ifdef CONFIG_DB_JSON
            std::ofstream fOut(strOutputFile.c_str());
            if (!fOut.is_open())
            {
                amilog.Error() << CODE_TRACE << "Can not open: " << strOutputFile << COLOR_ENDL;
                return -1;
            }
            fOut << AmigosDatabaseJsonOut<AmigosDatabaseIni>::JsonOut.dump(4);
#endif
        }
        return 0;
    }
#endif
#ifdef CONFIG_DB_JSON
    if (AmigosDatabaseJson::Json.size())
    {
        AmigosDatabaseJson::Json.clear();
        if (strOutputFile.size())
        {
            std::ofstream fOut(strOutputFile.c_str());
            if (!fOut.is_open())
            {
                amilog.Error() << CODE_TRACE << "Can not open: " << strOutputFile << COLOR_ENDL;
                return -1;
            }
            fOut << AmigosDatabaseJsonOut<AmigosDatabaseJson>::JsonOut.dump(4);
        }
        return 0;
    }
#endif
    return -1;
}
AmigosDatabase *AmigosDatabaseFactory::Create(const std::string &strSec)
{
    AmigosDatabase *pDbIns = NULL;
#ifdef CONFIG_DB_INI
    if (AmigosDatabaseIni::pDict)
    {
        if (strOutputFile.size())
        {
#ifdef CONFIG_DB_JSON
            pDbIns = new (std::nothrow)AmigosDatabaseJsonOut<AmigosDatabaseIni>(strSec);
#endif
        }
        else
        {
            pDbIns = new (std::nothrow)AmigosDatabaseIni(strSec);
        }
    }
#endif
#ifdef CONFIG_DB_JSON
    if (strOutputFile.size())
    {
        pDbIns = new (std::nothrow)AmigosDatabaseJsonOut<AmigosDatabaseJson>(strSec);
    }
    else
    {
        pDbIns = new (std::nothrow)AmigosDatabaseJson(strSec);
    }
#endif
    assert(pDbIns);
    pDbIns->SetFactory(this);
    return pDbIns;
}
void AmigosDatabaseFactory::Destroy(AmigosDatabase *pDb)
{
    delete pDb;
}
