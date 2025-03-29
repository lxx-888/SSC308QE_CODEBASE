# cv_server_demo使用说明

---

## 一、demo说明

demo用于配合cv tool进行视频预览和抓图

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/cv/server_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_cv_server_demo`获取可执行文件;

---

## 三、运行环境说明

* 板端环境：

    `SSC029A-S01A-S`型号板子sensor pad0对应CON24，sensor pad1对应CON25，sensor pad3对应CON22;

    在sensor pad0和sensor pad1位置接上mipi sensor，对应跑的是4+4lane，可以是imx415，imx307等;

* dts配置：

    mipi snr0+snr1 4+4lane EVB上使用，默认dts已配好，无需修改

  > 例如上述编译环境说明的`SSC029A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：

    /customer/demo.sh路径修改insmod对应的sensor ko，例如imx307跑双sensor时，可配置为：

  ```
  insmod /config/modules/5.10/imx307_MIPI.ko chmap=3 lane_num=4
  ```

* 输入文件：

    串流场景依赖json文件，默认加载demo目录下的x_snr.json，x为串流的路数；
    json文件可在demo resource路径下获取。

>  注意：实际应用中请替换IqFilePath指向的IQ文件路径。

* json说明：

    一路视频json如下所示，几路视频流就配置几个以下json配置:
```
    {
        "stVifCfg":
        {
            "snrResIdx":0,
            "padId": 0,
            "GroupId":0,
            "vifPortId":0
        },
        "stIqCfg":
        {
            "IqFilePath": "/config/iqfile/imx307_api.bin"
        },
        "stIspCfg":
        {
            "IspDevId":0,
            "IspChnId":0,
            "IspOutPortId":1,
            "Isp3DNRLevel":2
        },
        "stSclCfg":
        {
            "SclSkip": 0,
            "SclDevId":1,
            "SclChnId":0,
            "SclOutPortId":0,
            "SclOutPortW": 1920,
            "SclOutPortH": 1080
        },
        "stVencCfg":
        {
            "VencDev":0,
            "VencChn":0,
            "eType":2,
            "VencWidth": 1920,
            "VencHeight": 1080,
            "VencMaxWidth": 3840,
            "VencMaxHeight": 2160
        }
    },
```
    json详细配置说明如下:
    <table>
        <tr>
            <th colspan="2">参数</th>
            <th width=415 colspan="2">描述</th>
        </tr>
        <tr>
            <td width=100 rowspan="4">stVifCfg</td>
            <td>snrResIdx  </td>
            <td colspan="2">SENSOR分辨率映射表中的索引</td>
        </tr>
        <tr>
            <td>padId  </td>
            <td colspan="2">SENSOR设备号</td>
        </tr>
        <tr>
            <td>GroupId  </td>
            <td colspan="2">VIF Group号</td>
        </tr>
        <tr>
            <td>vifPortId</td>
            <td colspan="2">VIF输出端口号</td>
        </tr>
        <tr>
            <td rowspan="1"> stIqCfg</td>
            <td>IqFilePath</td>
            <td colspan="2">IQ文件路径</td>
        </tr>
        <tr>
            <td width=100 rowspan="4">stIspCfg</td>
            <td>IspDevId  </td>
            <td colspan="2">ISP设备号</td>
        </tr>
        <tr>
            <td>IspChnId</td>
            <td colspan="2">ISP通道号</td>
        </tr>
        <tr>
            <td>IspOutPortId</td>
            <td colspan="2">ISP输出端口号</td>
        </tr>
        <tr>
            <td>Isp3DNRLevel</td>
            <td colspan="2">ISP 3D降噪等级</td>
        </tr>
        <tr>
            <td width=100 rowspan="6">stSclCfg</td>
            <td>SclSkip</td>
            <td colspan="2">是否跳过SCL模块，设置为1表示ISP->Venc,否则ISP->SCL</td>
        </tr>
        <tr>
            <td>SclDevId  </td>
            <td colspan="2">SCL设备号</td>
        </tr>
        <tr>
            <td>SclChnId</td>
            <td colspan="2">SCL通道号</td>
        </tr>
        <tr>
            <td>SclOutPortId</td>
            <td colspan="2">SCL输出端口号</td>
        </tr>
        <tr>
            <td>SclOutPortW</td>
            <td colspan="2">SCL输出分辨率的宽分量</td>
        </tr>
        <tr>
            <td>SclOutPortH</td>
            <td colspan="2">SCL输出分辨率的高分量</td>
        </tr>
        <tr>
            <td width=100 rowspan="7">stVencCfg</td>
            <td>VencDev  </td>
            <td colspan="2">VENC设备号</td>
        </tr>
        <tr>
            <td>VencChn</td>
            <td colspan="2">VENC通道号</td>
        </tr>
        <tr>
            <td>eType</td>
            <td colspan="2">编码类型，2-H264 3-H265 4-JPEG</td>
        </tr>
        <tr>
            <td>VencWidth</td>
            <td colspan="2">编码图像宽度</td>
        </tr>
        <tr>
            <td>VencHeight</td>
            <td colspan="2">编码图像高度</td>
        </tr>
        <tr>
            <td>VencMaxWidth</td>
            <td colspan="2">编码图像最大宽度</td>
        </tr>
        <tr>
            <td>VencMaxHeight</td>
            <td colspan="2">编码图像最大高度</td>
        </tr>
    </table>


---

## 四、运行说明

将可执行文件prog_cv_server_demo和json文件放到板子上，demo修改权限777

1. 按`./prog_cv_server_demo`运行demo；
2. 打开cv tool输入板子ip点击连接，连接成功后点击start预览；

---

## 五、运行结果说明

* preview效果查看

正常出流会打印rtsp url，单sensor打印一个url，双sensor打印两个url，例如以下log：

成功后cv tool会自动获取链接并播放，播放成功在tool端可以可看到sensor画面。

```
rtsp venc dev0, chn 0, type 3, url 6600
=================URL===================
rtsp://your_ipaddr/6600
=================URL===================
Create Rtsp H265 Session, FPS: 30
rtsp venc dev0, chn 1, type 3, url 6601
=================URL===================
rtsp://your_ipaddr:555/6601
=================URL===================
Create Rtsp H265 Session, FPS: 30
```

>  注意：your_ipaddr如果为0.0.0.0，请先配置网络，配置方法可参考如下：

```
ifconfig eth0 192.168.1.10 netmask 255.255.255.0
route add default gw 192.168.1.1
```

* 退出命令

    输入`ctrl + c`即可退出demo

---