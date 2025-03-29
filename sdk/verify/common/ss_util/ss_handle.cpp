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
#include "ss_handle.h"

using namespace std;

template <class T>
std::map<std::string, T *> ss_handle_template<T>::handle_map;

ss_handle::ss_handle()
{
}
ss_handle::~ss_handle()
{
}
ss_handle *ss_handle::install(const string &handle, ss_handle *instance)
{
    ss_handle *ret_ins = ss_handle_template<ss_handle>::install(handle, instance);
    if (ret_ins)
    {
        instance->self_handle = handle;
    }
    return ret_ins;
}
ss_handle *ss_handle::get(const string &handle)
{
    return ss_handle_template<ss_handle>::get(handle);
}
int ss_handle::destroy(const string &handle)
{
    return ss_handle_template<ss_handle>::destroy(handle);
}
const string &ss_handle::get_handle()
{
    return self_handle;
}
