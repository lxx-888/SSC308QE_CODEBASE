#!/bin/bash

#############################################################
# Script: buildEnvInstall.sh
# Version: 1.0
# Description: This is a compilation environment installation script.
# Author: Sigmstar
# Date: 2023-06-10
# Last Modified: 2023-07-25
#############################################################

set -e

# global variable
#############################################################
USERNAME=$1

TIMEOUT_INPUT=10
DEFAULT_TOOLCHAIN_CHOICE=0
TOOLCHAIN_NAME="gcc-11.1.0-20210608-sigmastar-glibc-x86_64_arm-linux-gnueabihf"

EXPORT_FILE="/etc/profile"
EXPORT_STR_TOOLCHAIN_BIN_PATH="export PATH=\$PATH:/tools/toolchain/${TOOLCHAIN_NAME}/bin"
EXPORT_STR_CROSS_COMPILE="export CROSS_COMPILE=arm-linux-gnueabihf-"
EXPORT_STR_ARCH="export ARCH=arm"
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
    local font_color_code="default"
    local background_color_code="default"
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

_update_global_variable()
{
    EXPORT_STR_TOOLCHAIN_BIN_PATH="export PATH=\$PATH:/tools/toolchain/${TOOLCHAIN_NAME}/bin"
}

# Function to perform countdown and print remaining seconds
countdown_print() {
    local duration=$1
    local interval=1  # Print interval (seconds)

    for ((i = duration; i >= 0; i--)); do
        echo -ne "Countdown: $i seconds \r"
        sleep $interval
    done

    echo "Countdown: 0 seconds"
}

## Set the countdown duration (seconds)
#countdown_duration=10

## Call the function to perform the countdown and print
#countdown_print $countdown_duration


_read_input_with_timeout()
{
    local timeout=$1
    local prompt=$2
    local input
    read -t "$timeout" -ep "$prompt" input
    echo "$input"
}

# Function to check the OS version
envInstall_checkHostOsVersion() {
    # Check if /etc/lsb-release exists (for Ubuntu and some Debian-based systems)
    if [ -f "/etc/lsb-release" ]; then
        ubuntu_version=$(grep "DISTRIB_RELEASE" /etc/lsb-release | cut -d '=' -f 2)
        if [ "$ubuntu_version" == "18.04" ] || [ "$ubuntu_version" == "20.04" ]; then
            print_with_color green yellow "Supported Ubuntu version: $ubuntu_version"
            # The current situation... Allows the script to continue running. Otherwise, all other cases exit the script
            # In the future, this script may support more different GNU Linux distributions
            return 0;
        else
            print_with_color red yellow blink "Unsupported Ubuntu version: $ubuntu_version"
        fi
        # Check if /etc/debian_version exists (for other Debian-based systems)
    elif [ -f "/etc/debian_version" ]; then
        debian_version=$(cat /etc/debian_version)
        print_with_color green white "It should be supported Debian version: $debian_version"
        print_with_color red yellow "This script may support this OS version directly in the future, but in the meantime you will need to manually modify the script to work with this OS version."
        # Check if /etc/redhat-release exists (for CentOS and Red Hat systems)
    elif [ -f "/etc/redhat-release" ]; then
        centos_version=$(cat /etc/redhat-release | awk '{print $4}' | cut -d '.' -f 1)
        print_with_color green white "It should be supported CentOS version: $centos_version"
        print_with_color red yellow "This script may support this OS version directly in the future, but in the meantime you will need to manually modify the script to work with this OS version."
        # Check if /etc/fedora-release exists (for Fedora systems)
    elif [ -f "/etc/fedora-release" ]; then
        fedora_version=$(cat /etc/fedora-release | awk '{print $3}')
        print_with_color green white "It should be supported Fedora version: $fedora_version"
        print_with_color red yellow "This script may support this OS version directly in the future, but in the meantime you will need to manually modify the script to work with this OS version."
        # Check if hostnamectl command is available (for systems with /etc/os-release)
    elif command -v hostnamectl > /dev/null; then
        os_name=$(hostnamectl | grep "Operating System" | cut -d ':' -f 2 | tr -d '[:space:]')
        os_version=$(hostnamectl | grep "Kernel" | cut -d ':' -f 2 | cut -d ' ' -f 2)
        print_with_color green white "It should be supported $os_name version: $os_version"
        print_with_color red yellow "This script may support this OS version directly in the future, but in the meantime you will need to manually modify the script to work with this OS version."
    else
        print_with_color red yellow blink "Unsupported Linux distribution"
        exit 1;
    fi

    # Default: just exit the script if it's not supported
    exit 1;
}

