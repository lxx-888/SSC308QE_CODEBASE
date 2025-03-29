#!/bin/sh

#################### PORT IDX(default p0) ###################
USB_PORT_IDX="0"

#################### CONFIGFS PATH #####################
KERNEL_CONFIG_DIR=/sys/kernel/config
USB_GADGET_DIR=$KERNEL_CONFIG_DIR/usb_gadget
USB_DEVICE_DIR=$USB_GADGET_DIR/$UDC
USB_CONFIGS_DIR=$USB_DEVICE_DIR/configs/default.1
USB_FUNCTIONS_DIR=$USB_DEVICE_DIR/functions

#################### DEVICE INFO ARRAY ###################
USB_DEVICE_VID_ARRAY="0x1d69,0x1d69 "
USB_DEVICE_PID_ARRAY="0x0101,0x0102"
MANUFACTURER_ARRAY="SigmaStar,SigmaStar"
PRODUCT_ARRAY="USB Gadget,USB Gadget"
SERIAL_NUM_ARRAY="0123,abcd"
DEVICE_ClASS_ARRAY="0xef,0xef"
DEVICE_PROTOCOL_ARRAY="0x01,0x01"
DEVUCE_SUBCLASS_ARRAY="0x02,0x02"
MAXPKACKET_SIZE0_ARRAY="0x40,0x40"
BCD_DEVICE_ARRAY="0x0419,0x0419"
BCD_USB_ARRAY="0x0200,0x0200"


#################### CONFIG INFO ###################
CONFIGURATION_ARRAY="composite device,composite device"
MAXPOWER_ARRAY="2,2"
REMOTE_WAKEUP_ARRAY="0,0"
SELF_POWERED_ARRAY="1,1"

#################### FUNCTION DEFAULT ENABLE ###################
NO_FUNCTION=0
# stream number
UVC_FUNCTION_ENABLE=0
# 1:capture 2:playback 3:both
UAC_FUNCTION_ENABLE=0
RNDIS_FUNCTION_ENABLE=0
DFU_FUNCTION_ENABLE=0
# 1:keyboard 2:mouse 3:both, 4: vendor com
HID_FUNCTION_ENABLE=0
ACM_FUNCTION_ENABLE=0
SERIAL_FUNCTION_ENABLE=0
MASS_STORAGE_FUNCTION_ENABLE=0
ADB_FUNCTION_ENABLE=0
PRINTER_FUNCTION_ENABLE=0
ACCESSORY_FUNCTION_ENABLE=0
ECM_FUNCTION_ENABLE=0
NCM_FUNCTION_ENABLE=0
EEM_FUNCTION_ENABLE=0
SOURCE_SINK_FUNCTION_ENABLE=0
LOOPBACK_FUNCTION_ENABLE=0
MTP_FUNCTION_ENABLE=0

#################### UVC ###################
# maxpkt=ep_maxpkt*(mult+1), ex:3072=1024*(2+1), 1024=1024*(0+1)
UVC_STREAMING_MAX_PACKET_ARRAY="3072,1024,3072,3072,3072,3072"
UVC_STREAMING_MAX_BURST_ARRAY="13,13,13,13,0,0"
UVC_STREAMING_INTERVAL_ARRAY="1,1,1,1,1,1"
UVC_STREAMING_NAME_ARRAY="UVC Camera 0,UVC Camera 1,UVC Camera 2,UVC Camera 3,UVC Camera 4,UVC Camera 5"
UVC_STREAMING_FORMAT_YUV_ENABLE=1
UVC_STREAMING_FORMAT_NV12_ENABLE=0
UVC_STREAMING_FORMAT_MJPEG_ENABLE=1
UVC_STREAMING_FORMAT_H264_ENABLE=0
UVC_STREAMING_FORMAT_H265_ENABLE=0
UVC_VERSION=0x0110  #uvc1.1:0x0110; uvc1.5:0x0150
UVC_MODE=0x0   #ISO:0,  BULK:1
UVC_NEED_CONFIG_FRAME_INDEX=0

#################### UAC ###################
UAC_IN_SAMPLE_RATE=48000
UAC_IN_SAMPLE_SIZE=2
UAC_IN_CHANNEL_MASK=0x3
UAC_IN_MAXPKT_SIZE=200
UAC_IN_CTRL_VOLUME=1
UAC_IN_CTRL_VOLUME_MAX=0
UAC_IN_CTRL_VOLUME_MIN=-25600  # -100db
UAC_IN_CTRL_VOLUME_RES=256 #1db
UAC_IN_CTRL_MUTE=1
UAC_OUT_SAMPLE_RATE=48000
UAC_OUT_SAMPLE_SIZE=2
UAC_OUT_CHANNEL_MASK=0x3
UAC_OUT_MAXPKT_SIZE=200
UAC_OUT_CTRL_VOLUME=1
UAC_OUT_CTRL_VOLUME_MAX=0  #db
UAC_OUT_CTRL_VOLUME_MIN=-25600   # -100db
UAC_OUT_CTRL_VOLUME_RES=256   #1db
UAC_OUT_CTRL_MUTE=1

#################### mass storage ###################
STORAGE_LUN_FILE_ARRAY="/tmp/disk1.img,/customer/disk2.img,/customer/disk3.img"
STORAGE_LUN_NUM_ARRAY="1,1,1"
STORAGE_LUN_BS_ARRAY="1M,1M,1M"
STORAGE_LUN_COUNT_ARRAY="50,50,50"

#################### ADB ###################
ADBD_PATH="/mnt/configfs/64adbd"

#################### MTP ###################
MTP_PATH="/mnt/configfs/64_umtprd"

configfs_mkdir()
{
    mkdir $1 $2
    if [ $? -ne 0 ]; then
        echo "usb function config failed!"
        exit
    fi
}

