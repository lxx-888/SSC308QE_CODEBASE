/*
 * json_patch.cpp
 */

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include "json.hpp"

bool parse_param(int argc, char *argv[], std::list<std::string> &in_files, std::list<std::string> &out_files)
{
    if (argc < 2)
    {
        std::cout << "Usage : " << argv[0] << "[file]..." << std::endl;
        return false;
    }
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-o")
        {
            ++i;
            if (i < argc)
            {
                out_files.push_back(argv[i]);
            }
        }
        else
        {
            in_files.push_back(arg);
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    std::list<std::string> in_files;
    std::list<std::string> out_files;
    if (!parse_param(argc, argv, in_files, out_files))
    {
        return -1;
    }
    nlohmann::json json_out;
    for (auto file : in_files)
    {
        std::cout << "in <" << file << "> : ";
        std::ifstream fin;
        fin.open(file);
        if (!fin.is_open())
        {
            std::cout << "open failed" << std::endl;
            continue;
        }
        std::cout << "open success; ";
        nlohmann::json j;
        try
        {
            fin >> j;
            std::cout << "parse success; ";
        }
        catch (nlohmann::detail::parse_error &e)
        {
            std::cout << "parse error" << std::endl;
            continue;
        }
        json_out.merge_patch(j);
        std::cout << "merge success" << std::endl;
    }

    std::cout << json_out.dump(2) << std::endl;

    for (auto file : out_files)
    {
        std::cout << "out <" << file << "> : ";
        std::ofstream fout;
        fout.open(file);
        if (!fout.is_open())
        {
            std::cout << "open failed" << std::endl;
            continue;
        }
        std::cout << "open success; ";
        fout << json_out.dump(4);
        std::cout << "dump success" << std::endl;
    }

    return 0;
}