envInstall_chkParam()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    if [ -z "$1" ];then
        print_with_color red blink "========================= Warning! ========================="
        print_with_color red "Need to pass in the user name as a parameter"
        print_with_color red "@param userName:"
        print_with_color red "This name use for the new samba and other related services"
        print_with_color red "ex.:"
        print_with_color red "    ./buildEnvInstall.sh jason.yang"
        print_with_color red "============================================================"
        exit 1;
    fi
}

envInstall_setShellCfg()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    ls -l /bin/sh
    print_with_color red yellow "Ubuntu: For shell environments that don't use the default dash"
    print_with_color red yellow "Be sure to select No"
    countdown_print 5
    sudo dpkg-reconfigure dash
}

envInstall_updateSourceList()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    #pwdval=`pwd`

    #judge OS Version to install diff package
    sudo apt-get install lsb-core -y
    os_version=`lsb_release -a|awk 'NR==4{print $2}'`
    #os_version=16.04
    echo "The cur OS Version is:$os_version"
    baseline_version=18.04

    OS_VER=`printf "%.0f\n" $os_version`
    #echo "$OS_VER"


    y_or_n=`echo $os_version $baseline_version | awk '{if($1 >= $2) print 1; else print 0;}'`
    if [ $y_or_n -eq 1 ]; then
        echo "OS Version Comparison:$os_version >= $baseline_version"
    else
        echo "OS Version Comparison:$os_version < $baseline_version"
        echo "This machine's OS vesion is too low,Stop intall now!"
        echo "pls update to Ubuntu>=18.04"
        exit 1;
    fi

    if [ ! -f "/etc/apt/sources.list.bak" ];then
        echo "backup sources.list file..."
        echo "touch a new file for update source list"
        sudo mv /etc/apt/sources.list /etc/apt/sources.list.bak
        sudo touch /etc/apt/sources.list
        if [ $OS_VER -eq 20 ]; then
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ focal main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ focal main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ focal-updates main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ focal-updates main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ focal-backports main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ focal-backports main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ focal-security main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ focal-security main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ focal-proposed main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ focal-proposed main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
        elif [ $OS_VER -eq 18 ]; then
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-security main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-security main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
            echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse"|sudo tee -a /etc/apt/sources.list
        else
            echo "sth wrong![OS_VER=$OS_VER],pls check!"
            exit 1;
        fi
    else
        echo "The sources.list.bak already exists"
        #sudo rm -rf /etc/apt/sources.list
        #sudo touch /etc/apt/sources.list
    fi
    sudo apt update -y
    sudo apt upgrade -y
    sudo apt autoremove -y
}

