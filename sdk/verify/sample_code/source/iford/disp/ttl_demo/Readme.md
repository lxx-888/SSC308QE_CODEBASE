# disp_ttl_demo使用说明

---

## 一、demo场景

* 场景1：点亮ttl屏

  ```
  参数说明： 输入分辨率为1024 * 600 的YUV420SP 图像点亮ttl屏
  ```


---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

   例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

   `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

2. 在kernel打开对应的TTL16需要的padmux;

   修改`kernel/arch/arm/boot/dts/iford-ssc029a-s01a-padmux.dtsi`中对应的`// TTL16`部分为`if 1`

   并将与`TTL16`中内容有冲突的部分关闭，修改为`if 0`

3. 在project目录执行以下命令进行编译;

   `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
   `make clean && make image -j8`

4. 进到sample_code目录执行`make clean && make source/iford/disp/ttl_demo`进行编译;

5. 到`sample_code/out/arm/app/prog_disp_ttl_demo`获取可执行文件;

---

## 三、运行环境说明

* 板端环境：

  在板端JP74座子上插入ttl转接板并连接ttl屏;


* 输入文件：

  /resource/input/YUV420SP_1024_600.yuv

---

## 四、运行说明

将可执行文件prog_disp_ttl_demo放到板子上，修改权限777

1.   按`./prog_disp_ttl_demo`运行场景；



---

## 五、运行结果说明

* 屏幕正常显示输入的YUV图像。


* 退出命令

  输入`q`即可退出demo

---
