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
#ifndef __AMIGOS_DATABASE_H__
#define __AMIGOS_DATABASE_H__

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <iomanip>
#include <map>
#include "ss_auto_lock.h"
#include "amigos_log.h"
#define MAX_LOOP_CNT   64
#define LOOP_ID_START  (MAX_LOOP_CNT)
#define LOOP_ID_END    (MAX_LOOP_CNT + 1)

template <typename T>
class Default
{
public:
    static T Val() = delete;
};
template <>
class Default<int>
{
public:
    static int Val()
    {
        return 0;
    }
};
template <>
class Default<unsigned int>
{
public:
    static unsigned int Val()
    {
        return 0;
    }
};
template <>
class Default<long>
{
public:
    static long Val()
    {
        return 0;
    }
};
template <>
class Default<unsigned long>
{
public:
    static unsigned long Val()
    {
        return 0;
    }
};
template <>
class Default<const char *>
{
public:
    static const char *Val()
    {
        return "";
    }
};
template <>
class Default<std::string>
{
public:
    static std::string Val()
    {
        return "";
    }
};
template <>
class Default<std::vector<int>>
{
public:
    static std::vector<int> Val()
    {
        std::vector<int> emptyVector = {};
        return emptyVector;
    }
};
template <>
class Default<float>
{
public:
    static float Val()
    {
        return 0;
    }
};

class AmigosDatabase;

class AmigosDatabaseFactoryBase
{
public:
    AmigosDatabaseFactoryBase() {}
    virtual ~AmigosDatabaseFactoryBase() {}
    virtual AmigosDatabase *Create(const std::string &str) = 0;
    virtual void Destroy(AmigosDatabase *pDb) = 0;
};

class AmigosDatabase
{
    public:
        struct AmigosDatabaseMapSlice
        {
            unsigned int uintBlockX;
            unsigned int uintBlockH;
            std::vector<unsigned int> vectIoPorts[2];
            AmigosDatabaseMapSlice ()
                : uintBlockX(0), uintBlockH(0)
            {
            }
        };
        explicit AmigosDatabase(const std::string &section);
        virtual ~AmigosDatabase();
        virtual unsigned int GetRootLoopId(unsigned int uintId = LOOP_ID_START) = 0;
        virtual std::string GetRootEntryBlock(unsigned int uintId) = 0;
        virtual unsigned int GetRootChipId(unsigned int uintId) = 0;
        virtual bool GetRootDelayPass(unsigned int uintId) = 0;
        virtual unsigned int GetInLoopId(unsigned int uinitId = LOOP_ID_START) = 0;
        virtual unsigned int GetOutLoopId(unsigned int uinitId = LOOP_ID_START) = 0;

        template<typename T, typename... Args>
        T GetMod(Args &&... keyVal)
        {
            ss_auto_rdlock autoRdLock(rwlock);
            return GetValTraverse<T>(std::forward<Args>(keyVal)...);
        }

        template<typename T, typename... Args>
        T GetIn(unsigned int uintId, Args &&... keyVal)
        {
            ss_auto_rdlock autoRdLock(rwlock);
            if (ProcessIn(uintId) == 0)
            {
                return GetValTraverse<T>(std::forward<Args>(keyVal)...);
            }
            return Default<T>::Val();
        }

        template<typename T, typename... Args>
        T GetOut(unsigned int uintId, Args &&... keyVal)
        {
            ss_auto_rdlock autoRdLock(rwlock);
            if (ProcessOut(uintId) == 0)
            {
                return GetValTraverse<T>(std::forward<Args>(keyVal)...);
            }
            return Default<T>::Val();
        }

        template<typename... Args>
        int SetMod(Args &&... keyVal)
        {
            ss_auto_rwlock autoRwLock(rwlock);
            return SetValTraverse(std::forward<Args>(keyVal)...);
        }

        template<typename... Args>
        int SetIn(unsigned int uintId, Args &&... keyVal)
        {
            ss_auto_rwlock autoRwLock(rwlock);
            if (ConstructIn(uintId) == 0)
            {
                return SetValTraverse(std::forward<Args>(keyVal)...);
            }
            return -1;
        }

