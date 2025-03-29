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

list_defconfig=""
chip_file=".sstar_chip.txt"
dirs="arch/arm/configs arch/arm64/configs"

echo "available defconfig:"
for dir in $dirs
do
list_defconfig+=" "$(find $dir -maxdepth 1 -name "*${1}*_defconfig" | xargs grep -l "SSTAR")
list_defconfig=${list_defconfig//"$dir/"/""}
done
list_defconfig_sort=$(echo ${list_defconfig} | tr " " "\n" | sort -n)
max_defconfig_index=0
for line in $list_defconfig_sort
do
	list_defconfig_name[max_defconfig_index]=$line
	printf "%-2d - %s\n" $max_defconfig_index $line
	let max_defconfig_index++
done
defconfig=""
if [ -e $chip_file ]
then
defconfig=$(sed -n "2p" $chip_file)
fi
echo -n "defconfig [$defconfig]: "
read select_index

if [[ "$select_index" =~ ^[0-9]+$ ]] && [ $select_index -lt $max_defconfig_index ]
then
make ${list_defconfig_name[$select_index]}
fi