config_clear()
{
    if [ ! -d $USB_DEVICE_DIR ];then
        return
    fi

    if [ -n "$(cat ${USB_DEVICE_DIR}/UDC)" ];then
        echo "none" > ${USB_DEVICE_DIR}/UDC
    fi
    if [ -e $USB_CONFIGS_DIR ]; then
        for i in `ls ${USB_CONFIGS_DIR}/ | grep ".instance"`
        do
            rm ${USB_CONFIGS_DIR}/$i
        done
        for i in `ls ${USB_CONFIGS_DIR}/ | grep "ffs."`
        do
            rm ${USB_CONFIGS_DIR}/$i
        done
    fi
    #clear for adb
    if [ -e /dev/usb-ffs/adb ]; then
        start-stop-daemon --stop --quiet --background --exec $ADBD_PATH
        umount /dev/usb-ffs/adb
        rmdir /dev/usb-ffs/adb
        if [ -d  ${USB_CONFIGS_DIR}/ffs.adb ]; then
            rm ${USB_CONFIGS_DIR}/ffs.adb
        fi
        if [ -d  ${USB_FUNCTIONS_DIR}/ffs.adb ]; then
            rmdir ${USB_FUNCTIONS_DIR}/ffs.adb
        fi
    fi

    #clear for mtp
    if [ -e /dev/usb-ffs/mtp ]; then
        start-stop-daemon --stop --quiet --background --exec $MTP_PATH
        umount /dev/usb-ffs/mtp
        rmdir /dev/usb-ffs/mtp
        if [ -d  ${USB_CONFIGS_DIR}/ffs.mtp ]; then
            rm ${USB_CONFIGS_DIR}/ffs.mtp
        fi
        if [ -d  ${USB_FUNCTIONS_DIR}/ffs.mtp ]; then
            rmdir ${USB_FUNCTIONS_DIR}/ffs.mtp
        fi
    fi

    if [ -d $USB_CONFIGS_DIR/strings/0x409 ]; then
        rmdir  $USB_CONFIGS_DIR/strings/0x409
    fi
    if [ -d $USB_CONFIGS_DIR ]; then
        rmdir  $USB_CONFIGS_DIR
    fi

    for i in `ls $USB_FUNCTIONS_DIR | grep .instance`; do
        if [ -n "$(echo $i | grep uvc)" ]; then
            rm -f $USB_FUNCTIONS_DIR/$i/control/class/fs/default
            rm -f $USB_FUNCTIONS_DIR/$i/control/class/ss/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/class/fs/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/class/hs/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/class/ss/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/yuyv
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/nv12
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/mjpeg
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/h264
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/h265

            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/yuyv ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/yuyv/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/yuyv
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/nv12 ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/nv12/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/nv12
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/mjpeg/default ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/mjpeg/default/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/mjpeg/default
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/framebase/h264 ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h264/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h264
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/framebase/h265 ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h265/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h265
            fi

            if [ -d $USB_FUNCTIONS_DIR/$i/control/header/default ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/control/header/default
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/header/default ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/header/default
            fi
        elif [ -n "$(echo $i | grep mass_storage)" ]; then
            for j in `ls $USB_FUNCTIONS_DIR/$i | grep -E 'lun\.[^0]'`; do
                rmdir $USB_FUNCTIONS_DIR/$i/$j
            done
        fi
        rmdir $USB_FUNCTIONS_DIR/$i
    done

    for i in `ls $USB_FUNCTIONS_DIR | grep ffs.`; do
        rmdir $USB_FUNCTIONS_DIR/$i
    done

    if [ -d $USB_DEVICE_DIR/strings/0x409 ]; then
        rmdir  $USB_DEVICE_DIR/strings/0x409
    fi
    rmdir $USB_DEVICE_DIR
    #if [ -d $USB_GADGET_DIR ]
    #then
    #    umount $KERNEL_CONFIG_DIR
    #fi
}

###########################################UVC CONFIG######################################################
# bBitsPerPixel(uncompressed/framebase)/bDefaultFrameIndex/bmaControls/guidFormat(uncompressed/framebase)
config_uvc_format_yuyv()
{
    UVC_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv
    echo 0x10 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/bDefaultFrameIndex
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/bmaControls
    echo -ne \\x59\\x55\\x59\\x32\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/guidFormat
}

config_uvc_format_nv12()
{
    UVC_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12
    echo 0x0C > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/bDefaultFrameIndex
    echo 0x04 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/bmaControls
    echo -ne \\x4e\\x56\\x31\\x32\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/guidFormat
}

config_uvc_format_mjpeg()
{
    UVC_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/bDefaultFrameIndex
    echo 0x04 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/bmaControls
}

config_uvc_format_h264()
{
    UVC_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264
    echo 0x10 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/bDefaultFrameIndex
    echo 0x04 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/bmaControls
    echo -ne \\x48\\x32\\x36\\x34\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/guidFormat
}

config_uvc_format_h265()
{
    UVC_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265
    echo 0x10 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/bDefaultFrameIndex
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/bmaControls
    echo -ne \\x48\\x32\\x36\\x35\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/guidFormat
}

# bmCapabilities/dwDefaultFrameInterval/dwFrameInterval/dwMaxBitRate/dwMaxVideoFrameBufferSize(uncompressed/mjpeg)/dwMinBitRate/wHeight/wWidth
config_uvc_frame_yuyv()
{
    UVC_STREAM_INDEX=$1
    UVC_FRAME_WIDTH=$2
    UVC_FRAME_HEIGHT=$3
    UVC_FRAME_INTERVAL=$4
    UVC_FRAME_INDEX=$5

    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p
    if [ $UVC_NEED_CONFIG_FRAME_INDEX == 1 ]; then
        echo $UVC_FRAME_INDEX >  ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/bFrameIndex
    fi
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    if [ $UVC_FRAME_WIDTH -lt 2560 ];then
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    else
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    fi
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*2)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwMaxVideoFrameBufferSize
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_nv12()
{
    UVC_STREAM_INDEX=$1
    UVC_FRAME_WIDTH=$2
    UVC_FRAME_HEIGHT=$3
    UVC_FRAME_INTERVAL=$4
    UVC_FRAME_INDEX=$5

    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p
    if [ $UVC_NEED_CONFIG_FRAME_INDEX == 1 ]; then
        echo $UVC_FRAME_INDEX >  ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/bFrameIndex
    fi
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    if [ $UVC_FRAME_WIDTH -lt 2560 ];then
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*12*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    else
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*12*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    fi
    echo | awk "{print $UVC_FRAME_WIDTH*$UVC_FRAME_HEIGHT*3/2}" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwMaxVideoFrameBufferSize
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*12*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_mjpeg()
{
    UVC_STREAM_INDEX=$1
    UVC_FRAME_WIDTH=$2
    UVC_FRAME_HEIGHT=$3
    UVC_FRAME_INTERVAL=$4
    UVC_FRAME_INDEX=$5

    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p
    if [ $UVC_NEED_CONFIG_FRAME_INDEX == 1 ]; then
        echo $UVC_FRAME_INDEX >  ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/bFrameIndex
    fi
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    if [ $UVC_FRAME_WIDTH -lt 2560 ];then
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    else
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    fi
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*2)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwMaxVideoFrameBufferSize
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_h264()
{
    UVC_STREAM_INDEX=$1
    UVC_FRAME_WIDTH=$2
    UVC_FRAME_HEIGHT=$3
    UVC_FRAME_INTERVAL=$4
    UVC_FRAME_INDEX=$5

    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p
    if [ $UVC_NEED_CONFIG_FRAME_INDEX == 1 ]; then
        echo $UVC_FRAME_INDEX >  ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/bFrameIndex
    fi
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    if [ $UVC_FRAME_WIDTH -lt 2560 ];then
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    else
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    fi
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_h265()
{
    UVC_STREAM_INDEX=$1
    UVC_FRAME_WIDTH=$2
    UVC_FRAME_HEIGHT=$3
    UVC_FRAME_INTERVAL=$4
    UVC_FRAME_INDEX=$5

    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p
    if [ $UVC_NEED_CONFIG_FRAME_INDEX == 1 ]; then
        echo $UVC_FRAME_INDEX >  ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/bFrameIndex
    fi
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    if [ $UVC_FRAME_WIDTH -lt 2560 ];then
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    else
        echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    fi
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_yuv()
{
    UVC_STREAM_INDEX=$1
    config_uvc_format_yuyv $UVC_STREAM_INDEX
    config_uvc_frame_yuyv $UVC_STREAM_INDEX 320 240 333333 1
    config_uvc_frame_yuyv $UVC_STREAM_INDEX 640 480 333333 2
    config_uvc_frame_yuyv $UVC_STREAM_INDEX 1280 720 333333 3
    config_uvc_frame_yuyv $UVC_STREAM_INDEX 1920 1080 333333 4
    config_uvc_frame_yuyv $UVC_STREAM_INDEX 2560 1440 333333 5
    config_uvc_frame_yuyv $UVC_STREAM_INDEX 3840 2160 666666 6
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/yuyv
}

config_uvc_nv12()
{
    UVC_STREAM_INDEX=$1
    config_uvc_format_nv12 $UVC_STREAM_INDEX
    config_uvc_frame_nv12 $UVC_STREAM_INDEX 320 240 333333 1
    config_uvc_frame_nv12 $UVC_STREAM_INDEX 640 480 333333 2
    config_uvc_frame_nv12 $UVC_STREAM_INDEX 1280 720 333333 3
    config_uvc_frame_nv12 $UVC_STREAM_INDEX 1920 1080 333333 4
    config_uvc_frame_nv12 $UVC_STREAM_INDEX 2560 1440 333333 5
    config_uvc_frame_nv12 $UVC_STREAM_INDEX 3840 2160 666666 6
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/nv12
}

config_uvc_mjpeg()
{
    UVC_STREAM_INDEX=$1
    config_uvc_format_mjpeg $UVC_STREAM_INDEX
    config_uvc_frame_mjpeg $UVC_STREAM_INDEX 320 240 333333 1
    config_uvc_frame_mjpeg $UVC_STREAM_INDEX 640 480 333333 2
    config_uvc_frame_mjpeg $UVC_STREAM_INDEX 1280 720 333333 3
    config_uvc_frame_mjpeg $UVC_STREAM_INDEX 1920 1080 333333 4
    config_uvc_frame_mjpeg $UVC_STREAM_INDEX 2560 1440 333333 5
    config_uvc_frame_mjpeg $UVC_STREAM_INDEX 3840 2160 333333 6
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/mjpeg
}

config_uvc_h264()
{
    UVC_STREAM_INDEX=$1
    config_uvc_format_h264 $UVC_STREAM_INDEX
    config_uvc_frame_h264 $UVC_STREAM_INDEX 320 240 333333 1
    config_uvc_frame_h264 $UVC_STREAM_INDEX 640 480 333333 2
    config_uvc_frame_h264 $UVC_STREAM_INDEX 1280 720 333333 3
    config_uvc_frame_h264 $UVC_STREAM_INDEX 1920 1080 333333 4
    config_uvc_frame_h264 $UVC_STREAM_INDEX 2560 1440 333333 5
    config_uvc_frame_h264 $UVC_STREAM_INDEX 3840 2160 333333 6
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/h264
}

config_uvc_h265()
{
    UVC_STREAM_INDEX=$1
    config_uvc_format_h265 $UVC_STREAM_INDEX
    config_uvc_frame_h265 $UVC_STREAM_INDEX 320 240 333333 1
    config_uvc_frame_h265 $UVC_STREAM_INDEX 640 480 333333 2
    config_uvc_frame_h265 $UVC_STREAM_INDEX 1280 720 333333 3
    config_uvc_frame_h265 $UVC_STREAM_INDEX 1920 1080 333333 4
    config_uvc_frame_h265 $UVC_STREAM_INDEX 2560 1440 333333 5
    config_uvc_frame_h265 $UVC_STREAM_INDEX 3840 2160 333333 6
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/h265
}


config_uvc()
{
    UVC_STREAM_INDEX=$1
    UVC_STREAMING_MAX_PACKET=`echo $UVC_STREAMING_MAX_PACKET_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`
    UVC_STREAMING_MAX_BURST=`echo $UVC_STREAMING_MAX_BURST_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`
    UVC_STREAMING_INTERVAL=`echo $UVC_STREAMING_INTERVAL_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`
    UVC_STREAMING_NAME=`echo $UVC_STREAMING_NAME_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`

    # 配置speed/name
    # streaming_maxpacket/streaming_maxburst/streaming_interval/bulk_streaming_ep
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}
    echo $UVC_STREAMING_MAX_PACKET > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_maxpacket
    echo $UVC_STREAMING_MAX_BURST > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_maxburst
    echo $UVC_STREAMING_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_interval
    echo -n $UVC_STREAMING_NAME > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_name
    echo $UVC_MODE > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/bulk_streaming_ep

    # 配置control
    # bcdUVC/dwClockFrequency
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default
    echo $UVC_VERSION > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default/bcdUVC
    echo 0x02DC6C00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default/dwClockFrequency
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/class/fs/default
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/class/ss/default

    # 配置streaming
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default

    if [ $UVC_STREAMING_FORMAT_MJPEG_ENABLE -gt 0 ]; then
        config_uvc_mjpeg $UVC_STREAM_INDEX
    fi
    if [ $UVC_STREAMING_FORMAT_YUV_ENABLE -gt 0 ]; then
        config_uvc_yuv $UVC_STREAM_INDEX
    fi
    if [ $UVC_STREAMING_FORMAT_NV12_ENABLE -gt 0 ]; then
        config_uvc_nv12 $UVC_STREAM_INDEX
    fi
    if [ $UVC_STREAMING_FORMAT_H264_ENABLE -gt 0 ]; then
        config_uvc_h264 $UVC_STREAM_INDEX
    fi
    if [ $UVC_STREAMING_FORMAT_H265_ENABLE -gt 0 ]; then
        config_uvc_h265 $UVC_STREAM_INDEX
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/class/fs/default
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/class/hs/default
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/class/ss/default
}

###########################################UAC CONFIG######################################################
config_uac_in_enable()
{
    echo $UAC_IN_SAMPLE_RATE > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_srate
    echo $UAC_IN_SAMPLE_SIZE > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_ssize
    echo $UAC_IN_CHANNEL_MASK > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_chmask
    echo $UAC_IN_MAXPKT_SIZE > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_mpsize

    if [ -f ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_present ]; then
        echo $UAC_IN_CTRL_VOLUME > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_present
    fi
    if [ -f  ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_max ]; then
        echo $UAC_IN_CTRL_VOLUME_MAX > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_max
    fi
    if [ -f  ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_min ]; then
        echo $UAC_IN_CTRL_VOLUME_MIN > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_min
    fi
    if [ -f  ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_res ]; then
        echo $UAC_IN_CTRL_VOLUME_RES > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_volume_res
    fi
    if [ -f  ${USB_FUNCTIONS_DIR}/uac1.instance0/p_mute_present ]; then
        echo $UAC_IN_CTRL_MUTE > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_mute_present
    fi
}

config_uac_in_disable()
{
    echo 0 > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_chmask
}

config_uac_out_enable()
{
    echo $UAC_OUT_SAMPLE_RATE > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_srate
    echo $UAC_OUT_SAMPLE_SIZE > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_ssize
    echo $UAC_OUT_CHANNEL_MASK > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_chmask
    echo $UAC_OUT_MAXPKT_SIZE > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_mpsize
    if [ -f ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_present ]; then
        echo $UAC_OUT_CTRL_VOLUME > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_present
    fi
    if [ -f ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_max ]; then
        echo $UAC_OUT_CTRL_VOLUME_MAX > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_max
    fi
    if [ -f ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_min ]; then
        echo $UAC_OUT_CTRL_VOLUME_MIN > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_min
    fi
    if [ -f ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_res ]; then
        echo $UAC_OUT_CTRL_VOLUME_RES > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_volume_res
    fi
    if [ -f ${USB_FUNCTIONS_DIR}/uac1.instance0/c_mute_present ]; then
        echo $UAC_OUT_CTRL_MUTE > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_mute_present
    fi
}

config_uac_out_disable()
{
    echo 0 > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_chmask
}

config_uac()
{
    # audio_play_mode/req_number
    # c_srate/c_chmask/c_ssize/c_mpsize
    # p_srate/p_chmask/p_ssize/p_mpsize
    configfs_mkdir ${USB_FUNCTIONS_DIR}/uac1.instance0
    # for before uac mode set
    if [ -f ${USB_FUNCTIONS_DIR}/uac1.instance0/audio_play_mode ]; then
        echo $UAC_FUNCTION_ENABLE > ${USB_FUNCTIONS_DIR}/uac1.instance0/audio_play_mode
    fi
    echo 8 > ${USB_FUNCTIONS_DIR}/uac1.instance0/req_number
    if [ $UAC_FUNCTION_ENABLE == 1 ]; then
        config_uac_out_enable
        config_uac_in_disable
    elif [ $UAC_FUNCTION_ENABLE == 2 ]; then
        config_uac_in_enable
        config_uac_out_disable
    else
        config_uac_in_enable
        config_uac_out_enable
    fi
}

###########################################RNDIS CONFIG####################################################
config_rndis()
{
    # dev_addr/host_addr/qmult/class/subclass/protocol
    configfs_mkdir ${USB_FUNCTIONS_DIR}/rndis.instance0
}

###########################################DFU CONFIG######################################################
config_dfu()
{
    configfs_mkdir ${USB_FUNCTIONS_DIR}/dfu.instance0
}

###########################################HID CONFIG######################################################
config_hid()
{
    # protocol/report_desc/report_length/subclass
    case $HID_FUNCTION_ENABLE in
    1)
        # keyboard
        configfs_mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x01 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne \\x05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x03\\x95\\x05\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x05\\x91\\x02\\x95\\x01\\x75\\x03\\x91\\x03\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x3F > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        ;;
    2)
        # mouse
        configfs_mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x02 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne \\x05\\x01\\x09\\x02\\xa1\\x01\\x09\\x01\\xa1\\x00\\x05\\x09\\x19\\x01\\x29\\x03\\x15\\x00\\x25\\x01\\x95\\x08\\x75\\x01\\x81\\x02\\x05\\x01\\x09\\x30\\x09\\x31\\x09\\x38\\x15\\x81\\x25\\x7f\\x75\\x08\\x95\\x03\\x81\\x06\\xc0\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x2E > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        ;;
    3)
        # commuication
        configfs_mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne   \\x06\\x00\\xFF\\x09\\x01\\xa1\\x01\\x09\\x03\\x15\\x00\\x26\\x00\\xFF\\x75\\x08\\x95\\x40\\x81\\x02\\x09\\x04\\x15\\x00\\x26\\x00\\xFF\\x75\\x08\\x95\\x40\\x91\\x02\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x40 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        ;;
    4)
        # POS
        configfs_mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne   \\x05\\x8c\\x09\\x02\\xa1\\x01\\x09\\x12\\xa1\\x02\\x85\\x02\\x15\\x00\\x26\\xff\\x00\\x75\\x08\\x95\\x01\\x05\\x01\\x09\\x3b\\x81\\x02\\x95\\x03\\x05\\x8c\\x09\\xfb\\x09\\xfc\\x09\\xfd\\x81\\x02\\x95\\x38\\x09\\xfe\\x82\\x02\\x01\\x06\\x66\\xff\\x95\\x02\\x09\\x04\\x09\\x00\\x81\\x02\\x05\\x8c\\x25\\x01\\x75\\x01\\x95\\x08\\x09\\xff\\x81\\x02\\xc0\\x09\\x14\\xa1\\x02\\x85\\x04\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x09\\x00\\x09\\x5f\\x09\\x60\\x09\\x0\\x09\\x00\\x09\\x85\\x09\\x86\\x91\\x86\\xc0\\x06\\x66\\xff\\x75\\x08\\x26\\xff\\x00\\x09\\x03\\xa1\\x02\\x85\\xfe\\x95\\x01\\x09\\x03\\xb2\\x82\\x01\\xc0\\x09\\x01\\xa1\\x02\\x85\\xfd\\x95\\x01\\x09\\x21\\x91\\x02\\x95\\x3e\\x09\\x22\\x92\\x82\\x01\\xc0\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x40 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        ;;
    5)
        # TOUCH
        configfs_mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 2 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo 1 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        echo 13 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo -ne \\x05\\x0d\\x09\\x04\\xa1\\x01\\x85\\xaa\\x09\\x20\\xa1\\x00\\x09\\x42\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x01\\x81\\x02\\x95\\x03\\x81\\x03\\x09\\x32\\x09\\x37\\x95\\x02\\x81\\x02\\x95\\x0a\\x81\\x03\\x05\\x01\\x26\\xff\\x7f\\x75\\x10\\x95\\x01\\xa4\\x55\\x0d\\x65\\x00\\x09\\x30\\x35\\x00\\x46\\x00\\x00\\x81\\x02\\x09\\x31\\x46\\x00\\x00\\x81\\x02\\xb4\\x05\\x0d\\x09\\x60\\x09\\x61\\x95\\x02\\x81\\x02\\x95\\x01\\x81\\x03\\xc0\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
    esac
}

