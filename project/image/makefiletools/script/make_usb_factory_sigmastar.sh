#!/bin/bash

PROJECT_DIR="."
BUILD_TIME=`date '+%Y%m%d%H%M'`
UPGRADE_FILE_NAME=SstarUsbImage_${BUILD_TIME}.bin
TMP_UPGRADE_FILE=TMP_SigmastarUpgrade.bin
SCRIPT_SIZE=0x5000 #20KB
currentImageOffset=0
MAGIC_STRING="SSTAR_USB_IMAGE"
USAGE=0
############ default, you can change it by cmd ###
secureboot=0
RELEASE_L0=0
FullUpgrade=0

#############below as set in main function, change it is invalid here ####
TARGET_DIR=""
AUTO_UPDATE_SCRIPT=""
PADDED_BIN=""
PAD_DUMMY_BIN=""
UPGRADE_FILE=""
SCRIPT_FILE=""
USB_UPDATER_LOAD_FILE=""
U_BOOT_LOAD_FILE=""
USB_BOOT_BIN_DIR=""
IPL_PATH=""
UBOOT_PATH=""
TFA_PATH=""
IPL=""
UBOOT=""



function func_process_main_script()
{
    BUILD_PATH=`pwd`
    setpartition=0

    IPL_PATH=$USB_BOOT_BIN_DIR/ipl
    UBOOT_PATH=$USB_BOOT_BIN_DIR/uboot
    TFA_PATH=$USB_BOOT_BIN_DIR/tf_a

    #confirm each image is upgrade or not.
    mainScript=""
    tmp2="
"

    if [ ! -f "$AUTO_UPDATE_SCRIPT" ] ; then
        echo "auto_update script is not exist, can not generate bin, please build image!!!"
        exit
    fi

    if [ "$IPL" == "" ] || [ "$UBOOT" == "" ]; then
        echo
        echo "                 cmd param error: you must specify usb IPL and UBOOT!!!"
        usage;
        exit
    fi

    if [ ! -f "$IPL_PATH/$IPL" ]; then
        echo "$IPL_PATH/$IPL is not exist"
        exit
    fi

    if [ ! -f "$UBOOT_PATH/$UBOOT" ]; then
        echo "$UBOOT_PATH/$UBOOT is not exist"
        exit
    fi

    if [ "$FullUpgrade" != "1" ]; then
        parsing_upgrade_para;
    fi

    # generate usb_updater
    cat $IPL_PATH/$IPL > $USB_UPDATER_LOAD_FILE
    # generate boot
    if [ -f $TFA_PATH/u-bl31.bin ]; then
        if [ $secureboot == 1 ];then
            cat $TFA_PATH/u-bl31.bin_final > $U_BOOT_LOAD_FILE
        else
            cat $TFA_PATH/u-bl31.bin > $U_BOOT_LOAD_FILE
        fi
        cat $UBOOT_PATH/$UBOOT >> $U_BOOT_LOAD_FILE
    else
        cat $UBOOT_PATH/$UBOOT > $U_BOOT_LOAD_FILE
    fi

    tmpScript=$(cat $AUTO_UPDATE_SCRIPT)
    for mainContent in $tmpScript
    do
        if [ "$(echo $mainContent | awk '{print $1}')" == "estar" ];then
            imageName=$(echo $mainContent | awk '{print $2}' | cut -d '/' -f 2)
            if [ "$FullUpgrade" == "Y" ] || [ "$FullUpgrade" == "y" ] || [ -z "$FullUpgrade" ] || [ "$FullUpgrade" == "1" ]; then
                mainScript=$mainScript$mainContent$tmp2
            else
                read -p "Optional Upgrade $imageName? (Y/N)" temp
                if [ "$temp" == "Y" ] || [ "$temp" == "y" ] || [ -z "$temp" ]; then
                    mainScript=$mainScript$mainContent$tmp2
                fi
            fi
        else
            mainScript=$mainScript$mainContent$tmp2
        fi
    done

    echo "USB Facotry Image Generating....."
    # pad mangic to $UPGRADE_FILE
    printf "#%s\n" $MAGIC_STRING > $SCRIPT_FILE
    printf "#GENERATED TIME:%s\n" $BUILD_TIME  >> $SCRIPT_FILE
    printf "SCRIPT 0x0 0x%x\n" $SCRIPT_SIZE  >> $SCRIPT_FILE
    currentImageOffset=$(($currentImageOffset+$SCRIPT_SIZE))
    func_process_load_file_to_script
    for mainContent in $mainScript
    do
        if [ "$(echo $mainContent | awk '{print $1}')" == "estar" ];then
            func_process_sub_script $(echo $mainContent|awk '{print $2}')
        else
            if [ "$mainContent" != "% <- this is end of file symbol" ];then
                echo "# MAIN SCRIPT: " >> $SCRIPT_FILE
                echo $mainContent >> $SCRIPT_FILE
            fi
        fi
        echo  >> $SCRIPT_FILE
    done
}

