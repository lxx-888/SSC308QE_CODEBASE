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

srctree=$(dirname "$0")

pushd $srctree > /dev/null

function compare_size() {
    #deal with such as 0x50 0x4c264 0x4c3c4
    value1=$(printf "0x%08x" "$(( $2 ))")
    value2=$(printf "0x%08x" "$(( $3 ))")
    value3=$(printf "0x%08x" "$(( $4 ))")

    if [[ $value1 > $value2 ]]; then
        comp_size=$value1
        max_val=1
    else
        comp_size=$value2
        max_val=2
    fi

    if [[ $comp_size < $value3 ]]; then
        max_val=3
    fi

    if [[ $max_val == 1 ]]; then
        echo "$1=$2" >>$5
    fi
    if [[ $max_val == 2 ]]; then
        echo "$1=$3" >>$5
    fi
    if [[ $max_val == 3 ]]; then
        echo "$1=$4" >>$5
    fi

}

#$1 image type
#$2 def size
#$3 normal boot size
#$4 secure boot size
#$5 config
compare_size $1 $2 $3 $4 $5

popd > /dev/null
