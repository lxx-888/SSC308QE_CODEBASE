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
#ifndef __SS_MLOAD__
#define __SS_MLOAD__
#include <string.h>
#include <assert.h>
#include "ss_handle.h"
#include "ss_cmd_base.h"

struct mload_self_data
{
    char *data;
    unsigned int size;
    int (*judge)(void *, unsigned int);
};

class ss_mload : public ss_handle
{
    public:
        explicit ss_mload()
        {
        }
        virtual ~ss_mload()
        {
            for (int i = 0; i < (int)store_data.size(); i++)
            {
                delete []store_data[i].data;
            }
            store_data.clear();
            store_cmds.clear();
        }
        int register_judge_func(int (*judge)(void *, unsigned int), void *data, unsigned int size)
        {
            struct mload_self_data self_data;
            char * user_data = NULL;

            user_data = new char[size];
            assert(user_data);
            memcpy(user_data, data, size);
            self_data.judge = judge;
            self_data.data = user_data;
            self_data.size = size;
            store_data.push_back(self_data);
            return 0;
        }
        void add_cmds(const string &cmd_str)
        {
            store_cmds.push_back(cmd_str);
        }
        static ss_mload *create(const string &handle)
        {
            ss_mload *mload= new ss_mload();
            assert(mload);
            if (!ss_handle::install(handle, mload))
            {
                delete mload;
                return NULL;
            }
            return mload;
        }
        int run(void)
        {
            int ret = 0;
            bool bset_off = false;
            if (sslog.is_logon())
            {
                bset_off = true;
                sslog.set_log(false);
            }
            while (1)
            {
                ret = 0;
                for (unsigned int i = 0; i < store_data.size(); i++)
                {
                    if (store_data[i].judge)
                    {
                        ret |= store_data[i].judge(store_data[i].data, store_data[i].size);
                    }
                }
                if (ret < 0)
                {
                    return -1;
                }
                if (!ret)
                {
                    break;
                }
                for (unsigned int i = 0; i < store_cmds.size(); i++)
                {
                    if (run_cmd_nolock(store_cmds[i].c_str()) < 0)
                    {
                        return -1;
                    }
                }
            }
            if (bset_off)
            {
                sslog.set_log(true);
            }
            return 0;
        }
    private:
        vector<string> store_cmds;
        vector<struct mload_self_data> store_data;
};
#endif