###########################################CDC ACM CONFIG###################################################
config_acm()
{
    ACM_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/acm.instance${ACM_STREAM_INDEX}
}

###########################################GENERIC SERIAL CONFIG###################################################
config_gserial()
{
    SERIAL_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/gser.instance${SERIAL_STREAM_INDEX}
}

###########################################MSC CONFIG######################################################
config_mass_storage()
{
    STORAGE_STREAM_INDEX=$1
    STORAGE_LUN_FILE=`echo $STORAGE_LUN_FILE_ARRAY | awk -F ',' '{print $'$((STORAGE_STREAM_INDEX+1))'}'`
    STORAGE_LUN_NUM=`echo $STORAGE_LUN_NUM_ARRAY | awk -F ',' '{print $'$((STORAGE_STREAM_INDEX+1))'}'`
    STORAGE_LUN_BS=`echo $STORAGE_LUN_BS_ARRAY | awk -F ',' '{print $'$((STORAGE_STREAM_INDEX+1))'}'`
    STORAGE_LUN_COUNT=`echo $STORAGE_LUN_COUNT_ARRAY | awk -F ',' '{print $'$((STORAGE_STREAM_INDEX+1))'}'`
    #file/ro/removable/cdrom/nofua/"inquiry_string"
    if [ ! -f "$STORAGE_LUN_FILE" ]; then
        dd if=/dev/zero of="$STORAGE_LUN_FILE" bs="$STORAGE_LUN_BS" count="$STORAGE_LUN_COUNT"
        mkdosfs $STORAGE_LUN_FILE
    fi
    configfs_mkdir ${USB_FUNCTIONS_DIR}/mass_storage.instance${STORAGE_STREAM_INDEX}
    # code default exist lun.0
    echo "$STORAGE_LUN_FILE" > ${USB_FUNCTIONS_DIR}/mass_storage.instance${STORAGE_STREAM_INDEX}/lun.0/file

    LUN_IDX=1;
    while [ $LUN_IDX -lt $STORAGE_LUN_NUM ]
    do
        configfs_mkdir ${USB_FUNCTIONS_DIR}/mass_storage.instance${STORAGE_STREAM_INDEX}/lun.$LUN_IDX
        echo "$STORAGE_LUN_FILE" > ${USB_FUNCTIONS_DIR}/mass_storage.instance${STORAGE_STREAM_INDEX}/lun.$LUN_IDX/file
        let LUN_IDX++
    done
}