envInstall_samba()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    sudo apt-get install net-tools -y
    sudo apt-get install samba samba-common -y

    #check system user
    if id -u ${USERNAME} >/dev/null 2>&1; then
        echo "User <${USERNAME}> exists."
    else
        echo "User <${USERNAME}> does not exist! Now create it..."
        sleep 1
        sudo adduser $USERNAME --force-badname
    fi

    #check samba user
    SMB_USER_CNT=$(sudo pdbedit -L|grep -w $USERNAME|wc -l)
    #echo $SMB_USER_CNT
    if [ ${SMB_USER_CNT} -lt 1 ];then
        echo -e "Samba user <$USERNAME> does not exists. now create it...\n"
        sleep 1
        sudo smbpasswd -a $USERNAME
    else
        echo "Samba user <$USERNAME> exists. "
    fi

    sudo service smbd restart
    echo "upgrade & autoremove ..."
    sleep 1

    if [ ! -f "/etc/samba/smb.conf.bak" ];then
        echo "backup smb.conf file..."
        echo "touch a new file for update smb.conf"
        sudo mv /etc/samba/smb.conf /etc/samba/smb.conf.bak
        sudo touch /etc/samba/smb.conf
        echo "[homes]"|sudo tee -a /etc/samba/smb.conf
        echo "   comment = Home Directories"|sudo tee -a /etc/samba/smb.conf
        echo "   browseable = yes"|sudo tee -a /etc/samba/smb.conf
        echo "   writable = yes"|sudo tee -a /etc/samba/smb.conf
        sudo service smbd restart
    else
        echo "The source.list.bak already exists"
        #sudo rm -rf /etc/samba/smb.conf
        #sudo touch /etc/samba/smb.conf
    fi
}

envInstall_nfsServer()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    sudo apt install nfs-kernel-server -y

    NFS_CONFIG_FILE="/etc/exports"
    NFS_CONFIG_STR="/home 10.0.0.0/24(rw,sync,no_root_squash,no_subtree_check)"

    if [ `grep -c "$NFS_CONFIG_STR" $NFS_CONFIG_FILE` -ne '0' ]; then
        echo "The '${NFS_CONFIG_FILE}' has been set '${NFS_CONFIG_STR}'"
    else
        echo "/home 10.0.0.0/24(rw,sync,no_root_squash,no_subtree_check)"|sudo tee -a /etc/exports
    fi

    sudo service nfs-kernel-server restart
}

envInstall_toolchain()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    toolchain_path_find=`find ~/ -name gcc*sigmastar*.tar.xz`
    #echo $toolchain_path_find
    if [ ! -n "$toolchain_path_find" ]; then
        echo -e "\n\n================ Warning! ================"
        echo -e "can't find toolchain file, plz confirm...!"
        echo -e "==========================================\n\n"
        exit 1;
    else
        #echo "NOT NULL"

        count=0
        declare -a array_toolchain_path
        for i in `echo $toolchain_path_find | grep sigmastar`;do
            array_toolchain_path[${count}]=${i}
            echo "${count}) ${array_toolchain_path[${count}]}"
            count=$(expr ${count} + 1)
        done

        prompt="please select toolchain:"
        input_toolchain_path=$(_read_input_with_timeout "$TIMEOUT_INPUT" "$prompt")
        if [ -z "$input_toolchain_path" ];then
            input_toolchain_path=$DEFAULT_TOOLCHAIN_CHOICE
            echo -e "\nTimeout! select default toolchain path:\n${array_toolchain_path[$input_toolchain_path]}\n"
            sleep 2
        else
            echo -e "\nYour choice is: $input_toolchain_path) ${array_toolchain_path[$input_toolchain_path]}\n"
            sleep 2
        fi
        #read  -ep "please select toolchain:" input_toolchain_path
        #echo "select is $input_toolchain_path) ${array_toolchain_path[$input_toolchain_path]}"

        toolchain_path=${array_toolchain_path[$input_toolchain_path]}
        temp_name=$(basename $toolchain_path)
        TOOLCHAIN_NAME="${temp_name%.tar.xz}"
        echo "After move suffix, the TOOLCHAIN_NAME is£º$TOOLCHAIN_NAME"
        _update_global_variable

        echo "Compressed toolchain package location = $toolchain_path"
        echo "Now, toolchain will install to the path: /tools/toolchain/"
        if [ ! -d "/tools/toolchain" ]; then
            print_with_color red "No /tools/toolchain path, mkdir it"
            sudo mkdir -p /tools/toolchain
            sudo tar -xvf $toolchain_path -C /tools/toolchain
            print_with_color green "toolchain install done."
        elif [ -d "/tools/toolchain/gcc-11.1.0-20210608-sigmastar-glibc-x86_64_arm-linux-gnueabihf" ]; then
            echo "The toolchain has been installed!"
            #exit 0;
        else
            print_with_color red "There has /tools/toolchain path,but no toolchain installed!"
            echo "install now!"
            sleep 3
            sudo tar -xvf $toolchain_path -C /tools/toolchain
            print_with_color green "toolchain install done."
        fi
    fi
}

