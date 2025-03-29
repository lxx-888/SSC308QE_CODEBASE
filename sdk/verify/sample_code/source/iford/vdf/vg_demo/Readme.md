﻿# vg_demo使用说明

仅支持在pure linux系统中运行

---

## 一、demo场景

- 场景1：

  ```   
  +---------+                   +--------+      +--------+
  |   SCL   | ------port0-----> |  VENC  | ---> |  RTSP  |
  +----+----+                   +---+----+      +--------+
       |                            ^
       |                            | 
       |                            | 
       |                            | 
       |       +--------+       +---+----+
     port1---> |  VDF   | ----> |  RGN   |
               +--------+       +--------+

  ```

  从SCL灌视频流通过VDF_VG mode 实现电子围栏，并通过VENC上RGN模块显示检测结果。

  参数说明： 该场景内所有分辨率均为`640x360`

  绑定模式：scl->vdf vdf->venc 均为framemode

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`

    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/vdf/vg_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_vdf_vg_demo`获取可执行文件;

---

## 三、运行环境说明

- 板端环境：

    `SSC029A-S01A-S`型号板子sensor pad0对应CON3;


- dts配置：

  > 例如上述编译环境说明的`SSC029A-S01A-S`型号板子，直接编译使用即可


- 输入文件：

   场景1处理电子围栏所需要的需要yuv视频流 `vg_test_640x360_nv12.yuv`可在` "sample_code/source/iford/vdf/vg_demo/resource/input/vg_test_640x360_nv12.yuv" `获取

---

## 四、运行说明

将可执行文件prog_vdf_vg_demo放到板子上，修改权限777

1. 按`./prog_vdf_vg_demo`运行场景1处理电子围栏；

   > 视频流输入路径为`"./resource/input/vg_test_640x360_nv12.yuv"`

---

## 五、运行结果说明

1. demo的输出分为两个部分；

   > 检测处理的OSD贴图效果输出为`"./out/vdf/vg_demo_case0.es"`

   > VG检测结果的保存文本`"./out/vdf/vg_result.txt"`