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

chip_file=".sstar_chip.txt"
dirs="arch/arm/configs arch/arm64/configs"

if [ -e $chip_file ]
then
save_defconfig=$(sed -n "2p" $chip_file)
for dir in $dirs
do
defconfig_path=$(find $dir -maxdepth 1 -name "$save_defconfig")
if [ ! -z $defconfig_path ]
then
if [[ $defconfig_path =~ $ARCH ]]
then
echo "make savedefconfig"
make savedefconfig
if [ -e $defconfig_path ]
then
echo "cp defconfig $defconfig_path"
cp defconfig $defconfig_path
fi
fi
fi
done
fi
