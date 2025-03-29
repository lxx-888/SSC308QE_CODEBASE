/*
 * sstar_ut.h - Sigmastar
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

#ifndef __SSTAR_UT_H__
#define __SSTAR_UT_H__

#if defined(__KERNEL__)
#define KERNEL_SPACE_UT
#else
#define USER_SPACE_UT
#endif

#include "mdrv_types.h"

// =============================================================================
// Usage:
// 1. In your module_init(), Follow the steps below to create UT:
//   1.1 Create a new UT by sstar_ut_create()
//   1.2 Create and add a suite to UT by sstar_ut_add_suite()
//   1.3 Create and add a test by sstar_ut_add_test(), you can repeat many times
//   1.4 You can repeat step 1.2 and step 1.3 many times if needed
// 2. Use sstar_ut_read() and sstar_ut_write() in your module_read/write()
// 3. Use sstar_ut_free() int your module_exit() to free UT
//
// The UT will be organized as follows:
// ┌───────┐
// │  UT   │
// └───┬───┘
//     │
// ┌───▼───┐   ┌───────┐   ┌───────┐
// │ suite ├──►│ suite ├──►│ suite │
// └───┬───┘   └───┬───┘   └───┬───┘
//     │           │           │
// ┌───▼───┐   ┌───▼───┐   ┌───▼───┐
// │ test  │   │ test  │   │ test  │
// └───┬───┘   └───┬───┘   └───┬───┘
//     │           │           │
// ┌───▼───┐   ┌───▼───┐   ┌───▼───┐
// │ test  │   │ test  │   │ test  │
// └───────┘   └───────┘   └───────┘
// =============================================================================

#ifndef bool
#define bool unsigned char
#define true 1
#define false 0
#endif

#define UT_RESULT bool
#define UT_PASS   true
#define UT_FAIL   false

#define MAX_TYPE_LENGTH     (10)
#define MAX_NAME_LENGTH     (100)
#define MAX_CMD_ARGS_LENGTH (100)

#if defined(KERNEL_SPACE_UT)
#define UT(fmt, args...) printk(KERN_EMERG fmt, ##args)

#define SSTAR_UT_ERR(fmt, args...) \
    printk(KERN_ERR "\033[31m[UT ERR]\033[0m [%s():line %d] " fmt, __FUNCTION__, __LINE__, ##args)

#define SSTAR_UT_INFO(fmt, args...) printk(KERN_INFO "[UT INFO] [%s():line %d] " fmt, __FUNCTION__, __LINE__, ##args)

#define ut_alloc(size) vmalloc(size)
#define ut_free(size)  vfree(size)

#elif defined(USER_SPACE_UT)
#define UT(fmt, args...) printf(fmt, ##args)

#define SSTAR_UT_ERR(fmt, args...) printf("\033[31m[UT ERR]\033[0m [%s():line %d] " fmt, __FUNCTION__, __LINE__, ##args)

#define SSTAR_UT_INFO(fmt, args...) printf("[UT INFO] [%s():line %d] " fmt, __FUNCTION__, __LINE__, ##args)

#define ut_alloc(size) malloc(size)
#define ut_free(size)  free(size)
#endif

#define SEPARATOR() UT("======================================================================\n")

#define WRAP_SEPARATOR(fmt, args...) \
    {                                \
        SEPARATOR();                 \
        UT(fmt, ##args);             \
        SEPARATOR();                 \
    }

#define for_each_suite(pos, head) for (pos = head; pos != NULL; pos = pos->next_suite)

#define for_each_test(pos, head) for (pos = head; pos != NULL; pos = pos->next_test)

#define SSTAR_RUN_FUNC(root, func) \
    do                             \
    {                              \
        if (func)                  \
            func(root);            \
    } while (0);

// =============================================================================
// Some struct and function pointer
// =============================================================================
typedef struct sstar_ut       sstar_ut;
typedef struct sstar_ut_suite sstar_ut_suite;
typedef struct sstar_ut_test  sstar_ut_test;

typedef UT_RESULT sstar_ut_test_func(sstar_ut_test* test);

typedef void sstar_ut_suite_init_func(sstar_ut_suite* suite);

typedef void sstar_ut_suite_exit_func(sstar_ut_suite* suite);

typedef void sstar_ut_init_func(sstar_ut* ut);

typedef void sstar_ut_exit_func(sstar_ut* ut);

// =============================================================================
// struct sstar_ut_test represents a test item
// @name: Test name, it is best to use the same string as the function name
// @exexcute: Function pointer to the test function
// @ignore: Whether the test item is ignored. Since some test items will cause
//          kernel panic, they will not run by default unless specified separately
// @parent: Pointer to a parent test, if parent test fails, this test does not run,
// @user_data: Any user data associated with this test
// @id: A unique identifier for the test item
// @run: Identifies whether the test item needs to be run, set in miu_ut_prepare_run()
// @result: The result of the last run
// @suite: Pointer to suite
// @ut: Pointer to ut
// @next_test: Pointer to next test item, or NULL
// =============================================================================
struct sstar_ut_test
{
    const char*         name;
    sstar_ut_test_func* execute;
    bool                ignore;
    sstar_ut_test*      parent;
    void*               user_data;
    unsigned int        id;
    bool                run;
    bool                result;
    sstar_ut_suite*     suite;
    sstar_ut*           ut;
    sstar_ut_test*      next_test;
};

// =============================================================================
// struct sstar_ut_suite represents a test collection, including test items with
// similar functions
// @name: Suite name
// @suite_init: Run before suite if not NULL
// @suite_exit: Run after suite if not NULL
// @user_data: Any user data associated with this ut
// @id: A unique identifier for the suite, not used now
// @run: Any test in the suite needs to run, the suite needs to run
// @ut: Pointer to the ut
// @first_test: Pointer to the first test item in the suite, or NULL
// @this_test: Pointer to the test item in running
// @next_suite: Pointer to next suite, or NULL
// =============================================================================
struct sstar_ut_suite
{
    const char*               name;
    sstar_ut_suite_init_func* suite_init;
    sstar_ut_suite_exit_func* suite_exit;
    void*                     user_data;
    unsigned int              id;
    bool                      run;
    sstar_ut*                 ut;
    sstar_ut_test*            first_test;
    sstar_ut_test*            this_test;
    sstar_ut_suite*           next_suite;
};

// =============================================================================
// struct sstar_ut_test represents a sstar ut
// @name: UT name
// @ut_init: Run before all suite if not NULL
// @ut_exit: Run after all suite if not NULL
// @user_data: Any user data associated with this ut
// @first_suite: Pointer to the first suite in the ut, or NULL
// @cmd_args: Args from command line
// @suite_num: The number of suites included in ut
// @test_num: The number of tests included in ut
// @run_num: The number of tests run last time
// @pass_num: The number of tests that passed last time
// @skip_num: The number of tests that skipped last time
// @fail_num: The number of tests that failed last time
// =============================================================================
struct sstar_ut
{
    const char*         name;
    sstar_ut_init_func* ut_init;
    sstar_ut_exit_func* ut_exit;
    void*               user_data;
    sstar_ut_suite*     first_suite;
    char*               cmd_args;
    unsigned int        suite_num;
    unsigned int        test_num;
    unsigned int        run_num;
    unsigned int        pass_num;
    unsigned int        skip_num;
    unsigned int        fail_num;
};

// =============================================================================
// Create a new sstar_ut
// =============================================================================
sstar_ut* sstar_ut_create(const char* name, sstar_ut_init_func* ut_init, sstar_ut_exit_func* ut_exit, void* user_data);

// =============================================================================
// Create and add a new suite to sstar_ut
// =============================================================================
sstar_ut_suite* sstar_ut_add_suite(sstar_ut* ut, const char* name, sstar_ut_suite_init_func* suite_init,
                                   sstar_ut_suite_exit_func* suite_exit, void* user_data);

// =============================================================================
// Create and add a new test to sstar_ut_suite
// =============================================================================
sstar_ut_test* sstar_ut_add_test(sstar_ut_suite* suite, const char* name, sstar_ut_test_func* test_func, bool ignore,
                                 sstar_ut_test* parent, void* user_data);

// =============================================================================
// Free UT resource when you exit your module
// =============================================================================
void sstar_ut_free(sstar_ut* ut);

// =============================================================================
// Easy way to create ut test if you do not have extra parameters
// =============================================================================
#define sstar_ut_create_lite(name)         sstar_ut_create(name, NULL, NULL, NULL)
#define sstar_ut_add_suite_lite(ut, name)  sstar_ut_add_suite(ut, name, NULL, NULL, NULL)
#define sstar_ut_add_test_lite(ut, excute) sstar_ut_add_test(ut, #excute, excute, false, NULL, NULL)

#if defined(KERNEL_SPACE_UT)
// =============================================================================
// Print sstar_ut information and write last result into @buf,
// call this function in module_show
// =============================================================================
ssize_t sstar_ut_show(sstar_ut* ut, char* buf);

// =============================================================================
// Parse the contents of buf and run ut, call this function in module_store
// =============================================================================
ssize_t sstar_ut_run(sstar_ut* ut, const char* buf, size_t count);

// =============================================================================
// Easy way to define the show and store functions of the module
// =============================================================================
#define SSTAR_UT_ATTR(ut)                                                                                             \
    static ssize_t sstar_show_##ut(struct device* dev, struct device_attribute* attr, char* buf)                      \
    {                                                                                                                 \
        return sstar_ut_show(ut, buf);                                                                                \
    }                                                                                                                 \
    static ssize_t sstar_store_##ut(struct device* dev, struct device_attribute* attr, const char* buf, size_t count) \
    {                                                                                                                 \
        return sstar_ut_run(ut, buf, count);                                                                          \
    }

#elif defined(USER_SPACE_UT)
// =============================================================================
//
// =============================================================================
unsigned int sstar_ut_show(sstar_ut* ut, char* name);

// =============================================================================
//
// =============================================================================
unsigned int sstar_ut_run(sstar_ut* ut, int argc, char** argv);
#endif

#endif /* __SSTAR_UT_H__ */
