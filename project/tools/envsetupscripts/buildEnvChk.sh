#!/bin/bash

#############################################################
# Script: buildEnvChk.sh
# Version: 1.0
# Description: This is a compilation environment checking script.
# Author: Sigmstar
# Date: 2023-06-10
# Last Modified: 2023-07-25
#############################################################

set +e

###################### global variable ######################
FILTER_VAL_DEFCONFIG="defconfig"
FILTER_VAL_SOURCE="."

SRC_PATH=""
SELECT_DEFCONFIG=""

#declare -a option_val
option_val_array=("Souffle" "Donut" "All" "Quit")
#############################################################

# print with color define
#############################################################
# Function: print_with_color
# Parameters:
#   $1: Font color (optional parameter, supports common names like "red", "yellow", "blue", etc.)
#   $2: Background color (optional parameter, supports common names like "black", "red", "green", etc.)
#   $3: Blinking (optional parameter, set to "blink" to enable blinking, omit for no blinking)
#   $4: Text content

function print_with_color() {
    local font_color_code=""
    local background_color_code=""
    local blink="0"
    local text=""

    # Parse parameters
    case $# in
        4)
            ## text must be the last parameters
            text="$4"
            if [[ $1 == "blink" || $2 == "blink" || $3 == "blink" ]]; then
                blink="blink"
                if [[ $1 == "blink" ]]; then
                    font_color_code="$2"
                    background_color_code="$3"
                elif [[ $2 == "blink" ]]; then
                    font_color_code="$1"
                    background_color_code="$3"
                elif [[ $3 == "blink" ]]; then
                    font_color_code="$1"
                    background_color_code="$2"
                fi
            else
                echo "Usage: print_with_color [font_color] [background_color] [blink] text" >&2; return 1
            fi
            ;;
        3)
            text="$3"
            if [[ $1 == "blink" || $2 == "blink" ]]; then
                blink="blink"
                if [[ $1 == "blink" ]]; then
                    font_color_code="$2"
                elif [[ $2 == "blink" ]]; then
                    font_color_code="$1"
                fi
            else
                font_color_code="$1"
                background_color_code="$2"
            fi
            ;;
        2)
            text="$2"
            if [[ $1 == "blink" ]]; then
                blink="blink"
            else
                font_color_code="$1"
            fi
            ;;
        1)
            text="$1"
            ;;
        *) echo "Usage: print_with_color [font_color] [background_color] [blink] text" >&2; return 1;;
    esac
    :<<!
    # Parse parameters
    case $# in
        4) font_color_code="$1"; background_color_code="$2"; blink="$3"; text="$4";;
        3) font_color_code="$1"; background_color_code="$2"; text="$3";;
        2) font_color_code="$1"; text="$2";;
        1) text="$1";;
        *) echo "Usage: print_with_color [font_color] [background_color] [blink] text" >&2; return 1;;
    esac
!
# ANSI escape code sequence
local escape="\033["

    # Font color setting
    case "$font_color_code" in
        "black") font_color_code="30";;
        "red") font_color_code="31";;
        "green") font_color_code="32";;
        "yellow") font_color_code="33";;
        "blue") font_color_code="34";;
        "magenta") font_color_code="35";;
        "cyan") font_color_code="36";;
        "white") font_color_code="37";;
        "default") font_color_code="39";;
    esac

    # Background color setting
    case "$background_color_code" in
        "black") background_color_code="40";;
        "red") background_color_code="41";;
        "green") background_color_code="42";;
        "yellow") background_color_code="43";;
        "blue") background_color_code="44";;
        "magenta") background_color_code="45";;
        "cyan") background_color_code="46";;
        "white") background_color_code="47";;
        "default") background_color_code="49";;
    esac

    # Blinking setting
    if [ "$blink" == "blink" ]; then
        blink="1"
    else
        blink="0"
    fi

    # Assemble ANSI escape code
    escape="${escape}${font_color_code:-39};"
    if [ -n "$background_color_code" ]; then
        escape="${escape}${background_color_code};"
    fi
    if [ "$blink" -eq 1 ]; then
        escape="${escape}5;"
    fi

    # Remove trailing semicolon
    escape="${escape%?}"

    # End escape code
    escape="${escape}m"

    # Print styled text
    echo -e "${escape}${text}\033[0m"
}

# Example usage:
:<<!
print_with_color red yellow blink "This is red font with yellow background and blinking"
print_with_color blue yellow "This is blue font with yellow background, no blinking"
print_with_color yellow "This is yellow font, no background and blinking"
print_with_color white black "This is white font with black background, no blinking"
print_with_color "This is default font and default background, no blinking"
!
#############################################################