function func_process_load_file_to_script()
{
    if [ ! -f "$USB_UPDATER_LOAD_FILE" ] ||  [  ! -f "$U_BOOT_LOAD_FILE" ]; then
        echo "usb updater or u-boot is not exist!!!"
        exit
    fi
    # usb_updater
    usb_updater_size=$(stat -c%s $USB_UPDATER_LOAD_FILE)
    # align size to 512-byte
    if [ $(($usb_updater_size & 0xff)) != 0 ]; then
        afterAlignSize=$(($usb_updater_size+0x100-$(($usb_updater_size & 0xff))))
        printf "USB_UPDATER 0x%x 0x%x\n" $currentImageOffset $afterAlignSize >> $SCRIPT_FILE
    else
        printf "USB_UPDATER 0x%x 0x%x\n" $currentImageOffset $usb_updater_size >> $SCRIPT_FILE
    fi
    cat $USB_UPDATER_LOAD_FILE >> $TMP_UPGRADE_FILE
    # align image to 0x1000(4K)
    needAlignSize=0
    not_align_size=$(($usb_updater_size & 0xfff))
    if [ $not_align_size != 0 ]; then
        needAlignSize=$((0x1000-$not_align_size))
        for ((i=0; i<$needAlignSize; i++))
        do
            printf "\xff" >>$PADDED_BIN
        done

        cat $PADDED_BIN >>$TMP_UPGRADE_FILE
        rm $PADDED_BIN
    fi
    currentImageOffset=$(($currentImageOffset+$usb_updater_size+$needAlignSize))

    #u-boot
    needAlignSize=0
    u_boot_size=$(stat -c%s $U_BOOT_LOAD_FILE)
    printf "U-BOOT 0x%x 0x%x\n" $currentImageOffset $u_boot_size >> $SCRIPT_FILE
    cat $U_BOOT_LOAD_FILE >> $TMP_UPGRADE_FILE
    # align image to 0x1000(4K)
    not_align_size=$(($u_boot_size & 0xfff))
    if [ $not_align_size != 0 ]; then
        needAlignSize=$((0x1000-$not_align_size))
        for ((i=0; i<$needAlignSize; i++))
        do
            printf "\xff" >>$PADDED_BIN
        done

        cat $PADDED_BIN >>$TMP_UPGRADE_FILE
        rm $PADDED_BIN
    fi
    currentImageOffset=$(($currentImageOffset+$u_boot_size+$needAlignSize))
}

function func_process_sub_script()
{
    filePath=$1
    fileName=$(echo $1 | cut -d '/' -f 2 | cut -d '[' -f 3)
    echo "# File Partition: "$fileName >> $SCRIPT_FILE
    filecontent=$(grep -Ev "^\s*$|#" $TARGET_DIR/$filePath)
    for subContent in $filecontent
    do
        if [ "$subContent" != "% <- this is end of file symbol" ] && [ "$subContent" != "sync_mmap" ]; then
            subContent_prefix=$(echo $subContent | awk '{print $1}')
            if [ "$subContent_prefix" == "estar" ]; then
                func_process_sub_script $(echo $subContent|awk '{print $2}')
            elif [ "$subContent_prefix" == "tftp" ]; then
                DRAM_BUF_ADDR=$(echo $subContent | awk '{print $2}')
                imagePath=$(echo $subContent | awk '{print $3}')
                if [ ! -f "$TARGET_DIR/$imagePath" ] ; then
                    echo "error:image:$TARGET_DIR/$imagePath is not exist!!!"
                    rm -f $SCRIPT_FILE
                    exit
                fi
                imageSize=$(stat -c%s $TARGET_DIR/$imagePath)
                printf "usbload %s 0x%x 0x%x\n" $DRAM_BUF_ADDR $currentImageOffset $imageSize >> $SCRIPT_FILE
                cat $TARGET_DIR/$imagePath >> $TMP_UPGRADE_FILE

                # align image to 0x1000(4K)
                ImageAlignSize=0
                NOT_ALAIN_IMAGE_SIZE=$(($imageSize & 0xfff))
                if [ $NOT_ALAIN_IMAGE_SIZE != 0 ]; then
                    ImageAlignSize=$((0x1000-$NOT_ALAIN_IMAGE_SIZE))
                    for ((i=0; i<$ImageAlignSize; i++))
                    do
                        printf "\xff" >>$PADDED_BIN
                    done

                    cat $PADDED_BIN >>$TMP_UPGRADE_FILE
                    rm $PADDED_BIN
                fi
                currentImageOffset=$(($currentImageOffset+$imageSize+$ImageAlignSize))
            else
                subContent=$(echo $subContent | tr -d '\r\n')
                echo $subContent >> $SCRIPT_FILE
            fi
        fi
    done
}

