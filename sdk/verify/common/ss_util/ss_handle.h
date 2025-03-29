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
#ifndef __SS_HANDLE__
#define __SS_HANDLE__
#include <assert.h>
#include <map>
#include <iostream>

template <class T>
class ss_handle_template
{
public:
    ss_handle_template() {}
    virtual ~ss_handle_template() {}
    static T *install(const std::string &handle, T *instance)
    {
        auto finder = handle_map.find(handle);
        if (!instance)
        {
            std::cout << "NULL instance" << std::endl;
            return NULL;
        }
        if (finder != handle_map.end())
        {
            std::cout << "Double create!" << std::endl;
            return NULL;
        }
        handle_map[handle] = instance;
        return instance;
    }
    static int destroy(const std::string &handle)
    {
        //std::map<std::string, T *>::iterator finder = handle_map.find(handle);
        auto finder = handle_map.find(handle);
        if (finder == handle_map.end())
        {
            std::cout << "Can't find handle: " << handle << std::endl;
            return -1;
        }
        delete finder->second;
        finder->second = NULL;
        handle_map.erase(finder);
        return 0;
    }
    static T *get(const std::string &handle)
    {
        //std::map<std::string, T *>::iterator finder = handle_map.find(handle);
        auto finder = handle_map.find(handle);
        if (finder != handle_map.end())
        {
            return finder->second;
        }
        std::cout << "Can't find handle: " << handle << std::endl;
        return NULL;
    }
    static void clear()
    {
        for (auto &it : handle_map)
        {
            delete it.second;
        }
        handle_map.clear();
    }
private:
    static std::map<std::string, T *>handle_map;
};

class ss_handle
{
public:
    ss_handle();
    virtual ~ss_handle();
    static ss_handle *install(const std::string &handle, ss_handle *instance);
    static ss_handle *get(const std::string &handle);
    static int destroy(const std::string &handle);
    const std::string &get_handle();
protected:
    std::string self_handle;
};
#endif
