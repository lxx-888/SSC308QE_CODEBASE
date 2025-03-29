#
# Copyright (c) [2019~2020] SigmaStar Technology.
#
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#

#!/bin/sh

# $1: the first parameter is used to specify the architecture for mkimage (mandatory)
# $2: the second parameter is used to specify the postfix of the output image name (optional)

# declare global vars
srctree="$(realpath $(dirname "$0"))"
objtree=$(pwd)
uboot_bin=$objtree/u-boot$2.xz.img.bin
signed_uboot_bin=$objtree/u-boot$2.xz.img.sig.bin
temp_dir=$srctree/.sign_image_temp
debug_mode=0

# declare tools path
config="$srctree/scripts/config --file $objtree/.config"
key_proc=$srctree/scripts/key_proc.py
add_ipl_header=$srctree/scripts/add_ipl_header.py

# select CONFIG_SSTAR_SIGN_IMAGES to enable sign bootloader.img
if [ "$($config -s CONFIG_SSTAR_SIGN_IMAGES)" != 'y' ];then
    exit 0
fi

if [ "$($config -s CONFIG_SECURE_BOOT_DEBUG_MODE)" == 'y' ];then
    debug_mode=1
fi

if [ -f $srctree/$ARCH_PATH/IPLX_EMMC.bin ];then
    echo "----------------------------signing bootloader.img------------------------------"
    mkdir $temp_dir
    pushd $temp_dir > /dev/null
    echo "Spliting IPL_EMMC.bin ..."
    $key_proc --split -f $srctree/$ARCH_PATH/IPLX_EMMC.bin > /dev/null

    echo "Insert $($config -s CONFIG_IPLCUST_PUBKEY_FILE) to IPL.bin ..."
    $key_proc --insert --rsa=$srctree/$($config -s CONFIG_IPLCUST_PUBKEY_FILE) -f IPL.bin > /dev/null
    echo "adding IPL header with debug mode:$debug_mode ..."
    $add_ipl_header IPL.cipher.bin IPL.cipher1.bin 0 $debug_mode > /dev/null
    echo "Sign IPL.bin with $($config -s CONFIG_IPL_PRIVKEY_FILE) ...";
    $key_proc --sign --rsa=$srctree/$($config -s CONFIG_IPL_PRIVKEY_FILE) -f IPL.cipher1.bin > /dev/null

    echo "Insert $($config -s CONFIG_UBOOT_PUBKEY_FILE) to IPL_CUST.bin ..."
    $key_proc --insert --rsa=$srctree/$($config -s CONFIG_UBOOT_PUBKEY_FILE) -f IPL_CUST.bin > /dev/null
    echo "Add IPL_CUST header with debug mode:$debug_mode ..."
    $add_ipl_header IPL_CUST.cipher.bin IPL_CUST.cipher1.bin 0 $debug_mode > /dev/null
    echo "Sign IPL_CUST.bin with $($config -s CONFIG_IPLCUST_PRIVKEY_FILE) ...";
    $key_proc --sign --rsa=$srctree/$($config -s CONFIG_IPLCUST_PRIVKEY_FILE) -f IPL_CUST.cipher1.bin > /dev/null

    echo "Sign $uboot_bin with $($config -s CONFIG_UBOOT_PRIVKEY_FILE) ..."
    $key_proc --sign --rsa=$srctree/$($config -s CONFIG_UBOOT_PRIVKEY_FILE) -f $uboot_bin > /dev/null

    echo "Generate bootloder.sig.img ..."
    cat IPL.cipher1.sig.bin IPL_CUST.cipher1.sig.bin $signed_uboot_bin > $objtree/bootloader.sig.img
    popd > /dev/null
    rm -rf $temp_dir
    echo ""
fi

if [ -f $srctree/$ARCH_PATH/IPLX_EMMC_LPDDR4.bin ];then
    echo "----------------------------signing bootloader_lpddr4.img------------------------------"
    mkdir $temp_dir
    pushd $temp_dir > /dev/null
    echo "Spliting IPL_EMMC_LPDDR4.bin ..."
    $key_proc --split -f $srctree/$ARCH_PATH/IPLX_EMMC_LPDDR4.bin > /dev/null

    echo "Insert $($config -s CONFIG_IPLCUST_PUBKEY_FILE) to IPL.bin ..."
    $key_proc --insert --rsa=$srctree/$($config -s CONFIG_IPLCUST_PUBKEY_FILE) -f IPL.bin > /dev/null
    echo "adding IPL header with debug mode:$debug_mode ..."
    $add_ipl_header IPL.cipher.bin IPL.cipher1.bin 0 $debug_mode > /dev/null
    echo "Sign IPL.bin with $($config -s CONFIG_IPL_PRIVKEY_FILE) ...";
    $key_proc --sign --rsa=$srctree/$($config -s CONFIG_IPL_PRIVKEY_FILE) -f IPL.cipher1.bin > /dev/null

    echo "Insert $($config -s CONFIG_UBOOT_PUBKEY_FILE) to IPL_CUST.bin ..."
    $key_proc --insert --rsa=$srctree/$($config -s CONFIG_UBOOT_PUBKEY_FILE) -f IPL_CUST.bin > /dev/null
    echo "Add IPL_CUST header with debug mode:$debug_mode ..."
    $add_ipl_header IPL_CUST.cipher.bin IPL_CUST.cipher1.bin 0 $debug_mode > /dev/null
    echo "Sign IPL_CUST.bin with $($config -s CONFIG_IPLCUST_PRIVKEY_FILE) ...";
    $key_proc --sign --rsa=$srctree/$($config -s CONFIG_IPLCUST_PRIVKEY_FILE) -f IPL_CUST.cipher1.bin > /dev/null

    echo "Sign $uboot_bin with $($config -s CONFIG_UBOOT_PRIVKEY_FILE) ..."
    $key_proc --sign --rsa=$srctree/$($config -s CONFIG_UBOOT_PRIVKEY_FILE) -f $uboot_bin > /dev/null

    echo "Generate bootloder_lpddr4.sig.img ..."
    cat IPL.cipher1.sig.bin IPL_CUST.cipher1.sig.bin $signed_uboot_bin > $objtree/bootloader_lpddr4.sig.img
    popd > /dev/null
    rm -rf $temp_dir
    echo ""
fi

