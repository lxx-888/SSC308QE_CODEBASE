#!/bin/bash
#----------------------------------------
# PATH Define
#----------------------------------------

TARGET_DIR=./image/output/images
if [ ! -e "$TARGET_DIR" ]; then
TARGET_DIR=$(dirname "$(readlink -f "$0")")/../../image_secure
fi

AUTO_UPDATE_SCRIPT=$TARGET_DIR/auto_update.txt
TMP_UPGRADE_FILE=TMP_SigmastarUpgrade.bin
PADDED_BIN=$TARGET_DIR/padded.bin
PAD_DUMMY_BIN=$TARGET_DIR/dummy_pad
UPGRADE_FILE=$TARGET_DIR/SigmastarUpgrade.bin
SCRIPT_FILE=$TARGET_DIR/upgrade_script.txt

#----------------------------------------
# Globe Value Define
#----------------------------------------
SCRIPT_SIZE=0x20000 #128KB
PAD_DUMMY_SIZE=10240 #10KB
currentImageOffset=$SCRIPT_SIZE
MAGIC_STRING=12345678
FullUpgrade="y"
Release_L0="n"
DEV_PART="0"
DISP_UI="n"
TOTAL_BIN_SIZE=0;
TMP_TOTAL_BIN_SIZE=0

function func_cal_total_bin_size()
{
    filePath=$1

    filecontent=$(grep -Ev "^\s*$|#" $TARGET_DIR/$filePath)
    for subContent in $filecontent
    do
        if [ "$subContent" != "% <- this is end of file symbol" ] && [ "$subContent" != "sync_mmap" ]; then
            subContent_prefix=$(echo $subContent | awk '{print $1}')
            if [ "$subContent_prefix" == "tftp" ]; then
                imagePath=$(echo $subContent | awk '{print $3}')
                imageSize=$(stat -c%s $TARGET_DIR/$imagePath)
                TOTAL_BIN_SIZE=$(($TOTAL_BIN_SIZE+$imageSize))
                #echo "imagePath:$imagePath ,imageSize:$imageSize, TOTAL_BIN_SIZE:$TOTAL_BIN_SIZE"
            fi
        fi
    done

}

function func_process_main_script()
{
    mainScript=""
    tmp2="
"

     if [ ! -f "$AUTO_UPDATE_SCRIPT" ] ; then
         echo "auto_update script is not exist, can not generate bin, please build image!!!"
         exit
    fi

    tmpScript=$(cat $AUTO_UPDATE_SCRIPT)
    for mainContent in $tmpScript
    do
        if [ "$(echo $mainContent | awk '{print $1}')" == "estar" ]; then
            imageName=$(echo $mainContent | awk '{print $2}' | cut -d '/' -f 2)
            #confirm each image is upgrade or not.
            if [ "$FullUpgrade" == "Y" ] || [ "$FullUpgrade" == "y" ]; then
                temp=y
            else
                read -p "Upgrade $imageName? (y/N)" temp
            fi
            if [ "$temp" == "Y" ] || [ "$temp" == "y" ] || [ -z "$temp" ] ; then
                mainScript=$mainScript$mainContent$tmp2
            fi
        else
            mainScript=$mainScript$mainContent$tmp2
        fi
    done

    if [ "$DISP_UI" == "y" ]; then
        for mainContent in $mainScript
        do
            if [ "$(echo $mainContent | awk '{print $1}')" == "estar" ]; then
                func_cal_total_bin_size $(echo $mainContent|awk '{print $2}')
            fi
        done
    fi

    echo "UsbUpgradeImage Generating..."
    for mainContent in $mainScript
    do
        if [ "$(echo $mainContent | awk '{print $1}')" == "estar" ]; then
            func_process_sub_script $(echo $mainContent|awk '{print $2}')
        else
            if [ "$mainContent" != "% <- this is end of file symbol" ]; then
                echo $mainContent >> $SCRIPT_FILE
            fi
        fi
    done
}

function func_process_sub_script()
{
    update_percent_flag=0
    filePath=$1
    fileName=$(echo $1 | cut -d '/' -f 2 | cut -d '[' -f 3)
    echo "" >> $SCRIPT_FILE
    echo "# File Partition: "$fileName >> $SCRIPT_FILE
    filecontent=$(grep -Ev "^\s*$|#" $TARGET_DIR/$filePath)
    for subContent in $filecontent
    do
        if [ "$subContent" != "% <- this is end of file symbol" ] && [ "$subContent" != "sync_mmap" ]; then
            subContent_prefix=$(echo $subContent | awk '{print $1}')
            if [ "$subContent_prefix" == "tftp" ]; then
                DRAM_BUF_ADDR=$(echo $subContent | awk '{print $2}')
                imagePath=$(echo $subContent | awk '{print $3}')
                imageSize=$(stat -c%s $TARGET_DIR/$imagePath)
                printf "fatload usb %s %s \$(UpgradeImage) 0x%x 0x%x\n" $DEV_PART $DRAM_BUF_ADDR $imageSize $currentImageOffset >> $SCRIPT_FILE
                cat $TARGET_DIR/$imagePath >> $TMP_UPGRADE_FILE
                TMP_TOTAL_BIN_SIZE=$(($TMP_TOTAL_BIN_SIZE+$imageSize))
                if [ "$DISP_UI" == "y" ]; then
                    PERCENT=$(( $TMP_TOTAL_BIN_SIZE * 100 / $TOTAL_BIN_SIZE ))
                    if [ $PERCENT -gt 100 ]; then
                        PERCENT=100
                    fi
                    update_percent_flag=1
                fi
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
    if [ "$DISP_UI" == "y" ]; then
        if [ $update_percent_flag == 1 ]; then
            printf "disp_ui_update %s\n" $PERCENT >> $SCRIPT_FILE
            if [ $PERCENT == 100 ]; then
                echo "disp_ui_update success" >> $SCRIPT_FILE
            fi
        fi
    fi
}

