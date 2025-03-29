# 简介

该目录下的文件夹以各个模块为单位，分别存放了展示各个模块功能的演示demo。而每个demo的具体使用方法，均存于各个demo目录下的Readme中。

# aov

AOV（Always On Video）是一种视频监控方案，旨在实现全天候、持续不断的录像，而不会中断或耗费过多电力。

| 名称     | 支持平台  | 介绍            |
| -------- | --------- | --------------- |
| aov_demo | PureLinux | aov正常电量模式 |

# audio

audio目录下保存了包括音频输入、音频输出和各种音频处理算法的场景演示demo。

| 名称    | 支持平台          | 介绍                                                   |
| ------- | ----------------- | ------------------------------------------------------ |
| ai_demo | PureLinux、DualOS | 音频输入                                               |
| alg     | PureLinux、DualOS | 各种音频处理算法，具体算法内容可以查阅该目录下的Readme |
| ao_demo | PureLinux、DualOS | 音频输出                                               |

# cm4

该目录下保存了使用cm4的场景demo

| 名称    | 支持平台 | 介绍               |
| ------- | -------- | ------------------ |
| preload | DualOS   | 使用了cm4的preload |

# crypto

该目录下保存了加解密算法的场景演示demo

| 名称     | 支持平台          | 介绍      |
| -------- | ----------------- | --------- |
| aes_demo | PureLinux、DualOS | aes加解密 |
| rsa_demo | PureLinux、DualOS | rsa加解密 |
| sha_demo | PureLinux、DualOS | sha加解密 |

# cv

用于配合cv tool进行视频预览和抓图的场景演示demo

| 名称        | 支持平台          | 介绍                              |
| ----------- | ----------------- | --------------------------------- |
| server_demo | PureLinux、DualOS | 用于配合cv tool进行视频预览和抓图 |

# disp

用于视频显示的模块，主要功能是对前端输出的图像做硬件拼图，并对硬件拼图后的图像进行颜色空间转换，最终通过HDMI/VGA/MIPI/TTL等接口输出到显示器或LCD

| 名称     | 支持平台          | 介绍                            |
| -------- | ----------------- | ------------------------------- |
| ttl_demo | PureLinux、DualOS | 使用disp将图片输出到TTL显示屏上 |

# dla

基于深度学习，使用IPU模型达成的各种识别功能的场景演示demo。

| 名称                        | 支持平台          | 介绍                     |
| --------------------------- | ----------------- | ------------------------ |
| classification_demo         | PureLinux、DualOS | 人车宠物分类             |
| detection_demo              | PureLinux、DualOS | 人车宠物等各种对象的识别 |
| facerecognition_demo        | PureLinux、DualOS | 人脸识别                 |
| handgesturerecognition_demo | PureLinux、DualOS | 手势识别                 |
| headpose_demo               | PureLinux、DualOS | 姿态识别                 |

# dualos_sample

该目录demo主要用于dualos系统下相关模块在rtos端或者双端（linux+rtos）的接口使用方法，支持linux直接执行。

| 名称  | 支持平台 | 介绍                    |
| ----- | -------- | ----------------------- |
| rpmsg | DualOS   | 使用rpmsg实现dualOS通讯 |

# ive

IVE模块是指图像/视频引擎模块，是一种用于图像和视频处理的技术。在计算机视觉、图像处理、视频分析等领域具有广泛的应用。可以用于图像和视频的编码、解码、特征提取、目标检测、人脸识别、图像增强、图像去噪、运动检测等任务。

| 名称         | 支持平台          | 介绍                                                                      |
| ------------ | ----------------- | ------------------------------------------------------------------------- |
| bgb_pipeline | PureLinux、DualOS | 使用IVE的bgb算子进行动态背景虚化                                          |
| sample       | PureLinux、DualOS | 各种ive效果的算子，每种算子都有不同的效果，具体内容可查阅该目录下的Readme |

# ldc