###########################################ADB CONFIG######################################################
config_adb()
{
    #no attributes, all parameters are set through FunctioFS
    configfs_mkdir ${USB_FUNCTIONS_DIR}/ffs.adb
}

###########################################MTP CONFIG######################################################
config_mtp()
{
    #no attributes, all parameters are set through FunctioFS
    configfs_mkdir ${USB_FUNCTIONS_DIR}/ffs.mtp
}

###########################################PRINTER CONFIG###################################################
config_printer()
{
    PRINTER_STREAM_INDEX=$1
    configfs_mkdir ${USB_FUNCTIONS_DIR}/printer.instance${PRINTER_STREAM_INDEX}
}

###########################################ACCESSORY CONFIG###################################################
config_accessory()
{
    configfs_mkdir ${USB_FUNCTIONS_DIR}/accessory.instance0
}

###########################################CDC ECM##################################################
config_ecm()
{
    configfs_mkdir ${USB_FUNCTIONS_DIR}/ecm.instance0
}

###########################################CDC NCM##################################################
config_ncm()
{
    configfs_mkdir ${USB_FUNCTIONS_DIR}/ncm.instance0
}

###########################################CDC EEM##################################################
config_eem()
{
    configfs_mkdir ${USB_FUNCTIONS_DIR}/eem.instance0
}