function func_init()
{
    # delete related file
    rm -f $SCRIPT_FILE
    rm -f $UPGRADE_FILE
    rm -f $TMP_UPGRADE_FILE
    dos2unix $AUTO_UPDATE_SCRIPT $TARGET_DIR/scripts/* 1>/dev/null 2>&1
}

function func_generate_script_file()
{

    echo "% <- this is end of script symbol" >>$SCRIPT_FILE

    # pad script to script_file size
    SCRIPT_FILE_SIZE=$(stat -c%s $SCRIPT_FILE)
    PADDED_SIZE=$(($SCRIPT_SIZE-$SCRIPT_FILE_SIZE))

    printf "\xff" >$PAD_DUMMY_BIN
    for ((i=1; i<$PAD_DUMMY_SIZE; i++))
    do
        printf "\xff" >>$PAD_DUMMY_BIN
    done

    while [ $PADDED_SIZE -gt $PAD_DUMMY_SIZE ]
    do
        cat $PAD_DUMMY_BIN >> $SCRIPT_FILE
        PADDED_SIZE=$(($PADDED_SIZE-$PAD_DUMMY_SIZE))
    done
    rm -f $PAD_DUMMY_BIN
    if [ $PADDED_SIZE != 0 ]; then
        printf "\xff" >$PADDED_BIN
        for ((i=1; i<$PADDED_SIZE; i++))
        do
            printf "\xff" >>$PADDED_BIN
        done
        cat $PADDED_BIN >> $SCRIPT_FILE
        rm $PADDED_BIN
    fi
}

function func_generate_upgrade_file()
{
    # generate $UPGRADE_FILE
    cat $SCRIPT_FILE > $UPGRADE_FILE
    cat $TMP_UPGRADE_FILE >> $UPGRADE_FILE
    rm $TMP_UPGRADE_FILE
    rm $SCRIPT_FILE
    # pad mangic to $UPGRADE_FILE
    printf "%s" $MAGIC_STRING >> $UPGRADE_FILE

    # copy the first 16 bytes to last
    dd if=$UPGRADE_FILE of=./first16.bin bs=16 count=1;
    cat ./first16.bin >> $UPGRADE_FILE
    rm -f ./first16.bin
    echo "success, usb upgrade image have generated:"
    echo -e "      path:\033[40;31m$UPGRADE_FILE\033[0m"
    echo "      size:$(stat -c%s $UPGRADE_FILE) byte"
    if [ "$Release_L0" == "y" ]; then
        ssbuild_path=$(which ssbuild.sh)
        if [ -n $ssbuild_path ]; then
           sh $ssbuild_path $UPGRADE_FILE
        fi
    fi
}

function parsing_para()
{
    while [ $# != 0 ]; do
        tmp1=$1
        tmp2=`echo $1|cut -d= -f1`
        case "$tmp2" in
        "FullUpgrade" )
            FullUpgrade=`echo $tmp1|cut -d= -f2`
            ;;
        * )
            echo "unknown parameter !!"
            exit 1
            ;;
        esac
        shift
    done

    if [ "$FullUpgrade" == "Y" ];then
        tmp=$FullUpgrade
    else
        read -p "Full Upgrade? (Y/n)" tmp
    fi
}

#-----------------------------------------------
# Main():Generate Upgrade Image via Tftp Script
#-----------------------------------------------
IFS="
"
while getopts ":FPrdi:f:" opt
do
    case $opt in
    F)
        FullUpgrade=y
        ;;
    P)
        FullUpgrade=n
        ;;
    r)
        Release_L0=y
        ;;
    d)
        DISP_UI=y
        ;;
    i)
        DEV_PART="$OPTARG"
        ;;
    f)
        UPGRADE_FILE="$TARGET_DIR/$OPTARG"
        ;;
    ?)
        echo
        echo "Usage: $0 -F --> ALL"
        echo "       $0 -P --> PACK BY PART"
        echo "       $0 -r --> releae to L0 by ssbuild"
        echo "       $0 -d --> disp ui update"
        echo "       $0 -i --> specify dev:part"
        echo
        exit 1
        ;;
    esac
done
if [ "$FullUpgrade" == "Y" ] || [ "$FullUpgrade" == "y" ]; then
    echo "PACK ALL"
else
    echo "PACK BY PART"
fi
func_init;
func_process_main_script;
func_generate_script_file;
func_generate_upgrade_file;