LDC模块是专门对图像进行矫正处理的模块，包含多种功能

| 名称       | 支持平台          | 介绍                                                      |
| ---------- | ----------------- | --------------------------------------------------------- |
| drag_demo  | PureLinux、DualOS | 使用LDC模块的LDC功能实现图像展开及窗口拖动                |
| eis_demo   | PureLinux         | 使用LDC模块的DIS功能实现陀螺仪防抖                        |
| ldc_demo   | PureLinux、DualOS | 使用LDC模块的LDC功能实现畸变矫正                          |
| ldc1D_demo | PureLinux、DualOS | 通过ISP模块实现1维畸变矫正                                |
| pmf_demo   | PureLinux、DualOS | 通过LDC模块的PMF功能，通过使用各种3x3矩阵实现各种图像变换 |

# preload

preload是指在DualOS平台下，板端启动时提前创建好一条pipeline，使得可以在启动完毕后直接进行拉流获取视频的操作。

| 名称    | 支持平台 | 介绍                                            |
| ------- | -------- | ----------------------------------------------- |
| preload | DualOS   | 常规情况下的preload以及AOV低电量模式下的preload |

# rgn

RGN（Region）模块是用于根据实际需要，对pipeline中的图像，叠加一层新的图层的模块。

| 名称     | 支持平台          | 介绍                    |
| -------- | ----------------- | ----------------------- |
| rgn_demo | PureLinux、DualOS | 使用RGN模块进行图层叠加 |

# scl

SCL模块是用于将输入图像进行缩放的模块

| 名称     | 支持平台          | 介绍                          |
| -------- | ----------------- | ----------------------------- |
| scl_demo | PureLinux、DualOS | 使用SCL模块将输入图像进行缩放 |

# uac

UAC就是USB AUDIO CAMERA，使用USB连接音频设备。

| 名称            | 支持平台          | 介绍                                         |
| --------------- | ----------------- | -------------------------------------------- |
| uac_device_demo | PureLinux、DualOS | 连接USB音频设备，使用MIC进行录音或者播放音频 |

# uvc

UVC就是USB VEDIO CAMERA，使用USB连接视频设备。

| 名称            | 支持平台          | 介绍                                |
| --------------- | ----------------- | ----------------------------------- |
| uvc_device_demo | PureLinux、DualOS | 连接USB视频设备，播放视频设备的画面 |

# vdf

VDF模块是用于特定事件检测的模块

| 名称    | 支持平台          | 介绍                            |
| ------- | ----------------- | ------------------------------- |
| md_demo | PureLinux、DualOS | 使用VDF模块的MD功能实现移动检测 |
| od_demo | PureLinux、DualOS | 使用VDF模块的OD功能实现遮挡检测 |
| vg_demo | PureLinux、DualOS | 使用VDF模块的VG功能实现虚拟围栏 |

# venc

VENC模块是编码模块，对输入的图像进行需要的编码，然后输出。

| 名称             | 支持平台          | 介绍                                               |
| ---------------- | ----------------- | -------------------------------------------------- |
| jpeg_splice/demo | PureLinux、DualOS | 通过软件处理将jpeg图像进行拼接                     |
| jpeg_splice/pipe | PureLinux、DualOS | 通过软件处理将jpeg图像进行拼接的场景演示的pipeline |
| multi_ring       | PureLinux、DualOS | 使用ring mode，在VENC后再串接一个VENC的sub chn     |
| venc_demo        | PureLinux、DualOS | 经过VENC设定，输出不同格式码流                     |

# vif

VIF模块实现启用视频输入设备、视频输入通道、绑定视频输入通道等功能的模块。

| 名称            | 支持平台          | 介绍                    |
| --------------- | ----------------- | ----------------------- |
| getrawdata_demo | PureLinux、DualOS | 使用IQserver获取rawdata |
| sensor_demo     | PureLinux、DualOS | 单目/双目摄像头pipeline |
