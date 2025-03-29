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
#include <assert.h>
#include <string>
#include <iostream>
#ifdef CONFIG_DB_INI
#include "amigos_database_ini.h"
#endif
#ifdef CONFIG_DB_JSON
#include "amigos_database_json.h"
#endif
#include "ss_handle.h"
#include "ss_cmd_base.h"
#include "amigos_database_io.hpp"
#include "amigos_database_factory.h"

template <class T>
map<string, T *> ss_handle_template<T>::handle_map;
AmigosDatabaseFactory *gp_databaseFactory = NULL;

#ifdef CONFIG_DB_INI
static int amidb_load_ini_file(vector<string> &in_strs)
{
    size_t findOffset = in_strs[1].find_last_of('.');
    if (findOffset == string::npos
        || in_strs[1].size() <= 4
        || in_strs[1].compare(findOffset, 4, ".ini"))
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "FILE: " << in_strs[1] << " Name error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    gp_databaseFactory = new AmigosDatabaseFactory(in_strs[1]);
    assert(gp_databaseFactory);
    return gp_databaseFactory->LoadFile();
}
#endif
#ifdef CONFIG_DB_JSON
static int amidb_load_json_file(vector<string> &in_strs)
{
    size_t findOffset = in_strs[1].find_last_of('.');
    if (findOffset == string::npos
        || in_strs[1].size() <= 5
        || in_strs[1].compare(findOffset, 5, ".json"))
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "FILE: " << in_strs[1] << " Name error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    gp_databaseFactory = new AmigosDatabaseFactory(in_strs[1]);
    assert(gp_databaseFactory);
    return gp_databaseFactory->LoadFile();
}
static int amidb_dump_json(vector<string> &in_strs)
{
    auto iter = AmigosDatabaseJson::Json.find(in_strs[1]);
    if (iter == AmigosDatabaseJson::Json.end())
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Section: " << in_strs[1] << " error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    sslog << iter->dump(4) << endl;
    return 0;
}
#endif
static int amidb_create_section(vector<string> &in_strs)
{
    AmigosDatabase *ins = gp_databaseFactory->Create(in_strs[1]);
    if (!ins)
    {
        return -1;
    }
    AmigosDatabase *ret = ss_handle_template<AmigosDatabase>::install(in_strs[1], ins);
    if (!ret)
    {
        delete ins;
        return -1;
    }
    return 0;
}
static int amidb_unload_file(vector<string> &in_strs)
{
    if (!gp_databaseFactory)
    {
        return -1;
    }
    gp_databaseFactory->UnloadFile();
    delete gp_databaseFactory;
    return 0;
}
static int amidb_get_mod_val(vector<string> &in_strs)
{
    AmigosDatabase *pDb = ss_handle_template<AmigosDatabase>::get(in_strs[1]);
    if (!pDb)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Section: " << in_strs[1] << " Database Obj error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    if (in_strs[2] == "str")
    {
        string getStr;
        if (amigos_get_val(pDb, in_strs, 3, getStr) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET STR: " << getStr << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[3].c_str()] = getStr;
        return 0;
    }
    if (in_strs[2] == "uint")
    {
        unsigned int getUint;
        if (amigos_get_val(pDb, in_strs, 3, getUint) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET UINT: " << getUint << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[3].c_str()] = getUint;
        return 0;
    }
    if (in_strs[2] == "int")
    {
        int getInt;
        if (amigos_get_val(pDb, in_strs, 3, getInt) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET INT: " << getInt << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[3].c_str()] = getInt;
        return 0;
    }
    sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
        << "VAL TYPE " << in_strs[2] << " ERR." << endl << PRINT_COLOR_END;
    return -1;
}
static int amidb_get_in_val(vector<string> &in_strs)
{
    unsigned int uintId = ss_cmd_atoi(in_strs[2].c_str());
    AmigosDatabase *pDb = ss_handle_template<AmigosDatabase>::get(in_strs[1]);
    if (!pDb)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Section: " << in_strs[1] << " Database Obj error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    if (in_strs[3] == "str")
    {
        string getStr;
        if (amigos_get_in_val(pDb, uintId, in_strs, 4, getStr) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET STR: " << getStr << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[4].c_str()] = getStr;
        return 0;
    }
    if (in_strs[3] == "uint")
    {
        unsigned int getUint;
        if (amigos_get_in_val(pDb, uintId, in_strs, 4, getUint) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET UINT: "<< getUint << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[4].c_str()] = getUint;
        return 0;
    }
    if (in_strs[3] == "int")
    {
        int getInt;
        if (amigos_get_in_val(pDb, uintId, in_strs, 4, getInt) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET INT: " << getInt << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[4].c_str()] = getInt;
        return 0;
    }
    sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
        << "VAL TYPE " << in_strs[3] << " ERR." << endl << PRINT_COLOR_END;
    return -1;
}
static int amidb_get_out_val(vector<string> &in_strs)
{
    unsigned int uintId = ss_cmd_atoi(in_strs[2].c_str());
    AmigosDatabase *pDb = ss_handle_template<AmigosDatabase>::get(in_strs[1]);
    if (!pDb)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Section: " << in_strs[1] << " Database Obj error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    if (in_strs[3] == "str")
    {
        string getStr;
        if (amigos_get_out_val(pDb, uintId, in_strs, 4, getStr) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET STR: " << getStr << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[4].c_str()] = getStr;
        return 0;
    }
    if (in_strs[3] == "uint")
    {
        unsigned int getUint;
        if (amigos_get_out_val(pDb, uintId, in_strs, 4, getUint) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET UINT: " << getUint << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[4].c_str()] = getUint;
        return 0;
    }
    if (in_strs[3] == "int")
    {
        int getInt;
        if (amigos_get_out_val(pDb, uintId, in_strs, 4, getInt) == -1)
        {
            return -1;
        }
        sslog.color_set(PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT)
            << "GET INT: " << getInt << endl << PRINT_COLOR_END;
        sslog.store_msg()[in_strs[4].c_str()] = getInt;
        return 0;
    }
    sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
        << "VAL TYPE " << in_strs[3] << " ERR." << endl << PRINT_COLOR_END;
    return -1;
}
static int amidb_set_mod_val(vector<string> &in_strs)
{
    AmigosDatabase *pDb = ss_handle_template<AmigosDatabase>::get(in_strs[1]);
    if (!pDb)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Section: " << in_strs[1] << " Database Obj error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    return amigos_set_val(pDb, in_strs, 2);
}
static int amidb_set_in_val(vector<string> &in_strs)
{
    unsigned int uintId = ss_cmd_atoi(in_strs[2].c_str());
    AmigosDatabase *pDb = ss_handle_template<AmigosDatabase>::get(in_strs[1]);
    if (!pDb)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Section: " << in_strs[1] << " Database Obj error!" << endl << PRINT_COLOR_END;
    }
    return amigos_set_in_val(pDb, uintId, in_strs, 3);
}
static int amidb_set_out_val(vector<string> &in_strs)
{
    unsigned int uintId = ss_cmd_atoi(in_strs[2].c_str());
    AmigosDatabase *pDb = ss_handle_template<AmigosDatabase>::get(in_strs[1]);
    if (!pDb)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT)
            << "Section: " << in_strs[1] << " Database Obj error!" << endl << PRINT_COLOR_END;
        return -1;
    }
    return amigos_set_out_val(pDb, uintId, in_strs, 3);
}
static int amidb_destroy(vector<string> &in_strs)
{
    int ret = ss_handle_template<AmigosDatabase>::destroy(in_strs[1]);
    return ret;
}

MOD_CMDS(amidb_ut) {
#ifdef CONFIG_DB_INI
    ADD_CMD("amidb_load_ini_file", amidb_load_ini_file, 1);
    ADD_CMD_HELP("amidb_load_ini_file", "[ini_file]", "Load ini file.");
    ADD_CMD("amidb_create_ini_section", amidb_create_section, 1);
    ADD_CMD_HELP("amidb_create_ini_section", "[section]", "Create ini section object after load.");
#endif
#ifdef CONFIG_DB_JSON
    ADD_CMD("amidb_load_json_file", amidb_load_json_file, 1);
    ADD_CMD_HELP("amidb_load_json_file", "[json_file]", "Load json file.");
    ADD_CMD("amidb_create_json_section", amidb_create_section, 1);
    ADD_CMD_HELP("amidb_create_json_section", "[section]", "Create json section object after load.");
    ADD_CMD("amidb_dump_json", amidb_dump_json, 1);
    ADD_CMD_HELP("amidb_dump_json", "[section]", "Dump JSON data in section.");
#endif
    ADD_CMD("amidb_destroy", amidb_destroy, 1);
    ADD_CMD_HELP("amidb_destroy", "[section]", "Destroy amigod database section object.");
    ADD_CMD("amidb_unload_file", amidb_unload_file, 0);
    ADD_CMD_HELP("amidb_unload_file", "No argument.", "Unload ini file.");

    ADD_CMD_VAR_ARG("amidb_get_mod_val", amidb_get_mod_val, 3);
    ADD_CMD_HELP("amidb_get_mod_val", "[section] [value_type] [key] [...]",
                 "Get module self's parameter from section.",
                 "[section]    : Section name from the target database file.",
                 "[value_type] : Get value type, 'str','uint','int'.",
                 "[key]        : The value's key."
                 "[...]        : Append keys.");
    ADD_CMD_VAR_ARG("amidb_get_in_val", amidb_get_in_val, 4);
    ADD_CMD_HELP("amidb_get_in_val", "[section] [port id] [value_type] [key] [...]",
                 "Get module input's parameter from section.",
                 "[section]    : Section name from the target database file.",
                 "[port id]    : Input port id.",
                 "[value_type] : Get value type, 'str','uint','int'.",
                 "[key]        : The value's key.",
                 "[...]        : Append keys.");
    ADD_CMD_VAR_ARG("amidb_get_out_val", amidb_get_out_val, 4);
    ADD_CMD_HELP("amidb_get_out_val", "[section] [port id] [value_type] [key] [...]",
                 "Get module output's parameter from section.",
                 "[section]    : Section name from the target database file.",
                 "[port id]    : Output port id.",
                 "[value_type] : Get value type, 'str','uint','int'.",
                 "[key]        : The value's key.",
                 "[...]        : Append keys.");
    ADD_CMD_VAR_ARG("amidb_set_mod_val", amidb_set_mod_val, 3);
    ADD_CMD_HELP("amidb_set_mod_val", "[section] [key] [...] [set_val]", "Set section key's value");
    ADD_CMD_VAR_ARG("amidb_set_in_val", amidb_set_in_val, 4);
    ADD_CMD_HELP("amidb_set_in_val", "[section] [loop id] [key] [...] [set_val]", "Set section input key's value");
    ADD_CMD_VAR_ARG("amidb_set_out_val", amidb_set_out_val, 4);
    ADD_CMD_HELP("amidb_set_out_val", "[section] [loop id] [key] [...] [set_val]", "Set section output key's value");
}
