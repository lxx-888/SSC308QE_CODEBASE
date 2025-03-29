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

#include <iostream>
#include <string>
#include <sstream>
#include "ss_auto_lock.h"
#include "amigos_database_ini.h"

#define DBG_PRINT 0
using std::istringstream;

dictionary *AmigosDatabaseIni::pDict = NULL;
AmigosDatabaseIni::AmigosDatabaseIni(const std::string &section) : AmigosDatabase(section)
{
    std::string key;
    std::stringstream ss;
    std::string strGet;
    int intCount = 0;
    unsigned int uintPort = 0;

    assert(pDict);
    assert(section.size());
    key = strSection + ':' + "IN_CNT";
    int inCnt = iniparser_getint(pDict, key.c_str(), 0);
    key = strSection + ':' + "OUT_CNT";
    int outCnt = iniparser_getint(pDict, key.c_str(), 0);
    if (strSection == "ROOT")
    {
        key = strSection + ':' + "COUNT";
        int blockCnt = iniparser_getint(pDict, key.c_str(), 0);
        if (blockCnt)
        {
            for (int i = 0; i < blockCnt; i++)
            {
                ss << strSection + ':' + "NAME_" << i;
                const char *pBlockName = iniparser_getstring(pDict, ss.str().c_str(), NULL);
                ss.str("");
                ss << strSection + ':' + "NAME_" << i << "_CHIP_ID";
                unsigned int uintChipId = iniparser_getunsignedint(pDict, ss.str().c_str(), 0);
                ss.str("");
                ss << strSection + ':' + "NAME_" << i << "_DELAY";
                bool delayPass = (bool)iniparser_getunsignedint(pDict, ss.str().c_str(), 0);
                ss.str("");
                if (pBlockName)
                {
                    mapStrRootBlock[i] = pBlockName;
                    mapStrRootBlockChipId[i] = uintChipId;
                    mapStrRootBlockDelayPass[i] = delayPass;
                }
            }
        }
        return;
    }
    if (inCnt)
    {
        for (int i = 0; i < MAX_LOOP_CNT; i++)
        {
            ss << strSection + ':' + "IN_" << i;
            strGet = iniparser_getstring(pDict, ss.str().c_str(), (char *)"");
            ss.str("");
            if (strGet.size())
            {
                ss << strGet + ':' + "PORT";
                uintPort = iniparser_getunsignedint(pDict, ss.str().c_str(), -1);
                ss.str("");
                mapStrInSection[uintPort] = strGet;
                intCount++;
            }
            if (intCount == inCnt)
            {
                break;
            }
        }
    }
    if (outCnt)
    {
        intCount = 0;
        for (unsigned int i = 0; i < MAX_LOOP_CNT; i++)
        {
            ss << strSection + ':' + "OUT_" << i;
            strGet = iniparser_getstring(pDict, ss.str().c_str(), (char *)"");
            ss.str("");
            if (strGet.size())
            {
                ss << strGet + ':' + "PORT";
                uintPort = iniparser_getunsignedint(pDict, ss.str().c_str(), -1);
                ss.str("");
                mapStrOutSection[uintPort] = strGet;
                intCount++;
            }
            if (intCount == outCnt)
            {
                break;
            }
        }
    }
}
AmigosDatabaseIni::~AmigosDatabaseIni()
{
}
unsigned int AmigosDatabaseIni::GetRootLoopId(unsigned int uintId)
{
    std::map<unsigned int, std::string>::iterator it;
    if (LOOP_ID_START == uintId)
    {
        it = mapStrRootBlock.begin();
    }
    else
    {
        it = mapStrRootBlock.find(uintId);
        if (it == mapStrRootBlock.end())
        {
            return LOOP_ID_END;
        }
        it++;
    }
   return it == mapStrRootBlock.end() ? LOOP_ID_END : it->first;
}
std::string AmigosDatabaseIni::GetRootEntryBlock(unsigned int uintId)
{
    auto it = mapStrRootBlock.find(uintId);
    if (it == mapStrRootBlock.end())
    {
        return NULL;
    }
    return it->second;
}
unsigned int AmigosDatabaseIni::GetRootChipId(unsigned int uintId)
{
    auto it = mapStrRootBlockChipId.find(uintId);
    if (it == mapStrRootBlockChipId.end())
    {
        return 0;
    }
    return it->second;
}
bool AmigosDatabaseIni::GetRootDelayPass(unsigned int uintId)
{
    auto it = mapStrRootBlockDelayPass.find(uintId);
    if (it == mapStrRootBlockDelayPass.end())
    {
        return 0;
    }
    return it->second;
}
unsigned int AmigosDatabaseIni::GetInLoopId(unsigned int uinitId)
{
    std::map<unsigned int, std::string>::iterator it;
    if (LOOP_ID_START == uinitId)
    {
        it = mapStrInSection.begin();
    }
    else
    {
        it = mapStrInSection.find(uinitId);
        if (it == mapStrInSection.end())
        {
            return LOOP_ID_END;
        }
        it++;
    }
    return it == mapStrInSection.end() ? LOOP_ID_END : it->first;
}
unsigned int AmigosDatabaseIni::GetOutLoopId(unsigned int uinitId)
{
    std::map<unsigned int, std::string>::iterator it;
    if (LOOP_ID_START == uinitId)
    {
        it = mapStrOutSection.begin();
    }
    else
    {
        it = mapStrOutSection.find(uinitId);
        if (it == mapStrOutSection.end())
        {
            return LOOP_ID_END;
        }
        it++;
    }
    return it == mapStrOutSection.end() ? LOOP_ID_END : it->first;
}
int AmigosDatabaseIni::GetValInt(const char *keyVal, int intDef)
{
    std::string key;

    key = strTraverse + ':' + keyVal;
    intDef = iniparser_getint(pDict, key.c_str(), intDef);
#if DBG_PRINT
    AMILOG_INFO << key << "=" << '['  << intDef << ']' << COLOR_ENDL;
#endif
    strTraverse = strSection;
    return intDef;
}
unsigned int AmigosDatabaseIni::GetValUint(const char *keyVal, unsigned int uintDef)
{
    std::string key;

    key = strTraverse + ':' + keyVal;
    uintDef = iniparser_getunsignedint(pDict, key.c_str(), uintDef);
#if DBG_PRINT
    AMILOG_INFO << key << "=" << '['  << uintDef << ']' << COLOR_ENDL;
#endif
    strTraverse = strSection;
    return uintDef;
}
long AmigosDatabaseIni::GetValLong(const char *keyVal, long longDef)
{
    std::string key;
    char longChar[64];

    snprintf(longChar, 64, "%ld", longDef);
    key = strTraverse + ':' + keyVal;
    std::string getString = iniparser_getstring(pDict, key.c_str(), longChar);
#if DBG_PRINT
    AMILOG_INFO << key << "=" << '['  << getString << ']' << COLOR_ENDL;
#endif
    strTraverse = strSection;
    return Atoi(getString);
}
unsigned long AmigosDatabaseIni::GetValUlong(const char *keyVal, unsigned long ulongDef)
{
    std::string key;
    char longChar[64];

    snprintf(longChar, 64, "%lu", ulongDef);
    key = strTraverse + ':' + keyVal;
    std::string getString = iniparser_getstring(pDict, key.c_str(), longChar);
#if DBG_PRINT
    AMILOG_INFO << key << "=" << '['  << getString << ']' << COLOR_ENDL;
#endif
    strTraverse = strSection;
    return (unsigned long)Atoi(getString);
}
std::string AmigosDatabaseIni::GetValStr(const char *keyVal, const char *pDef)
{
    std::string key;

    key = strTraverse + ':' + keyVal;
    pDef = iniparser_getstring(pDict, key.c_str(), pDef);
#if DBG_PRINT
    AMILOG_INFO << key << "=" << '['  << uintDef << ']' << COLOR_ENDL;
#endif
    strTraverse = strSection;
    return pDef;
}
float AmigosDatabaseIni::GetValFloat(const char *keyVal, float floatDef)
{
    std::string key;
    char floatChar[64];
    float fValue;

    snprintf(floatChar, 64, "%fu", floatDef);

    key = strTraverse + ':' + keyVal;
    std::string getString = iniparser_getstring(pDict, key.c_str(), floatChar);
#if DBG_PRINT
    AMILOG_INFO << key << "=" << '['  << uintDef << ']' << COLOR_ENDL;
#endif
    strTraverse = strSection;
    istringstream(getString) >> fValue;
    return fValue;
}
int AmigosDatabaseIni::SetVal(const char *keyVal, const char *value)
{
    int ret = 0;
    std::string key;

    key = strTraverse + ':' + keyVal;
    ret = iniparser_setstring(pDict, key.c_str(), value);
#if DBG_PRINT
    AMILOG_INFO << key << "=" << '['  << uintDef << ']' << COLOR_ENDL;
#endif
    strTraverse = strSection;
    return ret;
}
int AmigosDatabaseIni::ProcessKey(const char *key)
{
    std::string &strKey = strTraverse;

    strKey = strKey + ':' + key;
    strKey = iniparser_getstring(pDict, strKey.c_str(), "");
    if (strKey == "")
    {
        strKey = strSection;
        AMILOG_ERR << strSection << ": Can't find " << key << COLOR_ENDL;
        return -1;
    }
    return 0;
}
int AmigosDatabaseIni::ProcessIn(unsigned int uintId)
{
    std::string &strKey = strTraverse;

    auto it = mapStrInSection.find(uintId);
    if (it == mapStrInSection.end())
    {
        strKey = strSection;
        AMILOG_ERR << __LINE__ << '|'<<__FUNCTION__ << '|' << "In loop id " << uintId
                   << " Not found! " << " Sec: " << strSection << COLOR_ENDL;
        return -1;
    }
    strKey = it->second;
    return 0;
}
int AmigosDatabaseIni::ProcessOut(unsigned int uintId)
{
    std::string &strKey = strTraverse;

    auto it = mapStrOutSection.find(uintId);
    if (it == mapStrOutSection.end())
    {
        strKey = strSection;
        AMILOG_ERR << __LINE__ << '|'<<__FUNCTION__ << '|' << "Out loop id " << uintId
                   << " Not found! " << " Sec: " << strSection << COLOR_ENDL;
        return -1;
    }
    strKey = it->second;
    return 0;
}
int AmigosDatabaseIni::ConstructKey(const char *key)
{
    ProcessKey(key);
    return -1;
}
int AmigosDatabaseIni::ConstructIn(unsigned int uintId)
{
    ProcessIn(uintId);
    return -1;
}
int AmigosDatabaseIni::ConstructOut(unsigned int uintId)
{
    ProcessOut(uintId);
    return -1;
}

