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
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

static std::string merge_out_info(const nlohmann::ordered_json &json_value, const std::string &out_info)
{
    std::string str_tmp = json_value.at("OUT_CNT");
    unsigned int cnt = atoi(str_tmp.c_str());
    for (unsigned int i = 0, j = 0; i < 32 && j < cnt; i++)
    {
        std::stringstream ss;
        ss << "OUT_" << i;
        if (ss.str() == out_info)
        {
            ss.str("");
            ss << "OUT_" << j;
            return ss.str();
        }
        if (json_value.find(ss.str()) != json_value.end())
        {
            j++;
        }
    }
    return "";
}
static std::string merge_key(const nlohmann::ordered_json &json_value)
{
    if (json_value.find("MOD") == json_value.end())
    {
        return "";
    }
    std::stringstream ss;
    std::string out_str = json_value.at("MOD");
    ss << out_str << '_';
    out_str = json_value.at("DEV");
    ss << out_str << '_';
    out_str = json_value.at("CHN");
    ss << out_str << "_0";
    return ss.str();
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "USAGE: " << argv[0] << "input.json " << "output.json" << std::endl;
        return -1;
    }
    std::ifstream fin(argv[1]);
    if (!fin.is_open())
    {
        std::cout << "OPEN " << argv[1] << " ERROR!" << std::endl;
        return -1;
    }
    nlohmann::ordered_json json_data;
    nlohmann::ordered_json json_merge;
    nlohmann::ordered_json json_out;
    fin >> json_data;
    json_merge = json_data;
    for (auto it = json_merge.begin(); it != json_merge.end(); it++)
    {
        std::string out_key;
        if (it.key() == "ROOT")
        {
            json_out["ROOT"] = it.value();
            for (auto it_root = json_out.at("ROOT").begin(); it_root != json_out.at("ROOT").end(); it_root++)
            {
                std::string root_name = (*it_root)["NAME"];
                nlohmann::ordered_json::iterator it_root_key = json_merge.find(root_name);
                if (it_root_key == json_merge.end())
                {
                    std::cout << "Can not find key " << root_name << std::endl;
                    return -1;
                }
                (*it_root)["NAME"] = merge_key(*it_root_key);
            }
            continue;
        }
        std::string out_str = merge_key(it.value());
        if (!out_str.size())
        {
            std::cout << it.key() << " is not a pipeline section's key, skip it."<< std::endl;
            continue;
        }
        std::string str_tmp = it.value().at("IN_CNT");
        unsigned int cnt = atoi(str_tmp.c_str());
        for (unsigned int i = 0, j = 0; i < 32 && j < cnt; i++)
        {
            std::stringstream ss;
            ss << "IN_" << i;
            nlohmann::ordered_json::iterator it_in = it.value().find(ss.str());
            if (it_in == it.value().end())
            {
                continue;
            }
            std::string prev_str = it_in->at("PREV");
            size_t off = prev_str.find_last_of(':');
            if (off == std::string::npos)
            {
                continue;
            }
            std::string out_info = prev_str.c_str() + off + 1;
            prev_str.erase(off, prev_str.size() - off);
            out_info = merge_out_info(json_data.at(prev_str), out_info);
            if (!out_info.size())
            {
                std::cout << "Not found loop out id. Key: " << it.key() << '.' << std::endl;
                return -1;
            }
            prev_str = merge_key(json_data.at(prev_str));
            if (!prev_str.size())
            {
                std::cout << "Can not find key " << prev_str << '.' << std::endl;
                return -1;
            }
            ss.str("");
            ss << prev_str << ':' << out_info;
            it_in->at("PREV") = ss.str();
            if (j != i)
            {
                std::stringstream ss_merge_dst;
                ss_merge_dst << "IN_" << j;
                it.value()[ss_merge_dst.str()] = it_in.value();
                std::cout << "merge IN_" << i << " to " << "IN_" << j << std::endl;
                it.value().erase(ss.str());
            }
            j++;
        }
        str_tmp = it.value().at("OUT_CNT");
        cnt = atoi(str_tmp.c_str());
        for (unsigned int i = 0, j = 0; i < 32 && j < cnt; i++)
        {
            std::stringstream ss;
            ss << "OUT_" << i;
            nlohmann::ordered_json::iterator it_out = it.value().find(ss.str());
            if (it_out == it.value().end())
            {
                continue;
            }
            if (j != i)
            {
                std::stringstream ss_merge_dst;
                ss_merge_dst << "OUT_" << j;
                it.value()[ss_merge_dst.str()] = it_out.value();
                std::cout << "merge " << "KEY "<< it.key() << " OUT_" << i << " to " << "OUT_" << j << std::endl;
                it.value().erase(ss.str());
            }
            j++;
        }
        if (it.value().at("MOD") == "RGN")
        {
            str_tmp = it.value().at("ATTACH_COUNT");
            cnt = atoi(str_tmp.c_str());
            for (unsigned int i = 0; i < cnt; i++)
            {
                std::stringstream ss;
                ss << "ATTACH_" << i;
                nlohmann::ordered_json::iterator it_attach = it.value().find(ss.str());
                if (it_attach == it.value().end())
                {
                    std::cout << "Can not find " << ss.str() << std::endl;
                    return -1;
                }
                str_tmp = it_attach->at("MODULE");
                it_attach->at("MODULE") = merge_key(json_data.at(str_tmp));
                std::cout << "Refine " << "RGN " << "KEY 'MODULE' from " << str_tmp << " to be " << it_attach->at("MODULE") << std::endl;
            }
        }
        else if (it.value().at("MOD") == "ENV_MONITOR")
        {
            str_tmp = it.value().at("OUT_CNT");
            cnt = atoi(str_tmp.c_str());
            for (unsigned int i = 0; i < cnt; i++)
            {
                std::stringstream ss;
                ss << "OUT_" << i;
                nlohmann::ordered_json::iterator it_out = it.value().find(ss.str());
                if (it_out == it.value().end())
                {
                    std::cout << "Can not find " << ss.str() << std::endl;
                    return -1;
                }
                str_tmp = it_out->at("MODULE");
                it_out->at("MODULE") = merge_key(json_data.at(str_tmp));
                std::cout << "Refine " << "ENV_MONITOR " << "KEY 'MODULE' from " << str_tmp << " to be " << it_out->at("MODULE") << std::endl;
            }
        }
        json_out[out_str] = it.value();
    }
    std::ofstream fout(argv[2]);
    if (!fout.is_open())
    {
        std::cout << "Output file " << argv[2] << " open fail!" << std::endl;
        return -1;
    }
    fout << json_out.dump(4);
    std::cout << "Write file " << argv[2] << " done." << std::endl;
    return 0;
}

