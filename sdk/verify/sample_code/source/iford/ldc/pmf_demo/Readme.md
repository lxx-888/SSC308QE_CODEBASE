# pmf_demo使用说明

---
## 一、demo场景
* ldc pmf 图像变换模式 pipeline
    ```
    vif->isp->ldc->scl->venc->rtsp

    参数说明： vif 接sensor pad0，isp/ldc/venc使用dev0 chn0,scl使用dev1 chn0,ldc 模块使用pmf模式

    绑定模式：均为framemode
    ```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/ldc/pmf_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_ldc_pmf_demo`获取可执行文件;

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

    场景可加载指定的iq bin文件，如不指定默认加载/config/iqfile/`sensorname`_api.bin文件；
    部分sensor的iq bin文件可在`/project/board/iford/iqfile`路径下获取。


---
## 四、运行说明
1. 将可执行文件prog_ldc_pmf_demo放到板子上，修改权限为777

   按`./prog_ldc_pmf_demo iqbin xxxiqbin `运行默认场景（ 透视变换） pmf mode pipeline。自定义参数说明：

   - move:  move [move x] [move y]  例如: move 100,100

   - rotate: rotate [angle]            例如: rotate 10

   - scale:  scale [scale x] [scale y]    例如: scale 0.6 1.2

   - comb1:  组合变换  中心旋转45度(只能应用于1920x1080)

   - comb2:  组合变换  镜像(只能应用于1920x1080)

2. 矩阵配置与运算

   - 原始的变换矩阵需要自己运算，得到原始的变换矩阵后，还需要经过逆置、量化、定点化，才能配置进LDC PMF的接口。变换运算的原理可以参考`CV_guide`中的`《ldc_user_guide》`doc，其中的`投影变换函数` 下的`矩阵使用说明`和`单一变换示例`小节。

   - 对于组合变换的矩阵运算，可以借鉴`投影变换函数` 下的` 组合变换计算示例`。

   - 此demo使用的矩阵运算库为经过裁剪第三方开源库：[Matrix_hub](https://github.com/Amoiensis/Matrix_hub/blob/master/README.md#matrix_hub)。


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
