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

source $srctree/../sign_image.config
source $srctree/log.sh
tools_path=$srctree
main_path=$srctree/..

enable_log=1

#don't change this
secureboot_enable=yes

#echo ""
#echo "secureboot_enable:  $secureboot_enable"

if [[ $enable_log == 1 ]]; then
    name="$3"
    special_enable=$(eval echo "\${${name}_enable}")
    special_aeskeylen=$(eval echo "\${${name}_aeskeylen}")
    special_aeskeynum=$(eval echo "\${${name}_aeskeynum}")
    special_aes_type=$(eval echo "\${${name}_aes_type}")
    special_iv=$(eval echo "\${${name}_IV}")
    special_abk=$(eval echo "\${${name}_abk}")
    echo ""
    echo "secureboot info"

    echo "rsa_len:            $rsalen"

    if [[ ${special_enable} == 1 ]]; then
        echo "${name}_aeskeylen:    $special_aeskeylen"
        echo "${name}_aeskeynum:    $special_aeskeynum"
        echo "${name}_aes_type:     $special_aes_type"
        echo "${name}_abk:          $special_abk"
    else
        if [[ ${name} == ipl || ${name} == ipl_cust ]]; then
            echo "ipl_aeskeynum:      $ipl_aeskeynum"
            echo "ipl_aeskeylen:      $ipl_aeskeylen"
            echo "rsalen:             $rsalen"
            echo "sw_debug:           $sw_debug"
            echo ""
        else
            echo "image_aeskeylen:    $image_aeskeylen"
            echo "image_aeskeynum:    $image_aeskeynum"
        fi

    fi
    echo ""
    NULL_LOG=""
else
    NULL_LOG="> /dev/null"
fi

#it not change IPL header
cid=0
anti=0

function uboot_env_aes() {
    if [[ $image_aeskeylen != 0 && $image_aeskeynum != 0 ]]; then
        echo "--aes" > $1
    else
        echo "" > $1
    fi
}

function image_getname() {
    filename=$1
    insert_name=null
    case "$2" in
    "signusb")
        insert_name=.cut.append.sig
        ;;
    "sign")
        insert_name=.sig
        ;;
    "aes")
        insert_name=.aes
        ;;
    "cipher")
        insert_name=.cipher
        ;;
    "debug")
        insert_name=2
        ;;
    "append")
        insert_name=.append
        ;;
    *)
        echo "ERROR"
        ;;
    esac

    if [[ $1 == *.* ]]; then
        extension="${filename##*.}"
        basename="${filename%.*}"
        IMAGE_NAME="${basename}${insert_name}.${extension}"
    else
        IMAGE_NAME="${filename}${insert_name}"
    fi
    echo $IMAGE_NAME
}


function do_export_rsakey() {
if [[ $enable_log == 1 ]]; then
    log_show_save "$tools_path/key_proc.py --exportkey_reverse --rsa=$main_path/rsa$1/$2 $NULL_LOG" " #It is convenient to burn otp rsakey under uboot, not necessary"
fi
    $tools_path/key_proc.py --exportkey_reverse --rsa=$main_path/rsa$1/$2 $NULL_LOG

if [[ $enable_log == 1 ]]; then
    log_show_save "#file is in $tools_path/rsaKey.txt. do cmd [cat $tools_path/rsaKey.txt]"
fi
}

#$1 image_name
#$2 rsakeylen
#$3 which rsakey
#$4 sign or signusb
function sign_mkimg() {
if [ -z "$4" ]; then
    sign_cmd=sign
else
    sign_cmd=$4
fi
if [[ $enable_log == 1 ]]; then
    log_show_save "$tools_path/key_proc.py --$sign_cmd --rsa=$main_path/rsa$2/$3 -f $1 $NULL_LOG"
fi
    $tools_path/key_proc.py --$sign_cmd --rsa=$main_path/rsa$2/$3 -f $1 $NULL_LOG
}

#$1 image_name
#$2 aeskeylen
#$3 aeskeynum
function encrypt_mkimg() {
    mode=" "
    IV=" "
    if [[ $4 == cbc ]]; then
        mode="--mode=cbc"
        IV="--IV=$main_path/aeskey/$5"
    elif [[ $4 == ctr ]]; then
        mode="--mode=ctr"
        IV="--IV=$main_path/aeskey/$5"
    fi
    if [[ $enable_log == 1 ]]; then
        log_show_save "$tools_path/key_proc.py --encrypt $mode $IV --aes=$main_path/aeskey/aesKey$2_$3.bin -f  $1 $NULL_LOG"
    fi
    $tools_path/key_proc.py --encrypt $mode $IV --aes=$main_path/aeskey/aesKey$2_$3.bin -f $1 $NULL_LOG
}

function append_mkimg() {
    log_show_save "$tools_path/key_proc.py --append --anti=$1 --aeskeylen=$2 --aeskeynum=$3 --mode=$4 --IV=$main_path/aeskey/$5 -f $6 $NULL_LOG"
    $tools_path/key_proc.py --append --anti=$1 --aeskeylen=$2 --aeskeynum=$3 --mode=$4 --IV=$main_path/aeskey/$5 -f $6 $NULL_LOG
}

#$1 image_name
#$2 rsakeylen
#$3 which rsakey
function insert_mkimg() {
if [[ -n $4 ]]; then
    IV="--IV=$main_path/aeskey/$4"
fi

if [[ $enable_log == 1 ]]; then
    log_show_save "$tools_path/key_proc.py --insert --rsa=$main_path/rsa$2/$3 $IV -f $1 $NULL_LOG"
fi

    $tools_path/key_proc.py --insert --rsa=$main_path/rsa$2/$3 $IV -f $1 $NULL_LOG
}

