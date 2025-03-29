#!/bin/bash

srctree=$(dirname "$0")
tools_path=$srctree

if [ "$1" == "" ] || [ "$2" == "" ]; then
    echo "Usage: sign_misc.sh [file_path] [partition_size]"
	exit
fi

input=$1
part_size=$(($2))
sign_out=$(dirname $input)/$(echo $(basename $input) | sed "s/\./\.sig\./g")
signpss_out=$(dirname $input)/$(echo $(basename $input) | sed "s/\./\.sigpss\./g")
sbot_tmp=$sign_out.tmp
sbot_hdr=$sign_out.hdr
sbot_out=$sign_out.sbot
padding_out=zeros.bin

echo "input: $input, partition size: $part_size"

$tools_path/key_proc.py --sign --rsa=./rsa2048/private-otp.pem -f $input
$tools_path/mkimage -A arm -C none -a 0 -e 0 -n "sbot" -d $sign_out $sbot_tmp

dd if=$sbot_tmp of=$sbot_hdr bs=1 count=$(($(stat -c%s $sbot_tmp) - $(stat -c%s $sign_out)))
dd if=/dev/zero of=$padding_out bs=1 count=$(($part_size - $(stat -c%s $sbot_tmp)))

cat $sign_out > $sbot_out
cat $padding_out >> $sbot_out
cat $sbot_hdr >> $sbot_out
rm -rf $sign_out $signpss_out $sbot_tmp $sbot_hdr $padding_out