# 1 概述

本文档用于介绍AOV场景下，MCU替换RTC进行休眠唤醒的软硬件方案MCU部分的实现。

本方案基于文档：  [SOC_power_scheme_zh.md ](../../../../../../../../project/release/docs/customer/Common/Development/SOC_power_scheme_zh.md)

MCU要实现两个功能：**SOC上下电** 、**唤醒SOC**



**MCU和SOC之间的引脚连接框图如下：**

![mcu_soc_pin_show](.\mymedia\mcu_soc_pin_show.svg)



**PB13/PB14** :    MCU给SOC的DRAM和Sensor上电引脚

**PA0** :                 MCU对SOC的大海上电/断电引脚

**PD11** :               MCU接收SOC的休眠通知引脚，下降沿中断

**PD3/PD4** :        用于标记PIR时间和用户预览的引脚

**PD6** :                 用于接收AOV处理完PIR或用户预览事件的通知

# 2 环境搭建

MCU已经实现了上图中的方案，源码在本目录下。需要将以下源码导入到keil 5 中：

- **Drivers** :               MCU芯片相关的driver源码,
- **Inc** :                      头文件路径
- **MF-Config** :         模板工程自带
- **Src** :                      MCU在AOV场景下休眠唤醒主要的源码



**推荐的源码管理**

 ![keil_files](.\mymedia\keil_files.svg)

源码怎么分布没有统一，编译无问题即可。



# 3 SOC上下电

MCU通过MCU_INT (PA0)给SOC上下电。



## 3.2 上电阶段

MCU在上电阶段，会拉高PA0、PB13、PB14，这样SOC四个电源在打开电源开关后即可上电。



## 3.3 下电阶段

在AOV场景下，原本RTC会在SOC休眠的时候拉低四个电源，换成MCU之后，需要SOC来告诉MCU何时应该关闭电源。因此

SOC会在休眠的时候通过拉低PAD_GPIO4，MCU收到这个信号的下降沿后进入中断，开启定时器定时休眠的时间，并且拉低MCU_INT来关闭SOC的大海电源。MCU上面会有两个LED灯亮起表示进入了休眠状态。

要注意的是，**这根休眠引脚不仅用于休眠，还用于连接到NMOS的G极，因此方案上这个引脚必须要选择默认内部上拉的引脚。**



# 4 唤醒SOC

## 4.1 timer唤醒

指的是MCU定时唤醒SOC,这是AOV场景中最基本的唤醒方式。

MCU的计时是从收到休眠通知开始，目前timer唤醒的定时时间在MCU内部写死1s唤醒一次，但是为了一秒多帧的需求，后续需要加入通信协议将这个时间由AOV app传递给MCU，知道本次需要休眠的时间。

timer唤醒在休眠时候关闭大海电源，在定时时间后打开大海电源，这要求SOC端拉低休眠引脚的时间尽可能接近休眠流程的末尾，否则可能出现MCU关闭大海电源SOC还没有走完休眠流程。



## 4.2 PIR、用户预览唤醒

参考以下框图，真实场景下PIR和用户预览是由PIR设备和WIFI决定，各自都会发出中断告知MCU，再由MCU向SOC传达这个事件。现在MCU实现的是按键中断来模拟PIR和WIFI中断。KEY1表示PIR中断，KEY4表示WIFI中断。

![PIR_Preview](.\mymedia\PIR_Preview.svg)

当按下KEY1的时候会延迟8ms打开大海电源，并且拉高PD3；

当按下KEY4的时候会延迟8ms打开大海电源，并且拉高PD4；

发生这两个事件的时候，会马上让SOC resume，然后AOV通过检测PAD_GPIO1和PAD_GPIO2来知道发生了什么事件，走对应的流程，处理完成后AOV拉低PAD_GPIO3告知事件已经处理完，MCU通过PD6的下降沿进入中断将PD3和PD4拉低，让下一次事件发生的时候可以再次传递。

SOC 下电未完成就上电会导致系统挂死，因此 MCU 在接到 PIR事件要延时一段时间等到硬件完全下电，延时时间通常是硬件下电时间，同时也作为了按键防抖的功能，目前这个时间是8ms。

实际上就是通过GPIO完成了一个简单交互。

