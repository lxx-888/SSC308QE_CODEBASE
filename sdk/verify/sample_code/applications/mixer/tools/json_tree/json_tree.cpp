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
#include <sstream>
#include <fstream>
#include <map>
#include <list>
#include <string>
#include <set>
#include "json.hpp"

using namespace std;

class json_tree
{
public:
    struct loop_pos
    {
        unsigned int max_depth;
        unsigned int x;
        unsigned int y;
    };
    struct obj_pos
    {
        set<string > prev;
        set<string > root;
        unsigned int weight;
        unsigned int x;
        unsigned int y;
    };
    explicit json_tree(const nlohmann::json &json_data, unsigned int w, unsigned int h)
                        : data(json_data)
    {
        const nlohmann::json data_root = data.at("ROOT");
        set<string> set_root;
        map<string, int> sec_ref;
        cur_pos.x         = 0;
        cur_pos.y         = 0;
        cur_pos.max_depth = 0;
        for (auto it = data_root.begin(); it != data_root.end(); ++it)
        {
            string name  = it->at("NAME");
            string delay = it->at("DELAY");
            int is_delay = ex_atoi(delay.c_str());
            if (is_delay)
            {
                delay_root.push_back(name);
            }
            else
            {
                pipeline_root.push_back(name);
            }
            set_root.insert(name);
        }
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            if (it.key() == "ROOT")
            {
                continue;
            }
            string str_cnt = it->at("IN_CNT");
            int cnt = ex_atoi(str_cnt.c_str());
            for (int i = 0; i < cnt; i++)
            {
                std::stringstream ss;
                ss << "IN_" << i;
                const nlohmann::json &in_sec =  it->at(ss.str());
                string prev_sec = in_sec["PREV"];
                size_t find_off = prev_sec.find_last_of(':');
                if (find_off != string::npos)
                {
                    prev_sec.erase(find_off, prev_sec.size() - find_off);
                    sec_ref[prev_sec]++;
                }
            }
            sec_ref[it.key()]++;
        }
        for (auto &it : sec_ref)
        {
            if (it.second == 1 && set_root.find(it.first) == set_root.end())
            {
                delay_root.push_back(it.first);
            }
        }
        for (auto &it : pipeline_root)
        {
            const nlohmann::json &value = data.at(it);
            previous_section(data, it, value, pipeline_obj, it);
        }
        for (auto &it : delay_root)
        {
            const nlohmann::json &value = data.at(it);
            previous_section(data, it, value, delay_obj, it);
        }
        for (auto &it : pipeline_obj)
        {
            it.second.x = cur_pos.max_depth - it.second.x;
            it.second.x = it.second.x * w + 200;
            it.second.y = it.second.y * h + 200;
        }
        for (auto &it : delay_obj)
        {
            it.second.x = cur_pos.max_depth - it.second.x;
            it.second.x = it.second.x * w + 200;
            it.second.y = it.second.y * h + 200;
        }
    }
    virtual ~json_tree()
    {
    }
    list<string> &get_pipeline_root()
    {
        return pipeline_root;
    }
    list<string> &get_delay_root()
    {
        return delay_root;
    }
    map<string, struct obj_pos> &get_pipiline_objects()
    {
        return pipeline_obj;
    }
    map<string, struct obj_pos> &get_delay_objects()
    {
        return delay_obj;
    }
