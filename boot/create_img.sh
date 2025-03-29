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

srctree=$(dirname "$0")

#In order to prevent some server can not execute ${CROSS_COMPILE}gdb, check it first
${CROSS_COMPILE}gdb -v > /dev/null
if [ $? -ne 0 ];then
  exit -1
fi

rm -Rf u-boot.bin.mz
rm -Rf u-boot.bin.xz
rm -Rf u-boot.bin.sz

version="$(strings -a -T binary u-boot.bin | grep 'MVX' | grep 'UBT2110' | sed 's/\\*MVX/MVX/g' | cut -c 1-32)"
ld_addr=$(${CROSS_COMPILE}gdb arch/arm/mach-sstar/sstar_sys_utility.o -ex 'p/x uboot_ld_addr' -ex 'quit' | grep '$1' | cut -d' ' -f3)
ep_addr=$(${CROSS_COMPILE}gdb arch/arm/mach-sstar/sstar_sys_utility.o -ex 'p/x uboot_ep_addr' -ex 'quit' | grep '$1' | cut -d' ' -f3)

#$srctree/mz c u-boot.bin u-boot.bin.mz

xz -z -k u-boot.bin

file_size_max=64MiB
cp u-boot.bin u-boot_sstar.bin
${srctree}/tools/mkimage -A $1 -O u-boot -C none -a "$ld_addr" -e "$ep_addr" -n "$version" -d u-boot_sstar.bin u-boot_sstar.img.bin > /dev/null
${srctree}/sz -z -f -k --check=crc32 --lzma2=dict=4KiB --block-list=$file_size_max --block-size=$file_size_max --threads=4 u-boot_sstar.img.bin
block_split_size_list=$(${srctree}/szdec u-boot_sstar.img.bin.xz u-boot_sstar.img.bin split_block=4)
${srctree}/sz -z -f -k --check=crc32 --lzma2=dict=4KiB --block-list=$block_split_size_list$file_size_max --block-size=$file_size_max --threads=4 u-boot_sstar.img.bin
${srctree}/szsplit u-boot_sstar.img.bin.xz u-boot.bin.sz
rm -rf u-boot_sstar.img.bin.xz
rm -rf u-boot_sstar.img.bin
rm -rf u-boot_sstar.bin
sz_ld_addr=$(printf "%x" $[ld_addr - 0x40])
sz_ep_addr=0


#out_file_mz=u-boot$2.mz.img.bin
out_file_xz=u-boot$2.xz.img.bin
out_file_sz=u-boot$2.sz.img.bin
out_file=u-boot$2.img.bin

if [ `echo $version | grep -c "MVX1S" ` -gt 0 ];then
  #out_file_mz=u-boot_S.mz.img.bin
  out_file_xz=u-boot_S.xz.img.bin
  out_file_sz=u-boot_S.sz.img.bin
fi

# echo ""
# echo $out_file_mz
# #echo ./tools/mkimage -A $1 -O u-boot -C mz -a "$ld_addr" -e "$ep_addr" -n "$version" -d u-boot.bin.mz "$out_file_mz"
# ${srctree}/tools/mkimage -A $1 -O u-boot -C mz -a "$ld_addr" -e "$ep_addr" -n "$version" -d u-boot.bin.mz "$out_file_mz"
# rm -Rf u-boot.bin.mz

echo ""
echo $out_file_xz
#echo ./tools/mkimage -A $1 -O u-boot -C lzma -a "$ld_addr" -e "$ep_addr" -n "$version" -d u-boot.bin.xz "$out_file_xz"
${srctree}/tools/mkimage -A $1 -O u-boot -C lzma -a "$ld_addr" -e "$ep_addr" -n "$version" -d u-boot.bin.xz "$out_file_xz"
rm -Rf u-boot.bin.xz

echo ""
echo $out_file_sz
#echo ./tools/mkimage -A $1 -O u-boot -C lzma2 -a "$sz_ld_addr" -e "$sz_ep_addr" -n "$version" -d u-boot.bin.sz "$out_file_sz"
${srctree}/tools/mkimage -A $1 -O u-boot -C lzma2 -a "$sz_ld_addr" -e "$sz_ep_addr" -n "$version" -d u-boot.bin.sz "$out_file_sz"
rm -Rf u-boot.bin.sz
echo ""

echo ""
echo $out_file
#echo ./tools/mkimage -A $1 -O u-boot -C none -a "$ld_addr" -e "$ep_addr" -n "$version" -d u-boot.bin "$out_file"
${srctree}/tools/mkimage -A $1 -O u-boot -C none -a "$ld_addr" -e "$ep_addr" -n "$version" -d u-boot.bin "$out_file"

echo ""
#for iplx_path in $srctree/$ARCH_PATH/IPLX.*.bin
for iplx_path in `find $srctree/$ARCH_PATH -name IPLX.*.bin`
do
  # get full path
  iplx_path=$(realpath $iplx_path)

  # cut dirname and extension, such as:
  # arch/arm/mach-sstar/xxx/IPLX.ext_NANYA_DDR4_2666_4Gb.bin -> IPLX.ext_NANYA_DDR4_2666_4Gb
  file_name=$(basename $iplx_path)
  file_name=${file_name%.*}

  # cut 'IPLX.', such as:
  # IPLX.ext_NANYA_DDR4_2666_4Gb -> ext_NANYA_DDR4_2666_4Gb
  suffix=$(echo ${file_name##*IPLX.})

  # Gen bootloader_xxx.img, such as:
  # bootloader_ext_NANYA_DDR4_2666_4Gb.img
  echo "Generating bootloader_${suffix}.img ..."
  if [ -f $srctree/$ARCH_PATH/PM8051.bin ] && [[ ${suffix} =~ PM51 ]];then
    cat $iplx_path $out_file_xz $srctree/$ARCH_PATH/PM8051.bin > bootloader_${suffix}.img;
  else
    cat $iplx_path $out_file_xz > bootloader_${suffix}.img
  fi
done
echo ""