#$1 input image
#$2 output image
#$3 aes encrypt flag
function debug_mkimg() {

if [[ $sw_debug != 1 && $sw_debug != 2 && $sw_debug != 3 ]]; then
    #for normal boot used sig image
    #rsa2048, sw_debug=4
    #rsa4096, sw_debug=5
    sw_debug=$((rsalen/2048+3))

fi

if [[ $enable_log == 1 ]]; then
    log_show_save "$tools_path/add_ipl_header.py $1 $2 $cid $sw_debug $4 $3 $NULL_LOG"
fi
    $tools_path/add_ipl_header.py $1 $2 $cid $sw_debug $4 $3 $NULL_LOG
}

#$1 image_name
#$2 rsakeylen
#$3 which rsakey
function split_mkimg() {
if [[ $enable_log == 1 ]]; then
    log_show_save "$tools_path/key_proc.py --split -f $1 $NULL_LOG"
fi
    $tools_path/key_proc.py --split -f $1
}

# $1 Binary path/name
# $2 final name=$1$2
# copy sign/aes last binary to $1$2
function mk_secure_image() {
    init_name=$1
    filename=$(basename "$1")
    if [[ $filename != *.* ]]; then
       init_name=$1.bin
       cp $1 $1.bin
    fi

    if [[ $secureboot_enable == no || $secureboot_enable == NO ]]; then
        output_name=$init_name
    elif [[ $init_name == */IPL_CUST*.bin ]]; then
        insert_mkimg $init_name $rsalen public-image.pem
        insert_name=$(image_getname $init_name cipher)
        debug_name=$(image_getname $insert_name debug)

        debug_mkimg  $insert_name $debug_name 0 $ipl_cust_abk
        deal_name=$debug_name

        sign_mkimg $deal_name $rsalen private-cust.pem
        output_name=$(image_getname $deal_name sign)

    elif [[ $init_name == */IPL*.bin ]]; then
        do_export_rsakey $rsalen private-otp.pem
        insert_mkimg $init_name $rsalen public-cust.pem $ipl_IV
        insert_name=$(image_getname $init_name cipher)

        debug_name=$(image_getname $insert_name debug)
        debug_mkimg  $insert_name $debug_name 0 $ipl_abk
        deal_name=$debug_name

        if [[ ${ipl_aeskeylen} != 0 && $ipl_aeskeynum != 0 ]]; then
            encrypt_mkimg  $deal_name ${ipl_aeskeylen} ${ipl_aeskeynum} cbc $ipl_IV
            deal_name=$(image_getname $deal_name aes)
        fi
        if [[ $4 == usb ]]; then
            #sign_cmd=signusb
            #i6dw ROM load IPL actual size
            sign_cmd=sign
        else
            sign_cmd=sign
        fi
        sign_mkimg $deal_name $rsalen private-otp.pem $sign_cmd
        output_name=$(image_getname $deal_name $sign_cmd)
    else
        sign_name=$init_name
        name="$3"
        special_enable=$(eval echo "\${${name}_enable}")
        special_aeskeylen=$(eval echo "\${${name}_aeskeylen}")
        special_aeskeynum=$(eval echo "\${${name}_aeskeynum}")
        special_aes_type=$(eval echo "\${${name}_aes_type}")
        special_iv=$(eval echo "\${${name}_IV}")
        special_abk=$(eval echo "\${${name}_abk}")

        tmp_name=$init_name
        if [[ ${special_enable} == 1 ]]; then
            if [[ $special_aeskeylen != 0 && $special_aeskeynum != 0 ]]; then
                encrypt_mkimg $init_name $special_aeskeylen $special_aeskeynum $special_aes_type $special_iv
                tmp_name=$(image_getname $init_name aes)
            fi
            append_mkimg $special_abk $special_aeskeylen $special_aeskeynum $special_aes_type $special_iv $tmp_name
            sign_name=$(image_getname $tmp_name append)
        elif [[ $image_aeskeylen != 0 && $image_aeskeynum != 0 ]]; then
            if [[ $init_name != */rootfs* ]]; then
                encrypt_mkimg $init_name $image_aeskeylen $image_aeskeynum
                tmp_name=$(image_getname $init_name aes)
            fi
            append_mkimg $image_abk $image_aeskeylen $image_aeskeynum $image_aes_type $image_IV $tmp_name
            sign_name=$(image_getname $tmp_name append)
        else
            append_mkimg $image_abk $image_aeskeylen $image_aeskeynum $image_aes_type $image_IV   $tmp_name
            sign_name=$(image_getname $tmp_name append)
        fi

        if [[ $4 == usb ]]; then
            sign_mkimg $sign_name $rsalen private-cust.pem
        else
            if [[ $3 == tf_a || $3 == optee || $3 == earlyinit ]]; then
                sign_mkimg $sign_name $rsalen private-cust.pem
            else
                sign_mkimg $sign_name $rsalen private-image.pem
            fi
        fi
        output_name=$(image_getname $sign_name sign)
    fi
    if [[ $enable_log == 1 ]]; then
        echo "cp $output_name $1$2"
    fi
    cp $output_name $1$2
}

if [[ $1 == uboot_env ]]; then
    uboot_env_aes $2

else
    echo  "[[$1]]"
    if [[ $1 == *IPLX* ]]; then
        split_mkimg $1
        sign_path=$(dirname "$(readlink -f "$1")")
        mv $srctree/IPL.bin $sign_path/
        mv $srctree/IPL_CUST.bin $sign_path/
        mk_secure_image $sign_path/IPL.bin _final $3
        mk_secure_image $sign_path/IPL_CUST.bin _final $3
        cat $sign_path/IPL.bin_final  $sign_path/IPL_CUST.bin_final > $1$2
    else
        mk_secure_image $1 $2 $3 $4
    fi
fi


popd > /dev/null
