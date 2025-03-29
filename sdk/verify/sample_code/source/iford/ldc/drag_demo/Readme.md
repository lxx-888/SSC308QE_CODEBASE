# drag_demo使用说明


---
## 一、demo场景
* LDC区域拖动功能 pipeline
    ```
    +----------+   +---------------+   +----------------+
    | keyboard +-->+ LDC           +-->+ LDC            |
    | input    |   | SetChnLDCAttr |   | GetRegionPoint |
    +----------+   +---------+-----+   +------------+---+
                             |                      |
                             |                   +--+---+
                             |                   | RGN  |
                             |                   +--+---+
                             |                      |
                             |                      |
                             v                      v
    +-----+    +-----+    +--+--+    +-----+     +--+---+
    | VIF +--->+ ISP +--->+ LDC +--->+ SCL +---->+ VENC |
    +-----+    +-----+    +-----+    +-----+     +------+
    ```

    - 绑定模式：模块间绑定均为frame mode；
    - 场景为多线程运行，一个主线程运行pipeline：vif->isp->ldc->scl->venc；另一个线程中，从键盘读取控制字符，根据字符参数动态配置LDC属性，并获取两个映射区域间的映射坐标，给到RGN绘制OSD贴图到VENC，随rtsp播流出来；

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/ldc/drag_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_ldc_dragh_demo `获取可执行文件;

---
## 三、运行环境说明

> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

* 板端环境：
    以`SSC029A-S01A-S`型号板子为例说明场景板端环境：
    在SNR0(CON3)处插入鱼眼镜头

* dts配置：
    默认dts已经配好，无需进行修改

* sensor 驱动配置：
    /customer/demo.sh路径修改insmod对应的sensor ko，此demo以imx415为例，配置为：

    ```
    insmod /config/modules/5.10/imx415_MIPI.ko chmap=1
    ```

---
## 四、运行说明
将可执行文件prog_ldc_drag_demo放到板子上，修改权限777。

参考运行命令：`./prog_ldc_dragh_demo iqbin xxx.bin index 1 mode 1`

- input：

  - demo内默认加载Poly.bin：`source/iford/ldc/drag_demo/resource/input/ldc/drag/CalibPoly_new.bin`
  - IMX226的iq bin可于`alkaid/project/board/iford/iqfile`路径下获取, 若不指定, 则默认加载/config/iqfile/`sensorname`_api.bin文件

- 命令参数：

  - `index x`：The character ‘x’ specified sensor index
  - `iqbin x`：The character ‘x’ specified iq bin path

  - `mode 0`: P->O (360 degrees panoramic correction -> Uncorrected original image)
  - `mode 1`: R->O (normal correction -> Uncorrected original image)
  - `mode 2`: R->P (normal correction -> 360 degrees panoramic corrected image)


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
当程序可以正常预览时，可以通过键入字符：

`mode 0`， P->O模式：'w'、's'、'a'、'd' + 步进值（可选）+回车键 控制上下左右拖拽

`mode 1`， R->O & `mode 2`， R->P：'w'、's'、'a'、'd' + 步进值（可选）+回车键 控制上下左右拖拽，'j'、'k' + 步进值（可选）+回车键 控制视场角远近切换，'r'、't' + 步进值（可选）+回车键 控制虚拟镜头的旋转。

特别说明，步进单位可通过`控制字符`+`步进值`重新设定。


* 退出命令

    输入`q`即可退出demo

---
