# bgb_pipeline使用说明

仅支持在pure linux系统中运行

---

## 一、demo场景

**场景：单sensor bgb pipeline**

```
  单路sensor在pipeline上实现 4K@30 的背景虚化功能。

      +----------+      +----------+      +----------+                            +-----+      +------+
      |          |      |          |      |          |  Chn0 port0  3840x2160     |     |      |      |
      | VIF Dev0 +------> ISP Dev0 +------> SCL Dev0 +----------------------------> IVE +------> VENC |
      |          |      |          |      |          |                            |     |      |      |
      +----------+      +----------+      +-----+----+                            +--^--+      +------+
                                                |       Chn0 port1  640x480       ^  |
                                                +---------------------------------+  |640x480 Ymsk
                                                |       DupBuf                       |
                                                |                                 +--+--+
                                                |       Chn0 port1  640x480       |     |
                                                +---------------------------------> DLA |
                                                                                  |     |
                                                                                  +-----+



  参数说明： vif接sensor pad0，isp/venc使用dev0 chn0, scl使用dev0 chn0 port0/port1

  其中：vif/isp/scl_port0/venc均为`3840x2160` scl_port1为`640x360`

  绑定模式：vif->isp->scl为realtime，scl->dla->ive->venc为framemode
```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;<br>
   例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册<br>
   `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`<br>
   在project目录执行以下命令进行编译;<br>
   `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`<br>
   `make clean && make image -j8`<br>

2. 进到sample_code目录执行`make clean && make source/iford/ive/bgb_pipeline`进行编译;

3. 到`sample_code/out/arm/app/prog_ive_bgb_pipeline`获取可执行文件;

---

## 三、运行环境说明

**板端环境：**

`SSC029A-S01A-S`型号板子sensor pad0对应CON24，sensor pad1对应CON25，sensor pad3对应CON22;

 在sensor pad0位置接上mipi sensor，对应跑的是4lane，使用imx415;

**dts配置：**

mipi snr0 4lane EVB上使用，默认dts已配好，无需修改

例如上述编译环境说明的`SSC029A-S01A-S`型号板子，直接编译使用即可

**sensor 驱动配置：**

    /customer/demo.sh路径修改insmod对应的sensor ko，例如用imx415时，可配置为：


>insmod /config/modules/5.10/imx415_MIPI.ko chmap=1 lane_num=4


**输入文件：**

```
   场景1处理背景虚化所需要的需要img文件 `hseg_y36.img`可在`    "/project/board/iford/dla_file/ipu_net/hseg_y36.img" `获取

   按`./prog_ive_bgb_pipeline index 7 iqbin xxx `运行，可在基础上加载指定iqbin文件，xxx为iqbin文件路径,

   默认为`/config/iqfile/[sensor_name]_api.bin`, 例如使用的为imx415时，默认iqbin路径为：`/config/iqfile/IMX415_MIPI_api.bin`

   可追加参数 “time”，可设置不同档位的模糊程度切换的时间间隙，若没有 “time” 则不开启切换的模式，默认为4个档位。

   如：`./prog_ive_bgb_pipeline index 7 iqbin xxx time 5`，表示每5s切换一次IVE_BGB的模糊程度。
```

---

## 四、运行说明

将可执行文件prog_vif_sensor_demo放到板子上，修改权限777

1. 按`./prog_ive_bgb_pipeline index 7`运行场景1单sensor bgb pipeline；

   > img输入路径为`"./resource/input/hseg_y36.img"`

运行demo后，会打印sensor可选择输入的res，输入需要的3840*2160@30fps需求的对应index 7，imx415sensor，会打印以下log：

```
  index 0, Crop(0,0,3840,2160), outputsize(3860,2250), maxfps 20, minfps 3, ResDesc 3840x2160@20fps

  index 1, Crop(0,0,3072,2048), outputsize(3096,2190), maxfps 30, minfps 3, ResDesc 3072x2048@30fps

  index 2, Crop(0,0,3072,1728), outputsize(3096,1758), maxfps 30, minfps 3, ResDesc 3072x1728@30fps

  index 3, Crop(0,0,2592,1944), outputsize(2616,1974), maxfps 30, minfps 3, ResDesc 2592x1944@30fps

  index 4, Crop(0,0,2944,1656), outputsize(2976,1686), maxfps 30, minfps 3, ResDesc 2944x1656@30fps

  index 5, Crop(0,0,2560,1440), outputsize(2592,1470), maxfps 30, minfps 3, ResDesc 2560x1440@30fps

  index 6, Crop(0,0,1920,1080), outputsize(1920,1080), maxfps 60, minfps 3, ResDesc 1920x1080@60fps

  index 7, Crop(12,16,3840,2160), outputsize(3864,2192), maxfps 30, minfps 3, ResDesc 3840x2160@30fps

  index 8, Crop(12,16,3840,2160), outputsize(3864,2192), maxfps 60, minfps 3, ResDesc 3840x2160@60fps

  index 9, Crop(0,0,3840,2160), outputsize(3864,2250), maxfps 5, minfps 3, ResDesc 3840x2160@5fps

  7
```

---

## 五、运行结果说明

**preview效果查看**

正常出流会打印rtsp url，单sensor打印一个url，例如以下log：

使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。

```
rtsp venc dev0, chn 0, type 3, url 6600

=================URL===================

rtsp://xxx.xxx.xxx.xxx/6600

=================URL===================

Create Rtsp H265 Session, FPS: 30

press q to exit
```

>  注意：your_ipaddr如果为0.0.0.0，请先配置网络，配置方法可参考如下：

```
ifconfig eth0 192.168.1.10 netmask 255.255.255.0

route add default gw 192.168.1.1
```

**退出命令**

    输入`q`即可退出demo
