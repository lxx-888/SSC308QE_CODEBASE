/*
 * sstar_ut.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */
#if defined(__KERNEL__)
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <sstar_ut.h>

//==============================================================================
//                          For UT Create
//==============================================================================
sstar_ut* sstar_ut_create(const char* name, sstar_ut_init_func* ut_init, sstar_ut_exit_func* ut_exit, void* user_data)
{
    sstar_ut* ut = (sstar_ut*)ut_alloc(sizeof(sstar_ut));

    if (ut == NULL)
    {
        SSTAR_UT_ERR("Unable to alloc memory for sstar_ut.\n");
        return NULL;
    }

    ut->name        = name;
    ut->suite_num   = 0;
    ut->test_num    = 0;
    ut->ut_init     = ut_init;
    ut->ut_exit     = ut_exit;
    ut->first_suite = NULL;
    ut->user_data   = user_data;
    ut->cmd_args    = NULL;
    ut->run_num     = 0;
    ut->pass_num    = 0;
    ut->skip_num    = 0;
    ut->fail_num    = 0;

    return ut;
}

sstar_ut_suite* sstar_ut_add_suite(sstar_ut* ut, const char* name, sstar_ut_suite_init_func* suite_init,
                                   sstar_ut_suite_exit_func* suite_exit, void* user_data)
{
    sstar_ut_suite* temp_suite = NULL;
    sstar_ut_suite* suite      = NULL;

    if (ut == NULL)
    {
        SSTAR_UT_ERR("sstar_ut* ut must not be NULL.\n");
        return NULL;
    }

    suite = (sstar_ut_suite*)ut_alloc(sizeof(sstar_ut_suite));
    if (suite == NULL)
    {
        SSTAR_UT_ERR("Unable to alloc memory for sstar_ut_suite.\n");
        return NULL;
    }

    suite->name       = name;
    suite->id         = ++(ut->suite_num);
    suite->run        = false;
    suite->suite_init = suite_init;
    suite->suite_exit = suite_exit;
    suite->user_data  = user_data;
    suite->this_test  = NULL;
    suite->first_test = NULL;
    suite->next_suite = NULL;
    suite->ut         = ut;

    // append new suite to suite list
    if (ut->first_suite == NULL)
    {
        ut->first_suite = suite;
    }
    else
    {
        temp_suite = ut->first_suite;
        while (temp_suite->next_suite)
            temp_suite = temp_suite->next_suite;
        temp_suite->next_suite = suite;
    }

    return suite;
}

sstar_ut_test* sstar_ut_add_test(sstar_ut_suite* suite, const char* name, sstar_ut_test_func* test_func, bool ignore,
                                 sstar_ut_test* parent, void* user_data)
{
    sstar_ut_test* temp_test = NULL;
    sstar_ut_test* test      = NULL;

    if (suite == NULL)
    {
        SSTAR_UT_ERR("sstar_ut_suite* suite must not be NULL.\n");
        return NULL;
    }

    if (test_func == NULL)
    {
        SSTAR_UT_ERR("sstar_ut_test_func* test_func must not be NULL.\n");
        return NULL;
    }

    test = (sstar_ut_test*)ut_alloc(sizeof(sstar_ut_test));
    if (test == NULL)
    {
        SSTAR_UT_ERR("Unable to alloc memory for sstar_ut_test.\n");
        return NULL;
    }

    test->name      = name;
    test->id        = ++(suite->ut->test_num);
    test->execute   = test_func;
    test->run       = false;
    test->result    = UT_PASS;
    test->ignore    = ignore;
    test->parent    = parent;
    test->suite     = suite;
    test->ut        = suite->ut;
    test->user_data = user_data;
    test->next_test = NULL;

    // append new test to test list for suite
    if (suite->first_test == NULL)
    {
        suite->first_test = test;
    }
    else
    {
        temp_test = suite->first_test;
        while (temp_test->next_test)
            temp_test = temp_test->next_test;
        temp_test->next_test = test;
    }

    return test;
}

//==============================================================================
//                          For UT Free
//==============================================================================
static void sstar_ut_test_free(sstar_ut_test* test)
{
    if (test == NULL)
        return;

    sstar_ut_test_free(test->next_test);
    ut_free(test);
}

static void sstar_ut_suite_free(sstar_ut_suite* suite)
{
    if (suite == NULL)
        return;

    sstar_ut_suite_free(suite->next_suite);
    sstar_ut_test_free(suite->first_test);
    ut_free(suite);
}

void sstar_ut_free(sstar_ut* ut)
{
    if (ut == NULL)
        return;

    sstar_ut_suite_free(ut->first_suite);
    ut_free(ut);
}

