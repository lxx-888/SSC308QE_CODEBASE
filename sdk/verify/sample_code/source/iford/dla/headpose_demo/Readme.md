# headpose_demo使用说明

---

## 一、demo功能演示

- 姿态识别

  ```
                                                                   +----------------------+
                                                                   |                      |
                                                                   v                      |
  +-------+       +-------+       +-------+ +-+port0:Source+-> +---+--+      +------+     |
  |  VIF  | +---> |  ISP  | +---> |  SCL  |         +          | VENC | +--> | RTSP |     |
  +-------+       +-------+       +---+---+         |          +------+      +------+     |
                                      |             |                                     |
                                      +             |          +---------+                |
                             port1：Scaled +-----------------> |HPOSE DET|                |
                                                    |          +----+----+                |
                                                    |               |                     |
                                                    |     coordinate|                     |
                                                    +<--------------+                     |
                                                    v                                     |
                                            +-------+-+        +---------+                |
                                            | Stretch +------> |HPOSE DET|                |
                                            +---------+        +----+----+                |
                                                                    |               +-----+-+
                                                                    +---------------+  RGN  |
                                                                                    +-------+
  ```

  - 将dla的hpose功能串进pipeline。HPOSE的检测接口先获取到头部、身体的数量和坐标，分别将头部和身体的坐标传给SCL的Stretch裁剪，再将数据分别给到HPOSE的头部识接口和身体识别接口做处理，最后将头部和身体的坐标信息、姿态的类别（shake、nod、stand、lie）用RGN框出来，通过RTSP拉流出来预览。
  - 四种姿态分别使用四种不同的颜色框框出来。
  - 当键入字符"q"时，程序退出。


---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/dla/headpose_demo`命令进行编译;

3. 到`sample_code/out/arm/app/prog_dla_headpose_demo`获取可执行文件;

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

- 将可执行文件`prog_dla_headpose_demo`放到板子上，修改权限为777
- 使用sensor为imx415

- 运行命令`./prog_dla_headpose_demo index 6`进入姿态识别

  - input:
    - `resource/input/dla/headpose/models/hpose_hdet_36y.img`：检测人形的网络模型。
      `resource/input/dla/headpose/models/hpose_fdet_36y.img`：检测人脸的网络模型。
      `resource/input/dla/headpose/models/hpose_angle_66a.img`：检测人脸欧拉角的网络模型。
      `resource/input/dla/headpose/models/hpose_pose_12y.img`：识别人体姿态的网络模型。

      需要手动从`alkaid/project/board/iford/dla_file/ipu_net/`拷贝到至此

  - output：

    - 无

---

## 五、运行结果说明

- 效果查看

  demo运行起来后，正常出流会打印rtsp url，使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。url例如：`rtsp://your_ipaddr/6600`。键入字符"q"退出手势识别。


---