###########################################SOURCE SINK##################################################
config_source_sink()
{
    configfs_mkdir ${USB_FUNCTIONS_DIR}/SourceSink.instance0
    echo 30720 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/bulk_buflen
    echo 3072 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/iso_buflen
    echo 64 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/int_buflen
    echo 0 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/pattern
    echo 1 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/buflen_vary
    echo 1 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/int_qlen
    echo 1 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/bulk_qlen
    echo 1 > ${USB_FUNCTIONS_DIR}/SourceSink.instance0/iso_qlen
}

###########################################LOOPBACK##################################################
config_loopback()
{
    configfs_mkdir ${USB_FUNCTIONS_DIR}/Loopback.instance0
}

usage()
{
    echo -e "Usage:./gadget-configfs.sh -x num1 [ -y num2 ] ...\n"
    echo -e "\t -a        UVC"
    echo -e "\t -b        UAC"
    echo -e "\t -c        RNDIS"
    echo -e "\t -d        DFU"
    echo -e "\t -e        HID"
    echo -e "\t -f        CDC ACM"
    echo -e "\t -g        MASS STORAGE"
    echo -e "\t -h        ADB(adbd path:$ADBD_PATH)"
    echo -e "\t -i        PRINTER"
    echo -e "\t -j        ACCESSORY"
    echo -e "\t -k        GENERIC SERIAL"
    echo -e "\t -l        CDC ECM"
    echo -e "\t -m        CDC NCM"
    echo -e "\t -n        CDC EEM"
    echo -e "\t -o        SOURCE SINK"
    echo -e "\t -p        LOOPBACK"
    echo -e "\t -q        MTP"
    echo -e "\t -u        SPECIFY USB PORT(default Port 0)"
    echo -e "\t -z        NO FUNCTION(Clear)"
    echo -e "\t Examples: ./gadget-configfs.sh -a1         ---> UVC"
    echo -e "\t           ./gadget-configfs.sh -a1 -b1     ---> UVC+UAC"
    echo -e "\t           ./gadget-configfs.sh -f2         ---> CDC_ACM_1 + CDC_ACM_2"
    echo -e "\t           ./gadget-configfs.sh -e1         ---> HID(keyboard)"
    echo -e "\t           ./gadget-configfs.sh -e2         ---> HID(mouse)"
    echo -e "\t           ./gadget-configfs.sh -g2         ---> MASS_STORAGE_1 + MASS_STORAGE_2"
    echo -e "\t           ./gadget-configfs.sh -b1 -e1 -h1 ---> UAC+HID(keyboard)+ADB"
    echo
    echo -e "\e[32m\t Note: multi functions need depend on chip's ep number and fifo size\e[0m"

}

