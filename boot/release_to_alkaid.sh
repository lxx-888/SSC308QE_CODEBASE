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

#!/bin/bash

while getopts "a:c:" opt; do
  case $opt in
    a)
      alkaid_dir=$OPTARG
      ;;
    c)
      chip=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

uboot_dir=$PWD

if [ "${chip}" = "i7" ]; then
  declare -x PATH="/tools/toolchain/gcc-10.2.1-20210303-sigmastar-glibc-x86_64_aarch64-linux-gnu/bin":$PATH
  declare -x ARCH="arm64"
  declare -x CROSS_COMPILE="aarch64-linux-gnu-"
  whereis ${CROSS_COMPILE}gcc
  GCC_VERSION=$(${CROSS_COMPILE}gcc --version | head -n 1 | sed -e 's/.*\([0-9]\.[0-9]\.[0-9]\).*/\1/')
  echo GCC_VERSION=${GCC_VERSION}

  make infinity7_emmc_defconfig
  make clean; make -j4
  if [ ! -d $alkaid_dir/project/board/${chip}/boot/emmc/uboot ]; then
    mkdir $alkaid_dir/project/board/${chip}/boot/emmc/uboot
  fi
  cp $uboot_dir/u-boot_emmc.xz.img.bin $alkaid_dir/project/board/${chip}/boot/emmc/uboot

  make infinity7_defconfig
  make clean; make -j4
  if [ ! -d $alkaid_dir/project/board/${chip}/boot/nor/uboot ]; then
    mkdir $alkaid_dir/project/board/${chip}/boot/nor/uboot
  fi
  cp $uboot_dir/u-boot.xz.img.bin $alkaid_dir/project/board/${chip}/boot/nor/uboot

  make infinity7_spinand_defconfig
  make clean; make -j4
  if [ ! -d $alkaid_dir/project/board/${chip}/boot/spinand/uboot ]; then
    mkdir $alkaid_dir/project/board/${chip}/boot/spinand/uboot
  fi
  cp $uboot_dir/u-boot_spinand.xz.img.bin $alkaid_dir/project/board/${chip}/boot/spinand/uboot
fi

if [ "${chip}" = "m6p" ]; then
  declare -x PATH="/tools/toolchain/gcc-11.1.0-20210608-sigmastar-glibc-x86_64_arm-linux-gnueabihf/bin":$PATH
  declare -x ARCH="arm"
  declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
  whereis ${CROSS_COMPILE}gcc
  GCC_VERSION=$(${CROSS_COMPILE}gcc --version | head -n 1 | sed -e 's/.*\([0-9][0-9]\.[0-9]\.[0-9]\).*/\1/')
  echo GCC_VERSION=${GCC_VERSION}

  make mercury6p_defconfig
  make clean; make -j16
  if [ ! -d $alkaid_dir/project/board/${chip}/boot/nor/uboot ]; then
    mkdir $alkaid_dir/project/board/${chip}/boot/nor/uboot
  fi
  cp $uboot_dir/u-boot.xz.img.bin $alkaid_dir/project/board/${chip}/boot/nor/uboot

  make mercury6p_spinand_defconfig
  make clean; make -j16
  if [ ! -d $alkaid_dir/project/board/${chip}/boot/spinand/uboot ]; then
    mkdir $alkaid_dir/project/board/${chip}/boot/spinand/uboot
  fi
  cp $uboot_dir/u-boot_spinand.xz.img.bin $alkaid_dir/project/board/${chip}/boot/spinand/uboot

  make mercury6p_demo_defconfig
  make clean; make -j16
  cp $uboot_dir/u-boot.xz.img.bin $alkaid_dir/project/board/${chip}/boot/nor/uboot/u-boot_demo.xz.img.bin

  make mercury6p_demo_spinand_defconfig
  make clean; make -j16
  cp $uboot_dir/u-boot_spinand.xz.img.bin $alkaid_dir/project/board/${chip}/boot/spinand/uboot/u-boot_demo_spinand.xz.img.bin

  make mercury6p_emmc_defconfig
  make clean; make -j16
  cp $uboot_dir/u-boot.xz.img.bin $alkaid_dir/project/board/${chip}/boot/emmc/uboot/u-boot_emmc.xz.img.bin
fi
