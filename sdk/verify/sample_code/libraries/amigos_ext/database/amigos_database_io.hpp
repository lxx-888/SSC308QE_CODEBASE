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

#ifndef __AMIGOS_DATABASE_IO_HPP__
#define __AMIGOS_DATABASE_IO_HPP__
#include <string>
#include <vector>
#include "amigos_database.h"

template<typename T>
static int amigos_get_val(AmigosDatabase *pDb, const std::vector<std::string> &in_strs,
                     int start, T &out_val)
{
    if (!pDb)
    {
        return -1;
    }
    if (in_strs.size() - start == 1)
    {
        out_val = pDb->GetMod<T>(in_strs[start].c_str());
        return 0;
    }
    if (in_strs.size() - start == 2)
    {
        out_val = pDb->GetMod<T>(in_strs[start].c_str(), in_strs[start + 1].c_str());
        return 0;
    }
    if (in_strs.size() - start == 3)
    {
        out_val = pDb->GetMod<T>(in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                 in_strs[start + 2].c_str());
        return 0;
    }
    if (in_strs.size() - start == 4)
    {
        out_val = pDb->GetMod<T>(in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                 in_strs[start + 2].c_str(), in_strs[start + 3].c_str());
        return 0;
    }
    if (in_strs.size() - start == 5)
    {
        out_val = pDb->GetMod<T>(in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                 in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                                 in_strs[start + 4].c_str());
        return 0;
    }
    AMILOG_ERR << "Not Support Current Para's Count." << std::endl << PRINT_COLOR_END;
    return -1;
}
template<typename T>
static int amigos_get_in_val(AmigosDatabase *pDb, unsigned int loopId, const std::vector<std::string> &in_strs,
                        int start, T &out_val)
{
    if (!pDb)
    {
        return -1;
    }
    if (in_strs.size() - start == 1)
    {
        out_val = pDb->GetIn<T>(loopId, in_strs[start].c_str());
        return 0;
    }
    if (in_strs.size() - start == 2)
    {
        out_val = pDb->GetIn<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str());
        return 0;
    }
    if (in_strs.size() - start == 3)
    {
        out_val = pDb->GetIn<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                in_strs[start + 2].c_str());
        return 0;
    }
    if (in_strs.size() - start == 4)
    {
        out_val = pDb->GetIn<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                in_strs[start + 2].c_str(), in_strs[start + 3].c_str());
        return 0;
    }
    if (in_strs.size() - start == 5)
    {
        out_val = pDb->GetIn<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                                in_strs[start + 4].c_str());
        return 0;
    }
    AMILOG_ERR << "Not Support Current Para's Count." << std::endl << PRINT_COLOR_END;
    return -1;
}
template<typename T>
static int amigos_get_out_val(AmigosDatabase *pDb, unsigned int loopId, const std::vector<std::string> &in_strs,
                         int start, T &out_val)
{
    if (!pDb)
    {
        return -1;
    }
    if (in_strs.size() - start == 1)
    {
        out_val = pDb->GetOut<T>(loopId, in_strs[start].c_str());
        return 0;
    }
    if (in_strs.size() - start == 2)
    {
        out_val = pDb->GetOut<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str());
        return 0;
    }
    if (in_strs.size() - start == 3)
    {
        out_val = pDb->GetOut<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                 in_strs[start + 2].c_str());
        return 0;
    }
    if (in_strs.size() - start == 4)
    {
        out_val = pDb->GetOut<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                 in_strs[start + 2].c_str(), in_strs[start + 3].c_str());
        return 0;
    }
    if (in_strs.size() - start == 5)
    {
        out_val = pDb->GetOut<T>(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                                 in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                                 in_strs[start + 4].c_str());
        return 0;
    }
    AMILOG_ERR << "Not Support Current Para's Count." << std::endl << PRINT_COLOR_END;
    return -1;
}
static int amigos_set_val(AmigosDatabase *pDb, const std::vector<std::string> &in_strs, int start)
{
    if (!pDb)
    {
        return -1;
    }
    if (!in_strs.size())
    {
        return -1;
    }
    const std::string &set_val = in_strs[in_strs.size() - 1];
    if (in_strs.size() - start - 1 == 1)
    {
        pDb->SetMod(in_strs[start].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 2)
    {
        pDb->SetMod(in_strs[start].c_str(), in_strs[start + 1].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 3)
    {
        pDb->SetMod(in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             in_strs[start + 2].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 4)
    {
        pDb->SetMod(in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             in_strs[start + 2].c_str(), in_strs[start + 3].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 5)
    {
        pDb->SetMod(in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                             in_strs[start + 4].c_str(), set_val.c_str());
        return 0;
    }
    AMILOG_ERR << "Not Support Current Para's Count." << std::endl << PRINT_COLOR_END;
    return -1;
}
static int amigos_set_in_val(AmigosDatabase *pDb, unsigned int loopId, const std::vector<std::string> &in_strs, int start)
{
    if (!pDb)
    {
        return -1;
    }
    if (!in_strs.size())
    {
        return -1;
    }
    const std::string &set_val = in_strs[in_strs.size() - 1];
    if (in_strs.size() - start - 1 == 1)
    {
        pDb->SetIn(loopId, in_strs[start].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 2)
    {
        pDb->SetIn(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 3)
    {
        pDb->SetIn(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             in_strs[start + 2].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 4)
    {
        pDb->SetIn(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                             set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 5)
    {
        pDb->SetIn(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                             in_strs[start + 4].c_str(), set_val.c_str());
        return 0;
    }
    AMILOG_ERR << "Not Support Current Para's Count." << std::endl << PRINT_COLOR_END;
    return -1;
}
static int amigos_set_out_val(AmigosDatabase *pDb, unsigned int loopId, const std::vector<std::string> &in_strs, int start)
{
    if (!pDb)
    {
        return -1;
    }
    if (!in_strs.size())
    {
        return -1;
    }
    const std::string &set_val = in_strs[in_strs.size() - 1];
    if (in_strs.size() - start - 1 == 1)
    {
        pDb->SetOut(loopId, in_strs[start].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 2)
    {
        pDb->SetOut(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                             set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 3)
    {
        pDb->SetOut(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                              in_strs[start + 2].c_str(), set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 4)
    {
        pDb->SetOut(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                              in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                              set_val.c_str());
        return 0;
    }
    if (in_strs.size() - start - 1 == 5)
    {
        pDb->SetOut(loopId, in_strs[start].c_str(), in_strs[start + 1].c_str(),
                              in_strs[start + 2].c_str(), in_strs[start + 3].c_str(),
                              in_strs[start + 4].c_str(), set_val.c_str());
        return 0;
    }
    AMILOG_ERR << "Not Support Current Para's Count." << std::endl << PRINT_COLOR_END;
    return -1;
}
#endif
