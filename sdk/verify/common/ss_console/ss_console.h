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
#ifndef __SS_CONSOLE__
#define __SS_CONSOLE__
#include <string>
#include "json.hpp"

struct ss_console_attr_s
{
    long (*trans_init)(const char *key);
    int (*trans_deinit)(long fd);
    int (*send_cmd)(long fd, const char *cmd, unsigned int size, nlohmann::json &json_out);
    int (*grab_tab_list)(long fd, std::string &str);
};
struct ss_console_json_out_s
{
    const char *out_file;
    const char *case_name;
    int log_opt;
    int do_append_cmds;
    int case_stage;
    int case_stage_range;
};
int ss_console(struct ss_console_attr_s *attr, const char *key);
int ss_command(struct ss_console_attr_s *attr, const char *key, const char *command);
int ss_lua(struct ss_console_attr_s *attr, const char *key, const char *script_file, const struct ss_console_json_out_s *console_json_out);
int ss_python(struct ss_console_attr_s *attr, const char *key, const char *script_file, const struct ss_console_json_out_s *console_json_out);
int ss_shell(struct ss_console_attr_s *attr, const char *key, const char *script_file, const struct ss_console_json_out_s *console_json_out);
int ss_json(struct ss_console_attr_s *attr, const char *key, const char *script_file, const struct ss_console_json_out_s *console_json_out);
int ss_run_json_case(struct ss_console_attr_s *attr, const char *key, const char *report_file, const char *case_path_name, const char *script_path, int stage, int log_opt);
int ss_run_json_case_all(struct ss_console_attr_s *attr, const char *key, const char *report_file, const char *script_path, int stage, int log_opt);
int ss_run_json_case_false(struct ss_console_attr_s *attr, const char *key, const char *report_file, const char *script_path, int stage, int log_opt);
int ss_run_json_case_random(struct ss_console_attr_s *attr, const char *key, const char *report_file, const char *script_path, int stage, int log_opt);
int ss_generate_report_file(const char *script_path, const char *report_file);
#endif
