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
#ifndef __AMIGOS_DATABASE_JSON_OUT_H__
#define __AMIGOS_DATABASE_JSON_OUT_H__

#include <sstream>
#include <string>
#include "amigos_database.h"
#include "json.hpp"


template <class T>
class AmigosDatabaseJsonOut : public T
{
    public:
        explicit AmigosDatabaseJsonOut(const std::string &section) : T(section)
        {
            JsonOut[T::strSection] = nullptr;
            pDstJsonOut = nullptr;
        }
        virtual ~AmigosDatabaseJsonOut()
        {
            if (T::strSection == "ROOT")
            {
                return;
            }
            AmigosDatabase::GetMod<std::string>("IN_CNT");
            AmigosDatabase::GetMod<std::string>("OUT_CNT");
            //This is for bw tool's ui location.
            AmigosDatabase::GetMod<std::string>("POS_X");
            AmigosDatabase::GetMod<std::string>("POS_Y");
        }
        virtual unsigned int GetRootLoopId(unsigned int uintId)
        {
            return T::GetRootLoopId(uintId);
        }
        virtual std::string GetRootEntryBlock(unsigned int uintId)
        {
            auto retVal = T::GetRootEntryBlock(uintId);
            if (retVal.size())
            {
                nlohmann::ordered_json &rootJson = JsonOut[T::strSection][uintId];
                rootJson["NAME"] = retVal;
            }
            return retVal;
        }
        virtual unsigned int GetRootChipId(unsigned int uintId)
        {
            unsigned int uintChip = T::GetRootChipId(uintId);
            std::stringstream ss;
            nlohmann::ordered_json &rootJsonChipId = JsonOut[T::strSection][uintId];
            ss << uintChip;
            rootJsonChipId["CHIP"] = ss.str();
            return uintChip;
        }
        virtual bool GetRootDelayPass(unsigned int uintId)
        {
            bool bDelay = T::GetRootDelayPass(uintId);
            std::stringstream ss;
            nlohmann::ordered_json &rootJsonChipId = JsonOut[T::strSection][uintId];
            ss << (bDelay ? 1 : 0);
            rootJsonChipId["DELAY"] = ss.str();
            return bDelay;
        }
        virtual unsigned int GetInLoopId(unsigned int uintId)
        {
            return T::GetInLoopId(uintId);
        }
        virtual unsigned int GetOutLoopId(unsigned int uintId)
        {
            return T::GetOutLoopId(uintId);
        }
        virtual int GetValInt(const char *keyVal, int intDef)
        {
            auto retVal = T::GetValInt(keyVal, intDef);
            nlohmann::ordered_json &targetJson = GetObj();
            targetJson[keyVal] = IntToString(retVal);
            return retVal;
        }
        virtual unsigned int GetValUint(const char *keyVal, unsigned int uintDef)
        {
            auto retVal = T::GetValUint(keyVal, uintDef);
            nlohmann::ordered_json &targetJson = GetObj();
            targetJson[keyVal] = IntToString(retVal);
            return retVal;
        }
        virtual long GetValLong(const char *keyVal, long longDef)
        {
            auto retVal = T::GetValLong(keyVal, longDef);
            nlohmann::ordered_json &targetJson = GetObj();
            targetJson[keyVal] = LongToString(retVal);
            return retVal;
        }
        virtual unsigned long GetValUlong(const char *keyVal, unsigned long ulongDef)
        {
            auto retVal = T::GetValLong(keyVal, ulongDef);
            nlohmann::ordered_json &targetJson = GetObj();
            targetJson[keyVal] = LongToString(retVal);
            return retVal;
        }
        virtual std::string GetValStr(const char *keyVal, const char *pVal)
        {
            auto retVal = T::GetValStr(keyVal, pVal);
            nlohmann::ordered_json &targetJson = GetObj();
            targetJson[keyVal] = retVal;
            return retVal;
        }
        virtual int ProcessKey(const char *key)
        {
            T::ProcessKey(key);
            GetObj(key);
            return 0;
        }
        virtual int ProcessIn(unsigned int uintId)
        {
            T::ProcessIn(uintId);
            std::stringstream ss;
            ss << "IN_" << uintId;
            GetObj(ss.str());
            return 0;
        }
        virtual int ProcessOut(unsigned int uintId)
        {
            T::ProcessOut(uintId);
            std::stringstream ss;
            ss << "OUT_" << uintId;
            GetObj(ss.str());
            return 0;
        }
        static nlohmann::ordered_json JsonOut;
    private:
        nlohmann::ordered_json *pDstJsonOut;
        nlohmann::ordered_json &GetObj(const std::string &nextKey = "")
        {
            nlohmann::ordered_json &retObj = pDstJsonOut ? *pDstJsonOut : JsonOut[T::strSection];
            if (nextKey.size())
            {
                pDstJsonOut = &retObj[nextKey];
                return retObj;
            }
            pDstJsonOut = nullptr;
            return retObj;
        }
        std::string IntToString(int val)
        {
            std::stringstream ss;
            ss << std::dec << val;
            return ss.str();
        }
        std::string LongToString(long val)
        {
            std::stringstream ss;
            ss << std::dec << val;
            return ss.str();
        }
};
#endif
