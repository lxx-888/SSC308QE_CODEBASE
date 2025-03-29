# 一. 概述

## 1.1 说明

    此ut目录下，包含用于测试nor flash的伪代码和编译Makefile，如果只需要做简单的发送接收通讯，请前往目录：kernel/tools/spi 下，获取通用测试程序。

## 1.2 文件

    Makefile：编译

    readme.md：ut说明文档

    ut_mspi_norflash.c ：伪代码

# 二.操作步骤

## 2.1 编译

    在当前目录 kernel/drivers/sstar/mspi/ut/ 下执行make，编译得到ut_mspi_norflash可执行文件

    ...kernel/drivers/sstar/mspi/ut$ make

## 2.2 测试

    a) linux环境搭建，具体请参考wiki

    b) 将编译获得的可执行文件ut_mspi_norflash拷贝到板子的linux环境当中，可以使用tftp拷贝到customer目录下，或者nfs挂载本地PC的目录到板子linux环境。

    c) 确认要测试的mspi的padmux，并在对应PAD上接入一块nor flash，型号:W25Q128JW

        确认接入VCC/ GND/ CZ/ CK/ DI/ DO就可以正常使用，按照引脚名一一对应即可。

    d)进入ut_mspi_norflash所在目录，执行命令：

        ./ut_mspi_norflash -D /dev/spidev0.0 -s 5000000

        -D /dev/spidev0.0：测试mspi0 cs0

        -s 5000000：通讯频率设定 5000000Hz

    e)结果：

        根据打印提示，确认验证结果。