envInstall_setToolchain2Profile()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    if [ `grep -c "$EXPORT_STR_TOOLCHAIN_BIN_PATH" $EXPORT_FILE` -ne '0' ]; then
        print_with_color green "The '${EXPORT_FILE}' has been set '${EXPORT_STR_TOOLCHAIN_BIN_PATH}'"
    else
        echo "export PATH=\$PATH:/tools/toolchain/gcc-11.1.0-20210608-sigmastar-glibc-x86_64_arm-linux-gnueabihf/bin"|sudo tee -a /etc/profile
    fi

    if [ `grep -c "$EXPORT_STR_CROSS_COMPILE" $EXPORT_FILE` -ne '0' ]; then
        print_with_color green "The '${EXPORT_FILE}' has been set '${EXPORT_STR_CROSS_COMPILE}'"
    else
        echo "export CROSS_COMPILE=arm-linux-gnueabihf-"|sudo tee -a /etc/profile
    fi

    if [ `grep -c "$EXPORT_STR_ARCH" $EXPORT_FILE` -ne '0' ]; then
        print_with_color green "The '${EXPORT_FILE}' has been set '${EXPORT_STR_ARCH}'"
    else
        echo "export ARCH=arm"|sudo tee -a /etc/profile
    fi
    source /etc/profile
}

envInstall_packageAcccordingOsVersion()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    #judge OS Version to install diff package
    sudo apt-get install lsb-core -y
    os_version=`lsb_release -a|awk 'NR==4{print $2}'`
    echo "The cur OS Version is:$os_version"
    baseline_version=18.04

    y_or_n=`echo $os_version $baseline_version | awk '{if($1 > $2) print 1; else print 0;}'`
    if [ $y_or_n -eq 1 ]; then
        print_with_color green "$os_version > $baseline_version"
        print_with_color green "install lib32ncurses5-dev android-sdk-libsparse-utils"
        sleep 2
        sudo apt-get install lib32ncurses5-dev -y
        sudo apt-get install android-sdk-libsparse-utils -y
    else
        print_with_color green "$os_version <= $baseline_version"
        print_with_color green "install lib32ncurses5 android-tools-fsutils"
        sleep 2
        sudo apt-get install lib32ncurses5 -y
        sudo apt-get install android-tools-fsutils -y
    fi
}

envInstall_commonPackage()
{
    print_with_color yellow green "runing at: ${FUNCNAME[0]}"; sleep 1
    #common same package install
    sudo apt-get install gcc g++ make -y
    sudo apt-get install python python2.7-dev -y
    sudo apt-get install libc6-dev-i386 lib32z1 libuuid1 cmake libncurses5 libncurses5-dev libncursesw5 libncursesw5-dev -y
    sudo apt-get install bc xz-utils automake libtool libevdev-dev -y
    sudo apt-get install pkg-config mtd-utils bison flex libssl-dev libmpc-dev squashfs-tools gawk -y
    sudo apt autoremove -y
}

envInstall_checkHostOsVersion
envInstall_chkParam $1
envInstall_setShellCfg
envInstall_updateSourceList
envInstall_samba
envInstall_nfsServer
envInstall_toolchain
envInstall_setToolchain2Profile
envInstall_packageAcccordingOsVersion
envInstall_commonPackage
print_with_color yellow green "Env Install all done!!If you want to chk it, Run buildEnvChk.sh in the same directory as this script"
print_with_color green blink "done!"