function func_insert_cmd_to_partition
{
    output_file=$set_partition_file
    insert_line="flash_autok nand 0 0"
    keyword="nand probe"

    # check the keyword
    if grep -q "$keyword" "$set_partition_file"; then
        cp $set_partition_file ${set_partition_file}.bak
        # inster keyword to next line
        sed "/$keyword/a$insert_line" $set_partition_file > $set_partition_file.insert
        mv $set_partition_file.insert $set_partition_file
        echo "insert $insert_line cmd to $set_partition_file"
    fi
}

function func_restore_partition
{
    # restore [[set_partition.es
    if [ -e "${set_partition_file}.bak" ]; then
        cp ${set_partition_file}.bak $set_partition_file
    else
        echo "${set_partition_file}.bak does not exist."
    fi
}

function func_init()
{
    # delete related file
    rm -f $SCRIPT_FILE
    rm -f $TMP_UPGRADE_FILE
    dos2unix $AUTO_UPDATE_SCRIPT $TARGET_DIR/scripts/* 1>/dev/null 2>&1
}

function func_generate_script_file()
{
    echo "% <- this is end of script symbol" >>$SCRIPT_FILE
    # pad script to script_file size
    REL_SCRIPT_FILE_SIZE=$(stat -c%s $SCRIPT_FILE)
    PADDED_SIZE=$(($SCRIPT_SIZE-$REL_SCRIPT_FILE_SIZE))
    if [ $PADDED_SIZE -lt 0 ]; then
        echo "error: actural script file size($REL_SCRIPT_FILE_SIZE) > setting file size($SCRIPT_SIZE), please change SCRIPT_SIZE, then try again"
        exit
    fi

    rm -f $PAD_DUMMY_BIN
    n=$PADDED_SIZE
    while [ $n -ne 0 ]; do
        echo -e -n '\xff' >> $PAD_DUMMY_BIN
        ((n=n-1))
    done
    if [ $(stat -c%s $PAD_DUMMY_BIN) != $PADDED_SIZE ]; then
        echo "padded size:$PADDED_SIZE error!!!"
        exit
    fi
    cat $PAD_DUMMY_BIN >> $SCRIPT_FILE
}

function func_generate_upgrade_file()
{
    # generate $UPGRADE_FILE
    cat $SCRIPT_FILE > $UPGRADE_FILE
    rm -f $SCRIPT_FILE
    cat $TMP_UPGRADE_FILE >> $UPGRADE_FILE
    rm $TMP_UPGRADE_FILE
    rm $PAD_DUMMY_BIN
    rm -f $U_BOOT_LOAD_FILE
    rm -f $USB_UPDATER_LOAD_FILE
    echo "success, usb factory image have generated:"
    echo -e "      path:\033[40;31m$UPGRADE_FILE\033[0m"
    echo "      size:$(stat -c%s $UPGRADE_FILE) byte"
    md5="$(md5sum $UPGRADE_FILE  | cut -d" " -f1)"
    echo "      md5sum:$md5"
    printf "$md5" >> $UPGRADE_FILE
    if [ $RELEASE_L0 == 1 ]; then
        ssbuild_path=$(which ssbuild.sh)
        if [ -n $ssbuild_path ]; then
           sh $ssbuild_path $UPGRADE_FILE
        fi
    fi
}

function parsing_upgrade_para()
{
    read -p "Full or Optional Upgrade ? (Y/N)" FullUpgrade
}

#-----------------------------------------------
# Main():Generate Upgrade Image  Script
#-----------------------------------------------
IFS="
"

function usage()
{
    echo
    echo "Usage:"
    if [ $secureboot == 0 ];then
        echo "         ./image/makefiletools/script/make_usb_factory_sigmastar.sh -i IPL -u UBOOT  [-r] [-f] [-s] "
    else
        echo "         ./tools/makefiletools/make_usb_factory_sigmastar.sh -i IPL -u UBOOT"
    fi
    echo
    echo "        -i IPL: specify USB IPL"
    echo "        -u UBOOT: specify USB UBOOT"
    echo "        -r: release to L0 from L3 by ssbuild"
    echo "        -f: full image upgrade"
    echo "        -s: secure boot"
    echo

    echo -e "\033[40;31mSupport IPL:\033[0m"
    ls -1 $IPL_PATH | sed 's/^/\t/'
    echo
    echo -e "\033[40;31mSupport UBOOT:\033[0m"
    ls -1  $UBOOT_PATH | sed 's/^/\t/'
    echo
    echo "    notice: If you can't find the boot file, compile it yourself and put it in the specified directory($USB_BOOT_BIN_DIR)"
    echo
}

function main()
{
    if [ -f $PROJECT_DIR/configs/current.configs ]; then
        # first find IPL/UBOOT from config
        IPL=$(cat $PROJECT_DIR/configs/current.configs | grep USB_UPGRADE_IPL_CONFIG)
        IPL=${IPL#*=}
        IPL=$(echo $IPL | awk '{gsub(/^\s+|\s+$/, "");print}')

        UBOOT=$(cat $PROJECT_DIR/configs/current.configs | grep USB_UPGRADE_UBOOT_CONFIG)
        UBOOT=${UBOOT#*=}
        UBOOT=$(echo $UBOOT | awk '{gsub(/^\s+|\s+$/, "");print}')

        chip=$(cat $PROJECT_DIR/configs/current.configs | grep "CHIP =")
        chip=${chip#*=}
        chip=$(echo $chip | awk '{gsub(/^\s+|\s+$/, "");print}')

        TARGET_DIR=$PROJECT_DIR/image/output/images
        USB_BOOT_BIN_DIR=$PROJECT_DIR/board/$chip/boot/usb/upgrade
        set_partition_file=$TARGET_DIR/scripts/[[set_partition.es
    fi

    while [ $# -ge 1 ]; do
        case "$1" in
            -f) FullUpgrade=1; shift 1;;
            -r) RELEASE_L0=1; shift 1;;
            -i) IPL="$2"; shift 2;;
            -u) UBOOT="$2"; shift 2;;
            -s) secureboot=1; shift 1;;
            -n) UPGRADE_FILE_NAME="$2"; shift 2;;
            -h) USAGE=1; shift 1;;
            *) echo "unknown parameter $1"; exit 1; break;;
        esac
    done

    if [ "$secureboot" == "1" ]; then
        TARGET_DIR=$(dirname "$(readlink -f "$0")")/../../image_secure
        USB_BOOT_BIN_DIR=$(dirname "$(readlink -f "$0")")/../../sboot_sign/usb
    fi

    AUTO_UPDATE_SCRIPT=$TARGET_DIR/auto_update.txt
    PADDED_BIN=$TARGET_DIR/padded.bin
    PAD_DUMMY_BIN=$TARGET_DIR/dummy_pad
    UPGRADE_FILE=$TARGET_DIR/$UPGRADE_FILE_NAME
    SCRIPT_FILE=$TARGET_DIR/sstar_usb_factory_script.txt
    USB_UPDATER_LOAD_FILE=$TARGET_DIR/usb_updater_ipl.bin
    U_BOOT_LOAD_FILE=$TARGET_DIR/usb_updater_boot.bin
    IPL_PATH=$USB_BOOT_BIN_DIR/ipl
    UBOOT_PATH=$USB_BOOT_BIN_DIR/uboot
    TFA_PATH=$USB_BOOT_BIN_DIR/tf_a

    if [ $USAGE == 1 ]; then
        usage
        exit 1;
    fi

    func_insert_cmd_to_partition;
    func_init;
    func_process_main_script;
    func_generate_script_file;
    func_generate_upgrade_file;
    func_restore_partition;
}

main $@
