# ldc_demo使用说明

---
## 一、demo场景
* ldc 畸变矫正模式 pipeline
    ```
    vif->isp->ldc->scl->venc->rtsp

    参数说明： vif 接sensor pad0，isp/ldc/venc使用dev0 chn0,scl使用dev1 chn0,ldc 模块使用ldc模式

    绑定模式：vif->isp、scl->venc为framemode；isp->ldc->scl为 hw autosync
    ```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/ldc/ldc_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_ldc_ldc_demo`获取可执行文件;

---
## 三、运行环境说明
> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

* 板端环境：

    `SSC029A-S01A-S`型号板子sensor pad0对应CON3;

    在sensor pad0位置接上mipi sensor，对应跑的是4lane，可以是imx415，imx307等;

* dts配置：
    mipi snr0+4lane EVB上使用，默认dts已配好，无需修改

    > 例如上述编译环境说明的`SSC029A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：
    /customer/demo.sh路径修改insmod对应的sensor ko，当前 demo可配置为：

    ```
    insmod /config/modules/5.10/imx415_MIPI.ko chmap=1 lane_num=4
    ```


* 输入文件：

    1. 需要将`source/iford/ldc/ldc_demo/resource/`文件夹放置于demo的运行路径，demo会读取：```./resource/input/CalibPoly_new.bin```，该文件描述了镜头的畸变信息，用于图像畸变矫正，不同镜头需要重新进行标定, 可通过工具`CVTool`生成，工具使用可参考相关文档。

    2. 场景可加载指定的iq bin文件，如不指定默认加载/config/iqfile/`sensorname`_api.bin文件；
部分sensor的iq bin文件可在`/project/board/iford/iqfile`路径下获取。


---
## 四、运行说明
将可执行文件prog_ldc_ldc_demo放到板子上，修改权限为777
1. 按`./prog_ldc_ldc_demo`运行场景 ldc mode pipeline；

   > 按`./prog_ldc_ldc_demo iqbin xxx`运行，可在场景上加载指定iqbin文件，xxx为iqbin文件路径
2. 运行demo后，会打印sensor可选择输入的res，输入你想选择preview的res（sensor resolution）


---
## 五、运行结果说明
1. preview效果查看

     正常出流会打印rtsp url，例如以下log：

   ```
   rtsp venc dev0, chn 0, type 3, url 6600
   =================URL===================
   rtsp://your_ipaddr/6600
   ```

    使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。

   > 注意：your_ipaddr如果为0.0.0.0，请先配置网络，配置方法可参考如下：

   ```
   ifconfig eth0 192.168.1.10 netmask 255.255.255.0
   route add default gw 192.168.1.1
   ```

2. 退出命令

   输入`q`即可退出demo

---