//==============================================================================
//                          For UT Run Prepare
//==============================================================================
static void sstar_ut_reset(sstar_ut* ut)
{
    sstar_ut_test*  test  = NULL;
    sstar_ut_suite* suite = NULL;

    ut->cmd_args = NULL;
    ut->run_num  = 0;
    ut->pass_num = 0;
    ut->skip_num = 0;
    ut->fail_num = 0;

    for_each_suite(suite, ut->first_suite)
    {
        suite->run = false;
        for_each_test(test, suite->first_test)
        {
            test->run    = false;
            test->result = UT_PASS;
        }
    }
}

static void sstar_ut_run_test(sstar_ut* ut, const char* test_name, int id)
{
    sstar_ut_test*  test  = NULL;
    sstar_ut_suite* suite = NULL;

    for_each_suite(suite, ut->first_suite)
    {
        for_each_test(test, suite->first_test)
        {
            if ((test->id == id) || (test_name && (strcmp(test->name, test_name) == 0)))
            {
                test->run  = true;
                suite->run = true;
                return;
            }
        }
    }
}

#define sstar_ut_run_test_by_name(ut, name) sstar_ut_run_test(ut, name, 0)
#define sstar_ut_run_test_by_id(ut, id)     sstar_ut_run_test(ut, NULL, id)

static void sstar_ut_run_suite(sstar_ut* ut, const char* suite_name)
{
    sstar_ut_test*  test  = NULL;
    sstar_ut_suite* suite = NULL;

    for_each_suite(suite, ut->first_suite)
    {
        if (suite_name && strcmp(suite->name, suite_name) != 0)
            continue;
        for_each_test(test, suite->first_test)
        {
            test->run = !(test->ignore);
            suite->run |= test->run;
        }
    }
}

#define sstar_ut_run_all(ut) sstar_ut_run_suite(ut, NULL)

//==============================================================================
//                          For UT Run
//==============================================================================
static bool sstar_ut_will_skip(sstar_ut_test* test)
{
    sstar_ut_test* parent = test->parent;

    while (parent)
    {
        if (!parent->result)
            return true;
        parent = parent->parent;
    }

    return false;
}

static void sstar_ut_real_run(sstar_ut* ut)
{
    sstar_ut_test*  test  = NULL;
    sstar_ut_suite* suite = NULL;

    WRAP_SEPARATOR("                     SStar %s Unit Tests\n", ut->name);

    SSTAR_RUN_FUNC(ut, ut->ut_init);

    for_each_suite(suite, ut->first_suite)
    {
        if (!suite->run)
            continue;

        WRAP_SEPARATOR("[%s UT][SUITE] %s\n", ut->name, suite->name);

        SSTAR_RUN_FUNC(suite, suite->suite_init);

        for_each_test(test, suite->first_test)
        {
            if (!test->run)
                continue;

            suite->this_test = test;
            ++(ut->run_num);

            if (sstar_ut_will_skip(test))
            {
                UT("\033[33m[Skip] %s\033[0m\n", test->name);
                test->result = UT_FAIL;
                ++(ut->skip_num);
            }
            else
            {
                UT("[Init] %s\n", test->name);
                if (test->execute(test))
                {
                    UT("\033[32m[Pass] %s\033[0m\n", test->name);
                    test->result = UT_PASS;
                    ++(ut->pass_num);
                }
                else
                {
                    UT("\033[31m[Fail] %s\033[0m\n", test->name);
                    test->result = UT_FAIL;
                    ++(ut->fail_num);
                }
            }
        }

        SSTAR_RUN_FUNC(suite, suite->suite_exit);
    }

    SSTAR_RUN_FUNC(ut, ut->ut_exit);

    WRAP_SEPARATOR("[%s UT] Result Summary\n", ut->name);
    UT("    Total test:  %d\n", ut->run_num);
    UT("     \033[32mPass test:  %d\033[0m\n", ut->pass_num);
    UT("     \033[33mSkip test:  %d\033[0m\n", ut->skip_num);
    UT("     \033[31mFail test:  %d\033[0m\n", ut->fail_num);
    SEPARATOR();
}

static void sstar_ut_show_tests(sstar_ut* ut)
{
    sstar_ut_test*  test  = NULL;
    sstar_ut_suite* suite = NULL;

    WRAP_SEPARATOR("                     SStar %s Unit Tests\n", ut->name);
    for_each_suite(suite, ut->first_suite)
    {
        WRAP_SEPARATOR("[%s UT] SUITE %s\n", ut->name, suite->name);
        for_each_test(test, suite->first_test)
        {
            if (test->ignore)
                UT("ID %2d  TEST %-50s\033[31m[ignore]\033[0m\n", test->id, test->name);
            else
                UT("ID %2d  TEST %-50s\n", test->id, test->name);
        }
    }
}