        template<typename... Args>
        int SetOut(unsigned int uintId, Args &&... keyVal)
        {
            ss_auto_rwlock autoRwLock(rwlock);
            if (ConstructOut(uintId) == 0)
            {
                return SetValTraverse(std::forward<Args>(keyVal)...);
            }
            return -1;
        }
        inline const std::string &GetSection() const
        {
            return this->strSection;
        }
        void ExpandBlockHeight(void)
        {
            auto iter = mapSlices.find(uintCurUsedSlice);
            if (iter == mapSlices.end())
            {
                AMILOG_ERR << "Internal ERR: Did not find map slice by current idx." << COLOR_ENDL;
                return;
            }
            iter->second.uintBlockH++;
        }
        void TraverseBlockPosition(unsigned int uintX, unsigned int uintY)
        {
            AmigosDatabaseMapSlice stSlice;
            if (uintCurUsedSlice != (unsigned int)-1)
            {
                auto iter = mapSlices.find(uintCurUsedSlice);
                if (iter == mapSlices.end())
                {
                    AMILOG_ERR << "Internal ERR: Did not find map slice by current idx." << COLOR_ENDL;
                    return;
                }
                if (uintCurUsedSlice >= uintY)
                {
                    AMILOG_ERR << "Internal ERR: CurrentUsedSlice will not be less than current." << COLOR_ENDL;
                    return;
                }
                if (iter->second.uintBlockH + uintCurUsedSlice == uintY
                    && iter->second.uintBlockX == uintX)
                {
                    iter->second.uintBlockH++;
                    //amilog.Out << "SEC: " << strSection << " EXPAND: " << "X:" << uintX << " Y:" << uintY << " H:" << iter->second.uintBlockH << std::endl;
                    return;
                }
                //To creat slice then.
            }
            stSlice.uintBlockH = 1;
            stSlice.uintBlockX = uintX;
            mapSlices[uintY]   = stSlice;
            uintCurUsedSlice   = uintY;
            //amilog.Out << "SEC: " << strSection << " INIT: " << "X:" << uintX << " Y:" << uintY << std::endl;
            return;
        }
        void HMapReverse(unsigned int uintReverseAxis)
        {
            for (auto iter = mapSlices.begin(); iter != mapSlices.end(); iter++)
            {
                if (uintReverseAxis < iter->second.uintBlockX)
                {
                    AMILOG_ERR << "Reverse Axis should be the largest x-coordinary of all map." << COLOR_ENDL;
                    return;
                }
            }
            for (auto iter = mapSlices.begin(); iter != mapSlices.end(); iter++)
            {
                iter->second.uintBlockX = uintReverseAxis - iter->second.uintBlockX;
            }
        }
        const std::map<unsigned int, AmigosDatabaseMapSlice> &GetBlockMap()
        {
            return mapSlices;
        }
        void SetInputId(const unsigned int &uintId)
        {
            auto iter = mapSlices.find(uintCurUsedSlice);
            if (iter == mapSlices.end())
            {
                AMILOG_ERR << "Internal ERR: Did not find map slice by current idx." << COLOR_ENDL;
                return;
            }
            iter->second.vectIoPorts[0].push_back(uintId);
        }
        void SetOutputId(const unsigned int &uintId)
        {
            auto iter = mapSlices.find(uintCurUsedSlice);
            if (iter == mapSlices.end())
            {
                AMILOG_ERR << "Internal ERR: Did not find map slice by current idx." << COLOR_ENDL;
                return;
            }
            iter->second.vectIoPorts[1].push_back(uintId);
        }
        AmigosDatabaseFactoryBase *GetFactory()
        {
            return this->insDatabaseFactory;
        }
        void SetFactory(AmigosDatabaseFactoryBase *factory)
        {
            this->insDatabaseFactory = factory;
        }
    protected:
        virtual int GetValInt(const char *keyVal, int intDef) = 0;
        virtual unsigned int GetValUint(const char *keyVal, unsigned int uintDef) = 0;
        virtual long GetValLong(const char *keyVal, long longDef) = 0;
        virtual unsigned long GetValUlong(const char *keyVal, unsigned long ulongDef) = 0;
        virtual std::string GetValStr(const char *keyVal, const char *pVal) = 0;
        virtual float GetValFloat(const char *keyVal, float floatDef) = 0;
        virtual int SetVal(const char *key, const char *value) = 0;
        virtual int ProcessKey(const char *key) = 0;
        virtual int ProcessIn(unsigned int uintId) = 0;
        virtual int ProcessOut(unsigned int uintId) = 0;
        virtual int ConstructKey(const char *key) = 0;
        virtual int ConstructIn(unsigned int uintId) = 0;
        virtual int ConstructOut(unsigned int uintId) = 0;
        long Atoi(std::string &str)
        {
            auto size = str.size();
            if (!size)
                return -1;

            if (size >= 2
                && str[0] == '0'
                && (str[1] == 'x' || str[1] == 'X'))
            {
                long ret = 0;
                sscanf(str.c_str(), "%lx", &ret);
                return ret;
            }
            return atol(str.c_str());
        }
        std::string strSection;
        static std::string strOutputFile;
        static std::string strInputFile;
    private:
        int GetValFinal(const char *keyVal, int intDef)
        {
            return GetValInt(keyVal, intDef);
        }
        unsigned int GetValFinal(const char *keyVal, unsigned int uintDef)
        {
            return GetValUint(keyVal, uintDef);
        }
        long GetValFinal(const char *keyVal, long longDef)
        {
            return GetValLong(keyVal, longDef);
        }
        unsigned long GetValFinal(const char *keyVal, unsigned long ulongDef)
        {
            return GetValUlong(keyVal, ulongDef);
        }
        std::string GetValFinal(const char *keyVal, std::string strDef)
        {
            return GetValStr(keyVal, strDef.c_str());
        }
        float GetValFinal(const char *keyVal, float floatDef)
        {
            return GetValFloat(keyVal, floatDef);
        }
        template<typename T, typename A>
        T GetValTraverse(A &&arg)
        {
            return GetValFinal(std::forward<A>(arg), Default<T>::Val());
        }

        template<typename T, typename A, typename... Args>
        T GetValTraverse(A &&arg, Args &&... keyVal)
        {
            if (ProcessKey(std::forward<A>(arg)) == 0)
            {
                return GetValTraverse<T>(std::forward<Args>(keyVal)...);
            }
            return Default<T>::Val();
        }

        int SetValTraverse(const char *key, const char *value)
        {
            return SetVal(key, value);
        }
        template<typename T, typename... Args>
        int SetValTraverse(T &&arg, Args &&... keyVal)
        {
            if (ConstructKey(std::forward<T>(arg)) == 0)
            {
                return SetValTraverse(std::forward<Args>(keyVal)...);
            }
            return -1;
        }
        std::map<unsigned int, AmigosDatabaseMapSlice> mapSlices; //Build for block slice use.
        unsigned int uintCurUsedSlice; //Current used slice.
        AmigosDatabaseFactoryBase *insDatabaseFactory;
        static pthread_rwlock_t rwlock;
};

#endif //__AMIGOS_DATABASE_H__
