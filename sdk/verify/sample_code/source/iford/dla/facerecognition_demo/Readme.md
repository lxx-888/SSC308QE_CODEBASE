# facerecognition_demo使用说明

---

## 一、demo功能演示

- 人脸注册

  ```
  +---------+      +---------+      +---------+                     +--------+     +--------+
  |   VIF   | ---> |   ISP   | ---> |   SCL   | ---port0:Source---> |  VENC  | --> |  RTSP  |
  +---------+      +---------+      +----+----+         +           +--------+     +--------+
                                         |              |
                                         |              |                ^
                                         |              v                |
                                         |                               |
                                         |         +--------+       +----+---+
                                  port1:Scaled---> |   FR   | ----> |  RGN   |
                                                   +----+---+       +--------+
                                                        |
                                                        |
                                                        v
                                              +---------+-------+
                                              | FaceFeatureData |
                                              +-----------------+
  ```

  - 将dla的fr功能串进pipeline，scl port0和port1分别将来自vif的原始分辨率的bufferinfo和经由scl缩放成480x288分辨率大小的bufferinfo送给fr处理，得到人脸的坐标信息后，使用rgn的frame功能，attach到venc绘制人脸框。

  - 当键入字符"q"时，线程dump出此时画面中的最大人脸图（ARGB8888）及其特征数据。

  - 在注册模式下，仅识别画面中最大的人脸，并用绿色框框出来。

- 人脸识别
  ```
  +---------+      +---------+      +---------+                     +--------+     +--------+
  |   VIF   | ---> |   ISP   | ---> |   SCL   | ---port0:Source---> |  VENC  | --> |  RTSP  |
  +---------+      +---------+      +----+----+         +           +--------+     +--------+
                                         |              |
                                         |              |                ^
                                         |              v                |
                                         |                               |
                                  port1:Scaled---> +--------+       +----+---+
                                                   |        | ----> |  RGN   |
                                                   |   FR   |       +--------+
                  +-----------------+              |        |
                  | FaceFeatureData +------------> +--------+
                  +-----------------+
  ```

  - 将dla的fr功能串进pipeline，scl port0和port1分别将来自vif的原始分辨率的bufferinfo和经由scl缩放成480x288分辨率大小的bufferinfo送给fr处理，且fr会读取人脸注册时的dump数据FaceFeatureData，用于识别目标人脸。fr处理得到人脸的坐标信息后，使用rgn的frame功能，attach到venc绘制人脸框。

  - 在识别模式下，会读取注册的人脸数据，若在画面中出现目标人脸，会用绿色框框出来，其余非目标人脸则用红色框框出来。

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/dla/facerecognition_demo`命令进行编译;

3. 到`sample_code/out/arm/app/prog_dla_facerecognition_demo `获取可执行文件;

---

## 三、运行环境说明

> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

- 板端环境：

  `SSC029A-S01A-S`型号板子sensor pad0对应CON3，在sensor pad0位置接上mipi sensor即可，此demo使用的是imx415

- dts配置：

  默认dts已配好，无需修改

- sensor 驱动配置：

  默认已加载好imx415_MIPI.ko，无需修改

  ```
  insmod /config/modules/5.10/imx415_MIPI.ko chmap=1
  ```

---

## 四、运行说明

- 将可执行文件`prog_dla_facerecognition_demo`放到板子上，修改权限为777
- 使用sensor为imx415
- 需要先运行人脸注册，获取人脸特征数据，再运行人脸识别



1. 运行命令`./prog_dla_facerecognition_demo 0 index 6`进入人脸注册流程

   - input:

     - `resource/input/dla/facerecognition/models/fr_feature_as.img`：人脸特征提取的网络模型。需要手动从`alkaid/project/board/iford/dla_file/ipu_net/fr_feature_as.img`拷贝到至此

     - `resource/input/dla/facerecognition/models/fr_det_y24s.img`：人脸检测的网络模型。需要手动从`alkaid/project/board/iford/dla_file/ipu_net/fr_det_y24s.img`拷贝到至此

   - output：

     - `out/dla/facerecognition/FaceFeatureData`：键入字符"q"，线程dump出注册的人脸特征数据

     - `out/dla/facerecognition/FaceCrop.argb8888`：键入字符"q"，线程dump出注册的人脸裁剪图，分辨率为112x112。可以使用7yuv等软件进行查看，或使用ffmpeg转换成jpg格式后再预览：`ffmpeg -f rawvideo -pix_fmt bgra -s 112x112 -i FaceCrop.argb8888 FaceCrop.jpg`

2. 运行命令`./prog_dla_facerecognition_demo 1 index 6`进入人脸识别流程

   - input：

     - `resource/input/dla/facerecognition/models/fr_feature_as.img`：人脸特征提取的网络模型。需要手动从`alkaid/project/board/iford/dla_file/ipu_net/fr_feature_as.img`拷贝到至此

     - `resource/input/dla/facerecognition/models/fr_det_y24s.img`：人脸检测的网络模型。需要手动从`alkaid/project/board/iford/dla_file/ipu_net/fr_det_y24s.img`拷贝到至此

     - `out/dla/facerecognition/FaceFeatureData`：注册人脸的特征数据

   - output:

     - 无

---

## 五、运行结果说明

- 效果查看

  demo运行起来后，正常出流会打印rtsp url，使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。url例如：`rtsp://your_ipaddr/6600`

- 人脸注册：

  1. 注册流程中，会自动使用绿色框框出一个最大人脸

  2. 当调整好注册角度时，键入字符"q"，demo将保存此时绿框内的人脸图及其特征数据，并自动退出demo。

- 人脸识别：

  1. 识别流程中，会自动使用绿色框框出注册时保留的目标人脸，其他人脸则使用红色框框出来（最大支持画面中10个人脸的检测与识别）

  2. 键入字符"q"自动退出demo。

---