main()
{
    while getopts ":a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:u:z" opt
    do
        case $opt in
        a)
            UVC_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mUVC:$UVC_FUNCTION_ENABLE\e[0m"
            ;;
        b)
            UAC_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mUAC:$UAC_FUNCTION_ENABLE\e[0m"
            ;;
        c)
            RNDIS_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mRNDIS:$RNDIS_FUNCTION_ENABLE\e[0m"
            ;;
        d)
            DFU_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mDFU:$DFU_FUNCTION_ENABLE\e[0m"
            ;;
        e)
            HID_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mHID:$HID_FUNCTION_ENABLE\e[0m"
            ;;
        f)
            ACM_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mCDC ACM:$ACM_FUNCTION_ENABLE\e[0m"
            ;;
        g)
            MASS_STORAGE_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mMASS STORAGE:$MASS_STORAGE_FUNCTION_ENABLE\e[0m"
            ;;
        h)
            ADB_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mADB:$ADB_FUNCTION_ENABLE\e[0m"
            ;;
        i)
            PRINTER_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mPRINTER:$PRINTER_FUNCTION_ENABLE\e[0m"
            ;;
        j)
            ACCESSORY_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mACCESSORY:$ACCESSORY_FUNCTION_ENABLE\e[0m"
            ;;
        k)
            SERIAL_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mGENERIC SERIAL:$SERIAL_FUNCTION_ENABLE\e[0m"
            ;;
        l)
            ECM_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mCDC ECM:$ECM_FUNCTION_ENABLE\e[0m"
            ;;
        m)
            NCM_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mCDC NCM:$NCM_FUNCTION_ENABLE\e[0m"
            ;;
        n)
            EEM_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mCDC EEM:$EEM_FUNCTION_ENABLE\e[0m"
            ;;
        o)
            SOURCE_SINK_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mSOURCE SINK:$SOURCE_SINK_FUNCTION_ENABLE\e[0m"
            ;;
        p)
            LOOPBACK_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mLOOPBACK:$LOOPBACK_FUNCTION_ENABLE\e[0m"
            ;;
        q)
            MTP_FUNCTION_ENABLE=$OPTARG
            echo -e "\e[32mMTP:$MTP_FUNCTION_ENABLE\e[0m"
            ;;
        u)
            USB_PORT_IDX=$OPTARG
            ;;
        z)
            NO_FUNCTION=1
            ;;
        ?)
            usage
            exit 1
            ;;
        esac
    done

    UDC=`ls /sys/class/udc/ | awk NR==$((USB_PORT_IDX+1))`
    if [ -z $UDC ]; then
        echo "usb device controller(UDC) driver is not exist!"
        exit
    fi
    ### specify dir depend on UDC
    USB_DEVICE_DIR=$USB_GADGET_DIR/$UDC
    USB_CONFIGS_DIR=$USB_DEVICE_DIR/configs/default.1
    USB_FUNCTIONS_DIR=$USB_DEVICE_DIR/functions

    if [ $NO_FUNCTION == 1 ]; then
        config_clear
        exit 0
    fi

    if [ $UVC_FUNCTION_ENABLE -eq 0 ] && [ $UAC_FUNCTION_ENABLE -eq 0 ] && [ $RNDIS_FUNCTION_ENABLE -eq 0 ]  && [ $RNDIS_FUNCTION_ENABLE -eq 0 ] \
            && [ $DFU_FUNCTION_ENABLE -eq 0 ] && [ $HID_FUNCTION_ENABLE -eq 0 ] && [ $SERIAL_FUNCTION_ENABLE -eq 0 ] \
            && [ $MASS_STORAGE_FUNCTION_ENABLE -eq 0 ] && [ $ADB_FUNCTION_ENABLE -eq 0 ] && [ $PRINTER_FUNCTION_ENABLE -eq 0 ] \
            && [ $ACCESSORY_FUNCTION_ENABLE -eq 0 ]  && [ $ACM_FUNCTION_ENABLE -eq 0 ]  && [ $ECM_FUNCTION_ENABLE -eq 0 ] \
            && [ $NCM_FUNCTION_ENABLE -eq 0 ] && [ $EEM_FUNCTION_ENABLE -eq 0 ] && [ $SOURCE_SINK_FUNCTION_ENABLE -eq 0 ]\
            && [ $LOOPBACK_FUNCTION_ENABLE -eq 0 ] && [ $MTP_FUNCTION_ENABLE -eq 0 ]; then
        usage
        exit 1
    fi

    # firstly clear config
    config_clear

    echo -e "\e[32mUsing UDC:$UDC\e[0m"

    if [ -z "$(mount | grep $KERNEL_CONFIG_DIR)" ]; then
        mount -t configfs none $KERNEL_CONFIG_DIR
    fi
    #创建device
    configfs_mkdir $USB_DEVICE_DIR
    configfs_mkdir ${USB_DEVICE_DIR}/strings/0x409
    #配置设备描述符的manufacturer, product, serialnumber
    MANUFACTURER=`echo $MANUFACTURER_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    PRODUCT=`echo $PRODUCT_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    SERIAL_NUM=`echo $SERIAL_NUM_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    echo $MANUFACTURER > ${USB_DEVICE_DIR}/strings/0x409/manufacturer
    echo $PRODUCT > ${USB_DEVICE_DIR}/strings/0x409/product
    echo $SERIAL_NUM > ${USB_DEVICE_DIR}/strings/0x409/serialnumber
    #bDeviceClass/bDeviceProtocol/bDeviceSubClass/bMaxPacketSize0/bcdDevice/bcdUSB/idProduct/idVendor
    DEVICE_ClASS=`echo $DEVICE_ClASS_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    DEVICE_PROTOCOL=`echo $DEVICE_PROTOCOL_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    DEVUCE_SUBCLASS=`echo $DEVUCE_SUBCLASS_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    MAXPKACKET_SIZE0=`echo $MAXPKACKET_SIZE0_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    BCD_DEVICE=`echo $BCD_DEVICE_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    BCD_USB=`echo $BCD_USB_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    echo $DEVICE_ClASS > ${USB_DEVICE_DIR}/bDeviceClass
    echo $DEVICE_PROTOCOL > ${USB_DEVICE_DIR}/bDeviceProtocol
    echo $DEVUCE_SUBCLASS > ${USB_DEVICE_DIR}/bDeviceSubClass
    echo $MAXPKACKET_SIZE0 > ${USB_DEVICE_DIR}/bMaxPacketSize0
    echo $BCD_DEVICE > ${USB_DEVICE_DIR}/bcdDevice
    echo $BCD_USB > ${USB_DEVICE_DIR}/bcdUSB
    # VID PID
    USB_DEVICE_PID=`echo $USB_DEVICE_PID_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    USB_DEVICE_VID=`echo $USB_DEVICE_VID_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    echo $USB_DEVICE_PID > ${USB_DEVICE_DIR}/idProduct
    echo $USB_DEVICE_VID > ${USB_DEVICE_DIR}/idVendor

    #创建一个config
    configfs_mkdir $USB_CONFIGS_DIR
    configfs_mkdir ${USB_CONFIGS_DIR}/strings/0x409
    # config的string
    CONFIGURATION=`echo $CONFIGURATION_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    echo $CONFIGURATION > ${USB_CONFIGS_DIR}/strings/0x409/configuration
    # config的MaxPower/bmAttributes
    MAXPOWER=`echo $MAXPOWER_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    echo $MAXPOWER > ${USB_CONFIGS_DIR}/MaxPower
    REMOTE_WAKEUP=`echo $REMOTE_WAKEUP_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    SELF_POWERED=`echo $SELF_POWERED_ARRAY | awk -F ',' '{print $'$((USB_PORT_IDX+1))'}'`
    ATTRIBUTES=$(((1<<7) + (REMOTE_WAKEUP<<5)+(SELF_POWERED<<6)))
    echo $ATTRIBUTES  > ${USB_CONFIGS_DIR}/bmAttributes
    # 配置functions
    # uvc
    FUNCTION_IDX=0;
    while [ $FUNCTION_IDX -lt $UVC_FUNCTION_ENABLE ]
    do
        config_uvc $FUNCTION_IDX
        ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${FUNCTION_IDX} ${USB_CONFIGS_DIR}/uvc.instance${FUNCTION_IDX}
        let FUNCTION_IDX++
    done
    # uac
    if [ $UAC_FUNCTION_ENABLE -gt 0 ]; then
        config_uac
        ln -s ${USB_FUNCTIONS_DIR}/uac1.instance0 ${USB_CONFIGS_DIR}/uac1.instance0
    fi
    # rndis
    if [ $RNDIS_FUNCTION_ENABLE -gt 0 ]; then
        echo 0x9999 > ${USB_DEVICE_DIR}/idVendor
        echo 0x9999 > ${USB_DEVICE_DIR}/idProduct
        config_rndis
        ln -s ${USB_FUNCTIONS_DIR}/rndis.instance0 ${USB_CONFIGS_DIR}/rndis.instance0
    fi
    # dfu
    if [ $DFU_FUNCTION_ENABLE -gt 0 ]; then
        config_dfu
        ln -s ${USB_FUNCTIONS_DIR}/dfu.instance0 ${USB_CONFIGS_DIR}/dfu.instance0
    fi
    # hid
    if [ $HID_FUNCTION_ENABLE -gt 0 ]; then
        config_hid
        ln -s ${USB_FUNCTIONS_DIR}/hid.instance0 ${USB_CONFIGS_DIR}/hid.instance0
    fi

    # acm
    FUNCTION_IDX=0;
    while [ $FUNCTION_IDX -lt $ACM_FUNCTION_ENABLE ]
    do
        config_acm $FUNCTION_IDX
        ln -s ${USB_FUNCTIONS_DIR}/acm.instance${FUNCTION_IDX} ${USB_CONFIGS_DIR}/acm.instance${FUNCTION_IDX}
        let FUNCTION_IDX++
    done

    # serial
    FUNCTION_IDX=0;
    while [ $FUNCTION_IDX -lt $SERIAL_FUNCTION_ENABLE ]
    do
        config_gserial $FUNCTION_IDX
        ln -s ${USB_FUNCTIONS_DIR}/gser.instance${FUNCTION_IDX} ${USB_CONFIGS_DIR}/gser.instance${FUNCTION_IDX}
        let FUNCTION_IDX++
    done

    # mass storage
    FUNCTION_IDX=0;
    while [ $FUNCTION_IDX -lt $MASS_STORAGE_FUNCTION_ENABLE ]
    do
        config_mass_storage $FUNCTION_IDX
        ln -s ${USB_FUNCTIONS_DIR}/mass_storage.instance${FUNCTION_IDX} ${USB_CONFIGS_DIR}/mass_storage.instance${FUNCTION_IDX}
        let FUNCTION_IDX++
    done

    # adb
    if [ $ADB_FUNCTION_ENABLE -gt 0 ]; then
        config_adb
        ln -s ${USB_FUNCTIONS_DIR}/ffs.adb ${USB_CONFIGS_DIR}/ffs.adb
        ##start adbd app
        if [ -e /dev/usb-ffs/adb ]; then
            :
        else
            configfs_mkdir /dev/usb-ffs/adb -p
            mount -o uid=2000,gid=2000 -t functionfs adb /dev/usb-ffs/adb
        fi
        start-stop-daemon --start --quiet --background --exec $ADBD_PATH
        sleep 1
    fi

    # mtp
    if [ $MTP_FUNCTION_ENABLE -gt 0 ]; then
        config_mtp
        ln -s ${USB_FUNCTIONS_DIR}/ffs.mtp ${USB_CONFIGS_DIR}/ffs.mtp
        ##start adbd app
        if [ -e /dev/usb-ffs/mtp ]; then
            :
        else
            configfs_mkdir /dev/usb-ffs/mtp -p
            mount -o uid=2000,gid=2000 -t functionfs mtp /dev/usb-ffs/mtp
        fi
        start-stop-daemon --start --quiet --background --exec $MTP_PATH
        sleep 1
    fi

    # printer
    FUNCTION_IDX=0;
    while [ $FUNCTION_IDX -lt $PRINTER_FUNCTION_ENABLE ]
    do
        config_printer $FUNCTION_IDX
        ln -s ${USB_FUNCTIONS_DIR}/printer.instance${FUNCTION_IDX} ${USB_CONFIGS_DIR}/printer.instance${FUNCTION_IDX}
        let FUNCTION_IDX++
    done
    # accessory
    if [ $ACCESSORY_FUNCTION_ENABLE -gt 0 ]; then
        config_accessory
        ln -s ${USB_FUNCTIONS_DIR}/accessory.instance0 ${USB_CONFIGS_DIR}/accessory.instance0
    fi
    # ecm
    if [ $ECM_FUNCTION_ENABLE -gt 0 ]; then
        config_ecm
        ln -s ${USB_FUNCTIONS_DIR}/ecm.instance0 ${USB_CONFIGS_DIR}/ecm.instance0
    fi
    # ncm
    if [ $NCM_FUNCTION_ENABLE -gt 0 ]; then
        config_ncm
        ln -s ${USB_FUNCTIONS_DIR}/ncm.instance0 ${USB_CONFIGS_DIR}/ncm.instance0
    fi
    # eem
    if [ $EEM_FUNCTION_ENABLE -gt 0 ]; then
        config_eem
        ln -s ${USB_FUNCTIONS_DIR}/eem.instance0 ${USB_CONFIGS_DIR}/eem.instance0
    fi
    # source sink
    if [ $SOURCE_SINK_FUNCTION_ENABLE -gt 0 ]; then
        # sink sink PID/VID setting
        echo 0x0525 > ${USB_DEVICE_DIR}/idVendor
        echo 0xa4a0 > ${USB_DEVICE_DIR}/idProduct
        config_source_sink
        ln -s ${USB_FUNCTIONS_DIR}/SourceSink.instance0 ${USB_CONFIGS_DIR}/SourceSink.instance0
    fi
    # loopback
    if [ $LOOPBACK_FUNCTION_ENABLE -gt 0 ]; then
        echo 0x0525 > ${USB_DEVICE_DIR}/idVendor
        echo 0xa4a0 > ${USB_DEVICE_DIR}/idProduct
        config_loopback
        ln -s ${USB_FUNCTIONS_DIR}/Loopback.instance0 ${USB_CONFIGS_DIR}/Loopback.instance0
    fi

    #bind udc
    echo $UDC > ${USB_DEVICE_DIR}/UDC
}


main $@
