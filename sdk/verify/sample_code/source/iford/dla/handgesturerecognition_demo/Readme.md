# handgesturerecognition_demo使用说明

---

## 一、demo功能演示

- 手势识别

  ```
                                                                   +----------------------+
                                                                   |                      |
                                                                   v                      |
  +-------+       +-------+       +-------+ +--port0:Source--> +---+--+      +------+     |
  |  VIF  | +---> |  ISP  | +---> |  SCL  |         +          | VENC | +--> | RTSP |     |
  +-------+       +-------+       +---+---+         |          +------+      +------+     |
                                      |             |                                     |
                                      |             |          +---------+                |
                             port1：Scaled-------------------> | HGR DET |                |
                                                    |          +----+----+                |
                                                    |               |                     |
                                                    |     coordinate|                     |
                                                    +<--------------+                     |
                                                    v                                     |
                                            +---------+        +----+----+                |
                                            | Stretch +------> | HGR REC |                |
                                            +---------+        +----+----+                |
                                                                    |               +-----+-+
                                                                    +---------------+  RGN  |
                                                                                    +-------+
  ```

  - 将dla的hgr功能串进pipeline。HGR_DET的检测接口获取到手的坐标信息后，传给SCL的Stretch裁剪，再将数据给到HGR_REC做识别处理，最后将手的坐标信息、手势的类别（OK、L、YES、STOP）用RGN框出来，通过RTSP拉流出来预览。
  - 四种手势分别使用四种不同的颜色框框出来。
  - 当键入字符"q"时，程序退出。


---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/dla/handgesturerecognition_demo`命令进行编译;

3. 到`sample_code/out/arm/app/prog_dla_handgesturerecognition_demo`获取可执行文件;

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

- 将可执行文件`prog_dla_handgesturerecognition_demo`放到板子上，修改权限为777
- 使用sensor为imx415

- 运行命令`./prog_dla_handgesturerecognition_demo index 6`进入手势识别

  - input:
    - `resource/input/dla/handgesturerecognition/models/hgr_det_36y.img`：检测手的网络模型。需要手动从`alkaid/project/board/iford/dla_file/ipu_net/`拷贝到至
    - `resource/input/dla/handgesturerecognition/models/hgr_land_22y.img`：手势识别的网络模型。需要手动从`alkaid/project/board/iford/dla_file/ipu_net/`拷贝到至此

  - output：
    - 无

---

## 五、运行结果说明

- 效果查看

  demo运行起来后，正常出流会打印rtsp url，使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。url例如：`rtsp://your_ipaddr/6600`。键入字符"q"退出手势识别。


---