#if defined(KERNEL_SPACE_UT)
static void sstar_ut_show_usage(void)
{
    SEPARATOR();
    UT("Uasge:\n");
    UT("  echo ALL [cmd_args]");
    UT("  echo SUITE [suite_name] [cmd_args]");
    UT("  echo TEST [test_name] [cmd_args]");
    UT("  echo ID [id] [id] [id] ....");
    UT("\nNote: [ignore] test can only be run by TEST or ID.\n");
    SEPARATOR();
}

ssize_t sstar_ut_run(sstar_ut* ut, const char* buf, size_t count)
{
    int  n;
    int  offset;
    int  test_id;
    char type[MAX_TYPE_LENGTH]         = {0};
    char name[MAX_NAME_LENGTH]         = {0};
    char cmd_args[MAX_CMD_ARGS_LENGTH] = {0};

    sstar_ut_reset(ut);

    sscanf(buf, "%s", type);
    offset = strlen(type) + 1;

    if (strcmp(type, "ALL") == 0)
    {
        sscanf(buf + offset, "%s", cmd_args);
        sstar_ut_run_all(ut);
    }
    else if (strcmp(type, "SUITE") == 0)
    {
        sscanf(buf + offset, "%s %s", name, cmd_args);
        sstar_ut_run_suite(ut, name);
    }
    else if (strcmp(type, "TEST") == 0)
    {
        sscanf(buf + offset, "%s %s", name, cmd_args);
        sstar_ut_run_test_by_name(ut, name);
    }
    else if (strcmp(type, "ID") == 0)
    {
        while (offset < count)
        {
            sscanf(buf + offset, "%d %n", &test_id, &n);
            sstar_ut_run_test_by_id(ut, test_id);
            offset += n;
        }
    }
    else
    {
        sstar_ut_show_usage();
        return count;
    }

    if (cmd_args[0] != '\0')
    {
        cmd_args[MAX_CMD_ARGS_LENGTH - 1] = '\0';
        ut->cmd_args                      = cmd_args;
    }

    sstar_ut_real_run(ut);

    return count;
}

ssize_t sstar_ut_show(sstar_ut* ut, char* buf)
{
    sstar_ut_show_tests(ut);
    sstar_ut_show_usage();

    return sprintf(buf, "%u %u %u %u\n", ut->run_num, ut->pass_num, ut->skip_num, ut->fail_num);
}

#elif defined(USER_SPACE_UT)

static void sstar_ut_show_usage(char* name)
{
    SEPARATOR();
    UT("Uasge:\n");
    UT("  ./%s ALL [cmd_args]\n", name);
    UT("  ./%s SUITE [suite_name] [cmd_args]\n", name);
    UT("  ./%s TEST [test_name] [cmd_args]\n", name);
    UT("  ./%s ID [id] [id] [id] ....\n", name);
    UT("\nNote: [ignore] test can only be run by TEST or ID.\n");
    SEPARATOR();
}

unsigned int sstar_ut_run(sstar_ut* ut, int argc, char** argv)
{
    int   i = 1;
    int   test_id;
    char* type;
    char

        if (argc == 1)
    {
        sstar_ut_show(argv[0]);
        return 1;
    }

    type = argv[i++];

    if (strcmp(type, "ID") == 0)
    {
        for (i = 2; i < argc; ++i)
        {
            sscanf(argv + i, "%d", &test_id);
            sstar_ut_run_test_by_id(ut, test_id);
        }
    }
    else
    {
        if (strcmp(type, "ALL") == 0)
        {
            sstar_ut_run_all(ut);
        }
        else if (strcmp(type, "SUITE") == 0)
        {
            sstar_ut_run_suite(ut, argv[i++]);
        }
        else if (strcmp(type, "TEST") == 0)
        {
            sstar_ut_run_test_by_name(ut, argv[i++]);
        }
        else
        {
            sstar_ut_show_usage(argv[0]);
            return 0;
        }
    }

    if (argc == i)
    {
        sscanf(argv[i], "%s", cmd_args);
        cmd_args[MAX_CMD_ARGS_LENGTH - 1] = '\0';
        ut->cmd_args                      = cmd_args;
    }

    sstar_ut_real_run(ut);

    return 0;
}

unsigned int sstar_ut_show(sstar_ut* ut, char* name)
{
    sstar_ut_show_tests(ut);
    sstar_ut_show_usage(name);

    return 0;
}
#endif

#if defined(KERNEL_SPACE_UT)
EXPORT_SYMBOL(sstar_ut_create);
EXPORT_SYMBOL(sstar_ut_free);
EXPORT_SYMBOL(sstar_ut_add_suite);
EXPORT_SYMBOL(sstar_ut_add_test);
EXPORT_SYMBOL(sstar_ut_show);
EXPORT_SYMBOL(sstar_ut_run);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIGMASTAR");
MODULE_DESCRIPTION("SSTAR UT Framework");
#endif