envChk_SelectChipCode()
{
    print_with_color green yellow "runing at: ${FUNCNAME[0]}"; sleep 1
    echo "Please select the chip code:"
    i=0
    for str in "${option_val_array[@]}"
    do
        pathval[$i]=$str
        echo "$i) $str"
        let i+=1;
    done;

    read -ep "Your choice:" choice

    case "$choice" in
        0)
            echo "You've chosen ${option_val_array[$choice]}"
            FILTER_VAL_DEFCONFIG="i6f"
            FILTER_VAL_SOURCE=${option_val_array[$choice]}
            ;;
        1)
            echo "You've chosen ${option_val_array[$choice]}"
            FILTER_VAL_DEFCONFIG="i6d"
            FILTER_VAL_SOURCE=${option_val_array[$choice]}
            ;;
        2)
            echo "You've chosen ${option_val_array[$choice]}"
            #FILTER_VAL_DEFCONFIG="defconfig"
            #FILTER_VAL_SOURCE="."
            ;;
        3)
            echo "You've chosen ${option_val_array[$choice]}"
            ;;
        *)
            print_with_color red "Invalid option, please re-run the script and select the valid option."
            ;;
    esac
}

envChk_findAndSelectSrcPath()
{
    print_with_color green yellow "runing at: ${FUNCNAME[0]}"; sleep 1
    echo "doing find 'SourceCode path' ......"
    echo "result is:"
    res=$(find ~/ -name SourceCode -type d | grep $FILTER_VAL_SOURCE)

    # Check if the string variable is empty
    if [ -z "$res" ]; then
        print_with_color red "The '${FILTER_VAL_SOURCE}'s SourceCode' you're looking for is empty."
        print_with_color red ".";sleep 1
        print_with_color red "..";sleep 1
        print_with_color red "...";sleep 1
        res=$(find ~/ -name SourceCode -type d)
        if [ ! -z "$res" ]; then
            print_with_color green yellow "This is all the SourceCode for your current ~/ directory, but no ${FILTER_VAL_SOURCE}'s code"
            echo -e "--------------------"
            echo $res
            echo -e "--------------------\n"
        else
            print_with_color red yellow "Oops...! you don't have SourceCode in your ~/ directory."
        fi
        print_with_color red "Please check that your source code directory is correct, I'm leaving now, bye!"
        exit 1
    else
        echo -e "--------------------"
        echo $res
        echo -e "--------------------\n"
    fi

    # If the string variable is not empty, continue with the script
    print_with_color yellow "SourceCode is not empty. Continuing with the script."

    declare -a pathval
    i=0
    for r in $res
    do
        pathval[$i]=$r
        let i+=1;
    done;


    #add Quit for option
    array_len=${#pathval[@]}
    pathval[$array_len]="Quit"
    array_len=${#pathval[@]}

    PS3='Choose which 'SourceCode' to check compile env: '
    select select in "${pathval[@]}"; do
        SelectDone=0
        if [[ -z $select ]];
        then
            print_with_color red "selcet item is empty. recheck!"
            continue
        fi

        for ((i=0;i<=array_len;i++))
        do
            if [ $select == ${pathval[$i]} ]; then
                SRC_PATH=$select
                echo Select 'SourceCode' path: $SRC_PATH
                SelectDone=1
                break;
            fi
        done

        if [ $SelectDone -eq 1 ]; then
            break;
        fi
    done

    if [ $SRC_PATH == "Quit" ]; then
        echo "em...byebye!"
        exit 1;
    fi

    ################### global variable #####################
    SRC_PATH_PROJECT=`find ${SRC_PATH} -name project*.tar.gz`
    SRC_PATH_SDK=`find ${SRC_PATH} -name sdk*.tar.gz`
    SRC_PATH_BOOT=`find ${SRC_PATH} -name boot*.tar.gz`
    SRC_PATH_KERNEL=`find ${SRC_PATH} -name kernel*.tar.gz`
    #########################################################

    #echo $SRC_PATH_PROJECT
    if [ ! -n "$SRC_PATH_PROJECT" ]; then
        print_with_color red "can't find project source code, plz confirm...!"
        exit 1;
    else
        print_with_color green "project src path check ok!"
    fi

    #echo $SRC_PATH_SDK
    if [ ! -n "$SRC_PATH_SDK" ]; then
        print_with_color red "can't find sdk source code, plz confirm...!"
        exit 1;
    else
        print_with_color green "sdk src path check ok!"
    fi

    #echo $SRC_PATH_BOOT
    if [ ! -n "$SRC_PATH_BOOT" ]; then
        print_with_color red "can't find boot source code, plz confirm...!"
        exit 1;
    else
        print_with_color green "boot src path check ok!"
    fi

    #echo $SRC_PATH_KERNEL
    if [ ! -n "$SRC_PATH_KERNEL" ]; then
        print_with_color red "can't find kernel source code, plz confirm...!"
        exit 1;
    else
        print_with_color green "kernel src path check ok!"
    fi

    cd $SRC_PATH
    pwdval=`pwd`
    if [ ! -d "$pwdval/boot" ];then
        echo "The boot directory is being decompressed, please wait..."
        tar xzf boot-*.tar.gz
    else
        echo "The boot path already exists"
    fi
    if [ ! -d "$pwdval/kernel" ];then
        echo "The kernel directory is being decompressed, please wait..."
        tar xzf kernel-*.tar.gz
    else
        echo "The kernel path already exists"
    fi
    if [ ! -d "$pwdval/sdk" ];then
        echo "The sdk directory is being decompressed, please wait..."
        tar xzf sdk-*.tar.gz
    else
        echo "The sdk path already exists"
    fi
    if [ ! -d "$pwdval/project" ];then
        echo "The project directory is being decompressed, please wait..."
        tar xzf project-*.tar.gz
    else
        echo "The project path already exists"
    fi

    #    rm -rf *.tar.gz
}

envChk_listAndExportToolchain()
{
    print_with_color green yellow "runing at: ${FUNCNAME[0]}"; sleep 1
    echo "doing find '/tools/toolchain/' list ..."
    echo "result is:"
    res=$(ls /tools/toolchain/)
    echo -e "------------------------------------------------------------"
    echo $res
    echo -e "------------------------------------------------------------\n"

    declare -a toolchain_val
    i=0
    for r in $res
    do
        export PATH=/tools/toolchain/$r/bin:$PATH
        echo "do <export PATH=/tools/toolchain/$r/bin:\$PATH>"
        let i+=1;
    done;

    export CROSS_COMPILE=arm-linux-gnueabihf-
    export ARCH=arm

    source /etc/profile

    echo "check done!!"
    #exit 1;
}

envChk_selectDefconfig()
{
    print_with_color green yellow "runing at: ${FUNCNAME[0]}"; sleep 1
    if [[ -z "$SRC_PATH" ]]; then
        print_with_color red "source code path in NULL! please run function: envChk_findAndSelectSrcPath"
        exit 1;
    else
        cd $SRC_PATH/project/configs/defconfigs
    fi

    count=0
    declare -a array_config
    for i in `ls | grep $FILTER_VAL_DEFCONFIG`;do
        array_config[${count}]=${i}
        echo "${count}) ${array_config[${count}]}"
        count=$(expr ${count} + 1)
    done
    #add a quit option
    array_config[${count}]="quit!"
    echo "${count}) ${array_config[${count}]}"

    read  -ep "please select build deconfig:" input_config
    print_with_color yellow "select is $input_config) ${array_config[$input_config]}"

    if [ "${array_config[$input_config]}" = "quit!" ]; then
        print_with_color red "quit now!"
        exit 1;
    fi
    SELECT_DEFCONFIG=${array_config[$input_config]}
}

envChk_compileChk()
{
    print_with_color green yellow "runing at: ${FUNCNAME[0]}"; sleep 1
    if [[ -z "$SRC_PATH" ]]; then
        print_with_color red "source code path in NULL! please run function: envChk_findAndSelectSrcPath"
        exit 1;
    else
        cd $SRC_PATH/project/
    fi
    if [[ -z $SELECT_DEFCONFIG ]]; then
        print_with_color red "SELECT_DEFCONFIG is NULL"
        exit 1;
    fi

    make $SELECT_DEFCONFIG
    time make clean -j4
    time make image -j2
}

envChk_SelectChipCode
envChk_findAndSelectSrcPath
envChk_listAndExportToolchain
envChk_selectDefconfig
envChk_compileChk
ret_val=$?
if [ $ret_val -eq 0 ]; then
    print_with_color yellow green "The compilation was successful and the return value is: $ret_val"
elif [ $ret_val -eq 1 ]; then
    print_with_color red yellow blink "The compilation execution fails with the return value: $ret_val"
else
    print_with_color red yellow blink "An error occurred in the compilation execution and the return value is: $ret_val"
fi

print_with_color green black blink "done!"
