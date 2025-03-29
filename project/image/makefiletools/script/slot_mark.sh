#!/bin/sh

slot_metadata=$(/etc/fw_printenv |grep slot_metadata|cut -d '=' -f2)
slot_number=$(/etc/fw_printenv |grep slot_number|cut -d '=' -f2)
slot_select=$(/etc/fw_printenv |grep slot_select|cut -d '=' -f2)

if [ -z $slot_metadata ] || [ -z $slot_number ] || [ -z $slot_select ];then
    echo "Missing slot variable!!!"
    exit 0
fi

if [ $slot_select -ge $slot_number ];then
    echo "The slot_select must not be greater than or equal to the slot_number!!!"
    exit 0
fi

slot_metadata_new=""

IFS=","
i=0
update=0
for metadata in $slot_metadata
do
    if [ $slot_select -eq $i ] && [ $((metadata & 0x80)) -eq 0 ];then
        metadata=$((metadata+0x80))
        update=1
    fi
    slot_metadata_new=$slot_metadata_new,"$(printf '0x%02x' $metadata)"
    let i++
done
IFS=" "

slot_metadata_new=${slot_metadata_new:1}

if [ $update -eq 1 ];then
/etc/fw_setenv slot_metadata $slot_metadata_new
fi