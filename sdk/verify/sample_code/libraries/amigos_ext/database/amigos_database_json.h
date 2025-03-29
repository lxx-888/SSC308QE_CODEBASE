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
#ifndef __AMIGOS_DATABASE_JSON_H__
#define __AMIGOS_DATABASE_JSON_H__

#include "amigos_database.h"
#include "json.hpp"

class AmigosDatabaseJson : public AmigosDatabase
{
    public:
        explicit AmigosDatabaseJson(const std::string &section);
        virtual ~AmigosDatabaseJson();
        virtual unsigned int GetRootLoopId(unsigned int uintId);
        virtual std::string GetRootEntryBlock(unsigned int uintId);
        virtual unsigned int GetRootChipId(unsigned int uintId);
        virtual bool GetRootDelayPass(unsigned int uintId);
        virtual unsigned int GetInLoopId(unsigned int initId);
        virtual unsigned int GetOutLoopId(unsigned int initId);
        static nlohmann::json Json;
    protected:
        virtual int GetValInt(const char *keyVal, int intDef);
        virtual unsigned int GetValUint(const char *keyVal, unsigned int uintDef);
        virtual long GetValLong(const char *keyVal, long longDef);
        virtual unsigned long GetValUlong(const char *keyVal, unsigned long ulongDef);
        virtual std::string GetValStr(const char *keyVal, const char *pVal);
        virtual float GetValFloat(const char *keyVal, float floatDef);
        virtual int SetVal(const char *key, const char *value);
        virtual int ProcessKey(const char *key);
        virtual int ProcessIn(unsigned int uintId);
        virtual int ProcessOut(unsigned int uintId);
        virtual int ConstructKey(const char *key);
        virtual int ConstructIn(unsigned int uintId);
        virtual int ConstructOut(unsigned int uintId);
    private:
        nlohmann::json *GetJsonObj();
        nlohmann::json *GetJsonObj(const std::string &strKey);
        int GetJsonValue(const nlohmann::json &jsonObj, const std::string &strKey, std::string &strVal);
        nlohmann::json *dstJson;
};
#endif //__AMIGOS_DATABASE_JSON_H__
