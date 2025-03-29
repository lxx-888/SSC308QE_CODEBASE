# cpp_suite_demo使用说明

---
## 一、Rtos下C++测试
#### 1.1. 支持情况

support c++11

|                    |         Support          |
| :----------------: | :----------------------: |
|       limits       |           Yes            |
|        new         |           Yes            |
|      utility       |           Yes            |
|     functional     |           Yes            |
|       memory       |           Yes            |
|       string       |           Yes            |
|       vector       |           Yes            |
|        list        |           Yes            |
|       deque        |           Yes            |
|       queue        |           Yes            |
|       stack        |           Yes            |
|        map         |           Yes            |
|        set         |           Yes            |
|       bitset       |           Yes            |
|      iterator      |           Yes            |
|     algorithm      |           Yes            |
|      valarray      |           Yes            |
|       regex        |           Yes            |
|      numeric       |           Yes            |
|       mutex        |           Yes            |
|       thread       |           Yes            |
|       atomic       |           Yes            |
|      iostream      | Yes(only support output) |
| exception handling |            No            |
|        rtti        |            No            |
|        try         |            No            |
|       catch        |            No            |
|       throw        |            No            |
|    dynamic_cast    |            No            |
|       typeid       |            No            |

## 二、编译环境说明
1. 例如`SSC029C-S01A-S`型号板子，使用nand，ddr3的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc-rtos_iford.spinand.uclibc-9.1.0-ramdisk.ssc029c.128.qfn128_ddr3_defconfig`
    根据场景在project目录执行以下命令进行编译;

    `make ipc-rtos_iford.spinand.uclibc-9.1.0-ramdisk.ssc029c.128.qfn128_ddr3_defconfig;`

    rtos/proj下make menuconfig打开下列配置

    - CONFIG_LIBSTDCPP_SUPPORT

    ```
    System Options  --->
        System control Options  --->
            [*] Support C++ standard library
    ```

    - CONFIG_FREERTOS_POSIX_SUPPORT

    ```
    Kernel Options  --->
        [*]   Support FreeRTOS POSIX
    ```

    `make clean && make image -j8`

    > 注意：编译之前需要配置CONFIG_DUALOS_SAMPLE_DEMO，才能正常运行demo，可以进入alkaid/project目录下通过`make menuconfig`来配置，进入menuconfig界面之后通过以下路径打开此config：

    `Rtos->Rtos Application Options->Support pipeline demo applications->Support dualos_sample application`

2. 进到sample_code目录执行`make clean && make source/iford/dualos_sample/cpp_suite/linux`进行编译;

    注意

    - 如果自行在外部编译lib，编译链请与Rtos系统的同步
    - 编译过程中须有下面的编译选项支持，且头文件include到rtos/proj/libs/toolchain/arm-eabi_9.1.0/normal/include

    ```
    CXX_OPTIONS += -fno-short-enums -fno-exceptions -fno-rtti -D_GLIBCXX_HAS_GTHREADS -D_POSIX_THREADS -D_POSIX_TIMEOUTS -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_GTHREAD_USE_MUTEX_INIT_FUNC -D_GTHREAD_USE_RECURSIVE_MUTEX_INIT_FUNC -D_GTHREAD_USE_MUTEX_TIMEDLOCK -D_POSIX_TIMEOUTS -D_GTHREAD_USE_COND_INIT_FUNC
    ```

3. 到`sample_code/out/arm/app/prog_dualos_sample_cpp_suite_linux`获取可执行文件;

---
## 三、运行说明
1.将可执行文件`prog_dualos_sample_cpp_suite_linux`放到板子上，修改权限777
2.查看`/misc/earlyinit_setting.json`中的"NAME"是否为dualos_sample，如果不是请做修改并reboot：

```
    "APP_0_0": {
        "NAME": "dualos_sample",
        "PARAM": ""
    }
```

> 按`./prog_dualos_sample_cpp_suite_linux`运行demo
3.查看`cat /proc/dualos/log`中的log确定测试结果

---