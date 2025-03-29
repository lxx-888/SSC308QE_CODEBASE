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


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "module.h"


#define __START(op, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, An, ...) \
        __ARGS_##An(op, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24)
#define __ARGS_1(op, A, ...) __##op(A)
#define __ARGS_2(op, A, ...) __##op(A) __ARGS_1(op, __VA_ARGS__)
#define __ARGS_3(op, A, ...) __##op(A) __ARGS_2(op, __VA_ARGS__)
#define __ARGS_4(op, A, ...) __##op(A) __ARGS_3(op, __VA_ARGS__)
#define __ARGS_5(op, A, ...) __##op(A) __ARGS_4(op, __VA_ARGS__)
#define __ARGS_6(op, A, ...) __##op(A) __ARGS_5(op, __VA_ARGS__)
#define __ARGS_7(op, A, ...) __##op(A) __ARGS_6(op, __VA_ARGS__)
#define __ARGS_8(op, A, ...) __##op(A) __ARGS_7(op, __VA_ARGS__)
#define __ARGS_9(op, A, ...) __##op(A) __ARGS_8(op, __VA_ARGS__)
#define __ARGS_10(op, A, ...) __##op(A) __ARGS_9(op, __VA_ARGS__)
#define __ARGS_11(op, A, ...) __##op(A) __ARGS_10(op, __VA_ARGS__)
#define __ARGS_12(op, A, ...) __##op(A) __ARGS_11(op, __VA_ARGS__)
#define __ARGS_13(op, A, ...) __##op(A) __ARGS_12(op, __VA_ARGS__)
#define __ARGS_14(op, A, ...) __##op(A) __ARGS_13(op, __VA_ARGS__)
#define __ARGS_15(op, A, ...) __##op(A) __ARGS_14(op, __VA_ARGS__)
#define __ARGS_16(op, A, ...) __##op(A) __ARGS_15(op, __VA_ARGS__)
#define __ARGS_17(op, A, ...) __##op(A) __ARGS_16(op, __VA_ARGS__)
#define __ARGS_18(op, A, ...) __##op(A) __ARGS_17(op, __VA_ARGS__)
#define __ARGS_19(op, A, ...) __##op(A) __ARGS_18(op, __VA_ARGS__)
#define __ARGS_20(op, A, ...) __##op(A) __ARGS_19(op, __VA_ARGS__)
#define __ARGS_21(op, A, ...) __##op(A) __ARGS_20(op, __VA_ARGS__)
#define __ARGS_22(op, A, ...) __##op(A) __ARGS_21(op, __VA_ARGS__)
#define __ARGS_23(op, A, ...) __##op(A) __ARGS_22(op, __VA_ARGS__)
#define __ARGS_24(op, A, ...) __##op(A) __ARGS_23(op, __VA_ARGS__)

#define __EX_FUNC(A)  extern int A##_mod_init(void);extern void A##_mod_exit(void);
#define __CALL_INIT_FUNC(A)  A##_mod_init();
#define __CALL_EXIT_FUNC(A)  A##_mod_exit();

#define EXTERN_LOOP(...) __START(EX_FUNC,__VA_ARGS__, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define CALL_INIT_LOOP(...) __START(CALL_INIT_FUNC,__VA_ARGS__, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define CALL_EXIT_LOOP(...) __START(CALL_EXIT_FUNC,__VA_ARGS__, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

EXTERN_LOOP(MODULES);

static int __init mi_driver_init(void)
{
    CALL_INIT_LOOP(MODULES);
    return 0;
}

static void __exit mi_driver_exit(void)
{
    // EXIT_MODULES has reverse order of MODULES
    CALL_EXIT_LOOP(EXIT_MODULES);
}

subsys_initcall(mi_driver_init);
module_exit(mi_driver_exit);

MODULE_DESCRIPTION("MI");
MODULE_AUTHOR("SigmaStar");
#ifdef NON_COMMERCIAL_FOR_INTERNAL_TEST_ONLY
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("PROPRIETARY");
#endif