private:
    void sort_depth(struct obj_pos &obj, map<string, struct obj_pos> &objects, const string &root)
    {
        obj.weight++;
        obj.x = ++cur_pos.x;
        obj.root.insert(root);
        for (auto &it : obj.prev)
        {
            sort_depth(objects[it], objects, root);
        }
        --cur_pos.x;
    }
    void add_weight(struct obj_pos &obj, map<string, struct obj_pos> &objects, const string &root)
    {
        obj.weight++;
        obj.root.insert(root);
        for (auto &it : obj.prev)
        {
            add_weight(objects[it], objects, root);
        }
    }
    int previous_section(const nlohmann::json &json_data, const string &key, const nlohmann::json &section,
                         map<string, struct obj_pos> &objects, const string &root)
    {
        int ret = 0;
        objects[key].x      = cur_pos.x;
        objects[key].y      = cur_pos.y;
        objects[key].weight = 1;
        objects[key].root.insert(root);
        if (cur_pos.x > cur_pos.max_depth)
        {
            cur_pos.max_depth = cur_pos.x;
        }
        string str_cnt = section.at("IN_CNT");
        int cnt = ex_atoi(str_cnt.c_str());
        if (cnt == 0)
        {
            cur_pos.y++;
            return 0;
        }
        for (int i = 0; i < cnt; i++)
        {
            std::stringstream ss;
            ss << "IN_" << i;
            const nlohmann::json &in_sec =  section.at(ss.str());
            string prev_sec = in_sec["PREV"];
            size_t find_off = prev_sec.find_last_of(':');
            if (find_off != string::npos)
            {
                prev_sec.erase(find_off, prev_sec.size() - find_off);
                objects[key].prev.insert(prev_sec);
                if (objects.find(prev_sec) == objects.end())
                {
                    auto iter = json_data.find(prev_sec);
                    if (iter == json_data.end())
                    {
                        cout << "Can not find " << prev_sec << endl;
                        ret = -1;
                        break;
                    }
                    ++cur_pos.x;
                    ret = previous_section(json_data, iter.key(), iter.value(), objects, root);
                    if (ret != 0)
                    {
                        break;
                    }
                    --cur_pos.x;
                    continue;
                }
                if (objects[prev_sec].x <= cur_pos.x)
                {
                    objects[prev_sec].x = cur_pos.x;
                    sort_depth(objects[prev_sec], objects, root);
                }
                else
                {
                    add_weight(objects[prev_sec], objects, root);
                }
            }
            if (objects[key].y == cur_pos.y)
            {
                ++cur_pos.y;
            }
        }
        return ret;
    }
    long ex_atoi(const char *str)
    {
        if (!str)
            return -1;

        if (strlen(str) >= 3
            && str[0] == '0'
            && (str[1] == 'x' || str[1] == 'X'))
        {
            long ret = 0;
            sscanf(str, "%lx", &ret);
            return ret;
        }
        return atol(str);
    }
    const nlohmann::json &data;
    list<string> pipeline_root;
    list<string> delay_root;
    map<string, struct obj_pos> pipeline_obj;
    map<string, struct obj_pos> delay_obj;
    struct loop_pos cur_pos;
};

int main(int argc, char **argv)
{
    nlohmann::json json_data;
    if (argc != 3)
    {
        cout << "USAGE: " << argv[0] << " [in_json] [out_json]" << endl;
        return -1;
    }
    ifstream fin(argv[1]);
    if (!fin.is_open())
    {
        cout << "Open file " << argv[1] << " failed!" << endl;
        return -1;
    }
    fin >> json_data;
    json_tree *tree= new json_tree(json_data, 200, 100);
    if (!tree)
    {
        cout << "Alloc error!" << endl;
        return -1;
    }
    cout << "pipeline root: " << endl;
    for (auto &it : tree->get_pipeline_root())
    {
        cout << it << endl;
    }
    cout << "delay root: " << endl;
    for (auto &it : tree->get_delay_root())
    {
        cout << it << endl;
    }
    for (auto &it : tree->get_pipiline_objects())
    {
        stringstream ss;
        ss << it.second.x;
        json_data[it.first]["POS_X"] = ss.str();
        ss.str("");
        ss << it.second.y;
        json_data[it.first]["POS_Y"] = ss.str();
        cout << it.first << ",X:" << it.second.x << ",Y:" << it.second.y << ",W:" << it.second.weight << endl;
    }
    for (auto &it : tree->get_delay_objects())
    {
        stringstream ss;
        ss << it.second.x;
        json_data[it.first]["POS_X"] = ss.str();
        ss.str("");
        ss << it.second.y;
        json_data[it.first]["POS_Y"] = ss.str();
        cout << it.first << ",X:" << it.second.x << ",Y:" << it.second.y << ",W:" << it.second.weight << endl;
    }
    std::ofstream fout(argv[2]);
    fout << json_data.dump(4);
    delete tree;
    return 0;
}

