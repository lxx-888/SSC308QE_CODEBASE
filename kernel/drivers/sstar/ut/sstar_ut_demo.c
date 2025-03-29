/*
 * sstar_ut_demo.c - Sigmastar
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

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <sstar_ut.h>

// =============================================================================
//                      Variables and Data Structures
// =============================================================================
static sstar_ut* demo_ut;
struct piont
{
    int x;
    int y;
};
static int          test_data    = 212;
static char         suite_data[] = "abcdefg";
static struct piont ut_data      = {10, 20};

// =============================================================================
//                      test functions
// =============================================================================
static UT_RESULT demo_ut_test_pass(sstar_ut_test* test)

{
    SSTAR_UT_INFO("Test pass.\n");
    return UT_PASS;
}

static UT_RESULT demo_ut_test_fail(sstar_ut_test* test)
{
    SSTAR_UT_ERR("Test Fail.\n");
    return UT_FAIL;
}

static UT_RESULT demo_ut_get_test_data(sstar_ut_test* test)
{
    int* data = (int*)(test->user_data);
    SSTAR_UT_INFO("Test data: %d\n", *data);

    return UT_PASS;
}

static UT_RESULT demo_ut_get_suite_data(sstar_ut_test* test)
{
    char* data = (char*)(test->suite->user_data);
    SSTAR_UT_INFO("Suite data: %s\n", data);

    return UT_PASS;
}

static UT_RESULT demo_ut_get_ut_data(sstar_ut_test* test)
{
    struct piont* data = (struct piont*)(test->ut->user_data);
    SSTAR_UT_INFO("UT data: %d %d\n", data->x, data->y);

    return UT_PASS;
}

static UT_RESULT demo_ut_get_cmd_args(sstar_ut_test* test)
{
    char* cmd_args = test->ut->cmd_args;

    if (cmd_args == NULL)
        return UT_FAIL;
    if (strcmp(cmd_args, "hello") == 0)
        return UT_PASS;
    return UT_FAIL;
}

static UT_RESULT demo_ut_will_skip(sstar_ut_test* test)
{
    SSTAR_UT_INFO("I will skip if %s failed.\n", test->parent->name);
    return UT_PASS;
}

static UT_RESULT demo_ut_will_not_skip(sstar_ut_test* test)
{
    SSTAR_UT_INFO("I will not skip.\n");
    return UT_PASS;
}

// =============================================================================
//                      init and exit functions
// =============================================================================
static void demo_ut_start(sstar_ut* ut)
{
    SSTAR_UT_INFO("%s start test.\n", ut->name);
}

static void demo_ut_suite_init(sstar_ut_suite* suite)
{
    SSTAR_UT_INFO("%s init.\n", suite->name);
}

static void demo_ut_suite_exit(sstar_ut_suite* suite)
{
    SSTAR_UT_INFO("%s exit.\n", suite->name);
}

// =============================================================================
//                      Register for tests
// =============================================================================
static void demo_ut_setup(void)
{
    sstar_ut_test*  pass_parent;
    sstar_ut_test*  fail_parent;
    sstar_ut_suite* suite;

    demo_ut = sstar_ut_create("DEMO", demo_ut_start, NULL, &ut_data);

    suite       = sstar_ut_add_suite(demo_ut, "demo_easy", demo_ut_suite_init, demo_ut_suite_exit, NULL);
    pass_parent = sstar_ut_add_test_lite(suite, demo_ut_test_pass);
    fail_parent = sstar_ut_add_test_lite(suite, demo_ut_test_fail);

    suite = sstar_ut_add_suite(demo_ut, "demo_data", demo_ut_suite_init, demo_ut_suite_exit, suite_data);
    sstar_ut_add_test(suite, "demo_ut_get_test_data", demo_ut_get_test_data, false, NULL, &test_data);
    sstar_ut_add_test_lite(suite, demo_ut_get_suite_data);
    sstar_ut_add_test_lite(suite, demo_ut_get_ut_data);
    sstar_ut_add_test(suite, "demo_ut_get_cmd_args", demo_ut_get_cmd_args, true, NULL, NULL);

    suite = sstar_ut_add_suite_lite(demo_ut, "demo_skip");
    sstar_ut_add_test(suite, "demo_ut_will_skip", demo_ut_will_skip, false, fail_parent, NULL);
    sstar_ut_add_test(suite, "demo_ut_will_not_skip", demo_ut_will_not_skip, false, pass_parent, NULL);
}

// =============================================================================
//                      Create a device node
// =============================================================================

// static int sstar_show_demo_ut(struct device* dev, struct device_attribute* attr, char* buf)
// {
//     return sstar_ut_show(demo_ut, buf);
// }

// static int sstar_store_demo_ut(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
// {
//     return sstar_ut_run(demo_ut, buf, count);
// }
// The above codes eaual to SSTAR_UT_ATTR(demo_ut)

SSTAR_UT_ATTR(demo_ut);

DEVICE_ATTR(demo_ut, S_IWUSR | S_IRUSR, sstar_show_demo_ut, sstar_store_demo_ut);

struct file_operations demo_ut_ops = {
    .owner = THIS_MODULE,
};

static int           major;
static struct class* ut_cls;
struct device*       ut_dev;

static int __init demo_ut_init(void)
{
    major  = register_chrdev(0, "demo_ut", &demo_ut_ops);
    ut_cls = class_create(THIS_MODULE, "demo");
    ut_dev = device_create(ut_cls, 0, MKDEV(major, 0), NULL, "ut");
    device_create_file(ut_dev, &dev_attr_demo_ut);

    demo_ut_setup();
    printk("[DEMO UT] init.\n");

    return 0;
}

static void __exit demo_ut_exit(void)
{
    device_destroy(ut_cls, MKDEV(major, 0));
    class_destroy(ut_cls);
    unregister_chrdev(major, "ut");

    sstar_ut_free(demo_ut);

    printk("[DEMO UT] exit.\n");
}

module_init(demo_ut_init);
module_exit(demo_ut_exit);
MODULE_LICENSE("GPL");
