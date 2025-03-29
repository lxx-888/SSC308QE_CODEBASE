# eis_demo使用说明

仅支持在pure linux系统中运行

---

## 一、demo场景

* 场景：eis 陀螺仪防抖模式 pipeline

  ```
  vif->isp->ldc->scl->venc->rtsp

  参数说明：vif接sensor pad0，isp/ldc/venc使用dev0 chn0,scl使用dev1 chn0,ldc模块使用dis模式

  绑定模式：vif->isp->scl->ldc->venc为framemode；
  ```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

   例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

   `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

   在project目录执行以下命令进行编译;

   `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`
   `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/ldc/eis_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_ldc_eis_demo`获取可执行文件;

---

## 三、运行环境说明

* 板端环境：

  `SSM000A-S01A-S`型号板子sensor pad0对应CON24，sensor pad1对应CON25，sensor pad3对应CON22;
  在sensor pad0位置接上mipi sensor，对应跑的是4lane，可以是imx415，imx307等。

  将陀螺仪接到引脚JP152上，具体的接线情况可以参考IS用户手册中的具体图文演示。

* dts配置：

  mipi snr0 4lane EVB上使用，默认dts已配好，无需修改

  > 例如上述编译环境说明的`SSC029A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：

  /customer/demo.sh路径修改insmod对应的sensor ko，当前 demo可配置为：

  ```
  #mi sensor
  insmod /config/modules/5.10/imx415_MIPI.ko chmap=1 lane_num=4
  ```

* 陀螺仪驱动配置：

  gyro.ko未打包进image中，需要手动从/kernel/drivers/sstar/gyro目录下取得gyro.ko放入到/cusomoer之中。当前demo使用的gyro型号是ICG20660, SPI Interface。

  /customer/demo.sh路径修改insmod对应的module ko，当前 demo 配置为：

  ```
  #mi module mknod
  insmod /customer/gyro.ko
  ```

* 输入文件：

  场景可加载指定的iq bin文件，如不指定默认加载/config/iqfile/`sensorname`_api.bin文件；
  部分sensor的iq bin文件可在`/project/board/iford/iqfile`路径下获取。

* 客制化相关：

  如果客户有需求需要更换自己的gyro_sensor时，公司提供了适配和移植的框架与代码，详情可以移步至《IS用户手册》 中查阅相关适配内容。

---

## 四、运行说明

将可执行文件prog_ldc_eis_demo放到板子上，修改权限777

按`./prog_ldc_eis_demo`运行场景dis mode pipeline；

> 按`./prog_ldc_eis_demo iqbin xxx`运行，可在场景上加载指定iqbin文件，xxx为iqbin文件路径

运行demo后，demo会自行寻找```./resource/input/CalibPoly_new.bin```文件，当找到该文件时会启动ldc+eis模式，若不存在该文件则会启动eis模式。

确认模式后会打印sensor可选择输入的res，输入你想选择preview的res（sensor resolution），例如imx317sensor，会打印以下log：

```
index 0, Crop(0,0,3840,2160), outputsize(3864,2174), maxfps 30, minfps 3, ResDesc 3840x2160@30fps
index 1, Crop(0,0,3840,2160), outputsize(3864,2174), maxfps 15, minfps 3, ResDesc 3840x2160@15fps
choice which resolution use, cnt 2
0
You select 0 res
Res 0
```
防抖效果默认为开启状态，输入`d`会关闭EIS的防抖效果，输入`e`会重新开启防抖效果

---

## 五、运行结果说明

* preview效果查看

正常出流会打印rtsp url，例如以下log：

使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。

```
rtsp venc dev0, chn 0, type 3, url 6600
=================URL===================
rtsp://your_ipaddr/6600
```

>  注意：your_ipaddr如果为0.0.0.0，请先配置网络，配置方法可参考如下：

```
ifconfig eth0 192.168.1.10 netmask 255.255.255.0
route add default gw 192.168.1.1
```

* 退出命令

  输入`q`即可退出demo

---