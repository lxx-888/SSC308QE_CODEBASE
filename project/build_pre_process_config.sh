#!/bin/sh

PROJ_ROOT=$PWD

if [ "$#" != "1" ] && [ "$#" != "2" ]&& [ "$#" != "3" ]; then
    echo "usage: $0 configs/config.chip"
    echo "or $0 configs/config.chip config.out"
    exit -1
fi

INPUT_CONFIG=$1
if [ ! -e  $INPUT_CONFIG ]; then
    echo "can't find $INPUT_CONFIG"
    exit -1
fi

OUTPUT_CONFIG=$PROJ_ROOT/configs/current.configs
if [ $# -ge 2 ]; then
    OUTPUT_CONFIG=$2
fi

function update_config() {
    local key="$1"
    local sep="$2"
    local value="$3"
    local file="$4"
    local config="$key$sep$value"
    config=$(echo $config) #strip space
    line_number=$(grep -n "^$key\s*=" $file | awk -F: 'NR==1 {print $1}')

    if [ -z "$line_number" ]; then
        echo "$config" >> $file
    else
        sed -i "s#^$key.*#$config#" $file
        sed -i "$((line_number + 1)),$$ { /^$key\s*=/d }" $file
    fi
}

function update_out_config() {
    update_config "$1" " = " "$2" $OUTPUT_CONFIG
}

function update_uboot_config() {
    update_config "$1" "=" "$2" $UBOOT_DEFCONFIG
}

CHIP=`cat $1 | awk '/CHIP/ {print substr($3,$1)}'`
MMAP=`cat $1 | awk '/MMAP/ {print substr($3,$1)}'`

kernel_reserved_env="mmap_reserved="
FBADDR=`$PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/$CHIP/mmap/$MMAP $CHIP E_MMAP_ID_FB phyaddr`
assemble_kernel_reserved_env()
{
    n=$#
    n=$(expr $n \/ 5 \- 1)
    for i in $(seq 0 $n)
    do
        j=$(expr $i \* 5)
        j=$(expr $j \+ 1)
        name=$(eval "echo \${$j}")
        name=$(tr [A-Z] [a-z] <<< $name)
        j=$(expr $j \+ 1)
        miu=$(eval "echo \${$j}")
        j=$(expr $j \+ 1)
        sz=$(eval "echo \${$j}")
        j=$(expr $j \+ 1)
        start=$(eval "echo \${$j}")
        j=$(expr $j \+ 1)
    end=$(eval "echo \${$j}")
    kernel_reserved_env+="$name,miu=$miu,sz=$sz,max_start_off=$start,max_end_off=$end "
    if [ "$name" == "bootlogo" ]; then
        sed -i "s/LOGO_ADDR = .*/LOGO_ADDR = $start/g" $OUTPUT_CONFIG
    fi
    if [ "$FBADDR" != "" ]; then
        sed -i "s/LOGO_ADDR = .*/LOGO_ADDR = $FBADDR/g" $OUTPUT_CONFIG
    fi
done
}
data=`$PROJ_ROOT/image/makefiletools/bin/reserved $PROJ_ROOT/board/$CHIP/mmap/$MMAP $CHIP `
assemble_kernel_reserved_env $data

check_param_empty_and_delete()
{
    for x in $@;
    do
        param_value=$(sed -n "/^$x\b/p" $OUTPUT_CONFIG | awk '{print $3}')
        if [ ! -n "$param_value" ]; then
            line=$(grep -n $x $OUTPUT_CONFIG | cut -d: -f1)
            echo "param = null, delete $x line:$line"
            if [ -n "$line" ]; then
                sed -i "$(echo $line)d" $OUTPUT_CONFIG
            fi
        elif [[ ! "$param_value" =~ ^0x.* ]]; then
            echo -e "$x = $param_value parameter error, please check"
            exit -1
        fi
    done
}

DRAM_LAYOUT=$PROJ_ROOT/configs/dram_layout.txt
touch $DRAM_LAYOUT
dram_base_addr=0x20000000
INITRAMFSLOADADDR=0x21800000

dram_size=$(sed -n "/^DRAM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
lx_size=$(sed -n "/^LX_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
mma_size=$(sed -n "/^MMA_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
cma_size=$(sed -n "/^CMA_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
logo_size=$(sed -n "/^LOGO_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
smf_size=$(sed -n "/^SMF_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
riscv_size=$(sed -n "/^RISCV_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
riu_recorder_size=$(sed -n "/^RIU_RECORDER_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
vmm_size=$(sed -n "/^VMM_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
optee_size=$(sed -n "/^OPTEE_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
tfa_size=$(sed -n "/^TFA_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
fb_size=$(sed -n "/^FB_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')

if [ $dram_size == "0" ]; then
    echo "set dram_size is $dram_size!"
    exit -1
fi

if [ $(($dram_size % 0x100000)) != "0" ]; then
    echo "set dram_size is $dram_size not 1M alignmen!"
    exit -1
fi

if [ $(($lx_size % 0x100000)) != "0" ]; then
    echo "set lx_size is $lx_size not 1M alignmen!"
    exit -1
fi

dram_end_addr=$(printf 0x%x $(echo $[ $dram_base_addr + $dram_size ]))
echo "DRAM:$dram_base_addr-$dram_end_addr" > $DRAM_LAYOUT
echo "MODULE:                        Start:                      End:                        size:" >> $DRAM_LAYOUT
lx_addr=$dram_base_addr
lx_end_addr=$(printf 0x%x $(echo $[ $dram_base_addr + $lx_size ]))

if [ $lx_size != "0" ]; then
    echo "LX                                   $lx_addr            $lx_end_addr             $lx_size" >> $DRAM_LAYOUT
fi

update_out_config KERNEL_LX_MEM "LX_MEM=$lx_size"

cma_addr=$(printf 0x%x $(echo $[ $lx_size - $cma_size ]))
cma_end_addr=$lx_size

if [ $cma_size != "0" ]; then
    update_out_config KERNEL_RESERVED_CMA "cma=$(($cma_size / 0x100000))M"
fi

logo_addr=$(printf 0x%x $(echo $[ $cma_addr - $logo_size ]))
logo_end_addr=$cma_addr

if [ $logo_size != "0" ]; then
    update_out_config KERNEL_RESERVED_LOGO "mmap_reserved=fb,miu=0,sz=$logo_size,max_start_off=$logo_addr,max_end_off=$logo_end_addr"
fi

smf_addr=$(printf 0x%x $(echo $[ $logo_addr - $smf_size ]))
smf_end_addr=$logo_addr

if [ $smf_size != "0" ]; then
    update_out_config KERNEL_RESERVED_SMF "mmap_reserved=smf,miu=0,sz=$smf_size,max_start_off=$smf_addr,max_end_off=$smf_end_addr"
fi

riscv_addr=$(printf 0x%x $(echo $[ $smf_addr - $riscv_size ]))
riscv_end_addr=$smf_addr

if [ $riscv_size != "0" ]; then
    update_out_config KERNEL_RESERVED_RISCV "mmap_reserved=riscv,miu=0,sz=$riscv_size,max_start_off=$riscv_addr,max_end_off=$riscv_end_addr"
fi

fb_addr=$(printf 0x%x $(echo $[ $riscv_addr - $fb_size ]))
fb_end_addr=$riscv_addr

if [ $fb_size != "0" ]; then
    update_out_config KERNEL_RESERVED_FB "mma_heap=mma_heap_fb,miu=0,sz=$fb_size"
fi

rtos_enable=$(sed -n "/^RTOS_ENABLE\b/p" $OUTPUT_CONFIG | awk '{print $3}')
if [[ "$rtos_enable" == "on" ]]; then
    rtos_size=$(sed -n "/^RTOS_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
    rtos_ramdisk_size=$(sed -n "/^RTOS_RAMDISK_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
    rtos_timestamp_size=$(sed -n "/^RTOS_TIMESTAMP_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')

    rtos_defconfig=$(sed -n "/^RTOS_CONFIG\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
    RTOS_DEFCONFIG=$PROJ_ROOT/../rtos/proj/mak/defconfigs/$rtos_defconfig

    rtos_dynamic_load_en=$(sed -n "/^CONFIG_DYNAMIC_LOADING_ADDR_SUPPORT\b/p" $RTOS_DEFCONFIG | awk -F'=' '{print $2}')

    mma_addr=$lx_end_addr

    if [ $mma_size == 0 ]; then
        mma_end_addr=$(printf 0x%x $(echo $[ $dram_base_addr + $dram_size - $tfa_size - $optee_size - $vmm_size - $rtos_size ]))
        mma_size=$(printf 0x%x $(echo $[ $mma_end_addr - $mma_addr ]))
        echo "MMA                              $mma_addr            $mma_end_addr             $mma_size" >> $DRAM_LAYOUT
    else
        mma_end_addr=$(printf 0x%x $(echo $[ $mma_addr + $mma_size ]))
        echo "MMA                              $mma_addr            $mma_end_addr             $mma_size" >> $DRAM_LAYOUT
    fi

    rtos_addr=$mma_end_addr
    rtos_end_addr=$(printf 0x%x $(echo $[ $rtos_addr + $rtos_size ]))
    rtos_load_addr=$(printf 0x%x $(echo $[ $rtos_addr + 0x8000 ]))
    rtos_ramdisk_addr=$(printf 0x%x $(echo $[ $rtos_end_addr - $rtos_ramdisk_size - 0x1000]))
    rtos_timestamp_addr=$(printf 0x%x $(echo $[ $rtos_ramdisk_addr - $rtos_timestamp_size - 0x1000]))
    rtos_heap=$(printf 0x%x $(echo $[ $rtos_ramdisk_size + $rtos_timestamp_size + 0x400000 ]))

    dram_size_behind_rtos=$(printf 0x%x $(echo $[ $dram_base_addr + $dram_size - $rtos_end_addr ]))
    rtos_premap_cache_area_size=$(printf 0x%x $(echo $[ $dram_size - $dram_size_behind_rtos - $rtos_size ]))
    rtos_premap_noncache_area_size=$(printf 0x%x $(echo $[ $dram_size - $dram_size_behind_rtos - $rtos_size ]))

    if [[ $rtos_size -lt $rtos_heap ]]; then
        echo "set rtos_size is $rtos_size not enough, rtos_ramdisk_size will reserve $rtos_ramdisk_size!"
        exit -1
    fi

    if [ $rtos_size != "0" ]; then
        if [ $(($rtos_addr % 0x4000)) != "0" ]; then
            echo "set rtos_addr is $rtos_addr not 16K alignmen!"
            exit -1
        fi
        if [ $(($rtos_timestamp_size % 0x1000)) != "0" ]; then
            echo "set rtos_timestamp_size is $rtos_timestamp_size not 4K alignmen!"
            exit -1
        fi
        if [ $(($rtos_ramdisk_size % 0x1000)) != "0" ]; then
            echo "set rtos_ramdisk_size is $rtos_ramdisk_size not 4K alignmen!"
            exit -1
        fi
        echo "RTOS                              $rtos_addr            $rtos_end_addr              $rtos_size" >> $DRAM_LAYOUT
        update_out_config CONFIG_MMA_HEAP_ADDR "$mma_addr"
        update_out_config CONFIG_MMA_HEAP_SIZE "$mma_size"
        update_out_config RTOS_LOAD_ADDR "$rtos_load_addr"
        update_out_config RTOS_BOOT_ENV "rtos_size=$rtos_size limit_dram_size=$dram_size no_access_size=$dram_size_behind_rtos"
        update_out_config RTOS_RAMDISK_LOAD_ADDR "$rtos_ramdisk_addr"
        update_out_config RTOS_RAMDISK_MEMORY_SIZE "$rtos_ramdisk_size"
        update_out_config RTOS_TIMESTAMP_RSVD_ADDR "$rtos_timestamp_addr"
        update_out_config RTOS_TIMESTAMP_SIZE "$rtos_timestamp_size"
        update_out_config RTOS_NO_ACCESS_MEM_SIZE "$dram_size_behind_rtos"
        if [ "$rtos_dynamic_load_en" = "y" ]; then
            sed -i 's/CONFIG_RTOS_MEM_START_VA=.*/CONFIG_RTOS_MEM_START_VA='"0xf0000000"'/g'  $RTOS_DEFCONFIG
        else
            sed -i 's/CONFIG_RTOS_MEM_START_VA=.*/CONFIG_RTOS_MEM_START_VA='"${rtos_addr}"'/g'  $RTOS_DEFCONFIG
        fi
        sed -i 's/CONFIG_RTOS_MEM_START_PA=.*/CONFIG_RTOS_MEM_START_PA='"${rtos_addr}"'/g'  $RTOS_DEFCONFIG
        sed -i 's/CONFIG_RTOS_MEMORY_SIZE=.*/CONFIG_RTOS_MEMORY_SIZE='"${rtos_size}"'/g'  $RTOS_DEFCONFIG
        sed -i 's/CONFIG_LIMIT_DRAM_SIZE=.*/CONFIG_LIMIT_DRAM_SIZE='"${dram_size}"'/g'  $RTOS_DEFCONFIG
        sed -i 's/CONFIG_PREMAP_CACHE_SIZE=.*/CONFIG_PREMAP_CACHE_SIZE='"${rtos_premap_cache_area_size}"'/g'  $RTOS_DEFCONFIG
        sed -i 's/CONFIG_PREMAP_NONCACHE_SIZE=.*/CONFIG_PREMAP_NONCACHE_SIZE='"${rtos_premap_noncache_area_size}"'/g'  $RTOS_DEFCONFIG
        earlyinit_size=$(sed -n "/^EARLYINIT_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
        earlyinit_addr=$(printf 0x%x $(echo $[ $INITRAMFSLOADADDR - $earlyinit_size ]))
        sed -i 's/CONFIG_EARLYINIT_FW_LOAD_ADDR=.*/CONFIG_EARLYINIT_FW_LOAD_ADDR='"${earlyinit_addr}"'/g'  $RTOS_DEFCONFIG
        sed -i 's/CONFIG_EARLYINIT_FW_MEMORY_SIZE=.*/CONFIG_EARLYINIT_FW_MEMORY_SIZE='"${earlyinit_size}"'/g'  $RTOS_DEFCONFIG
   fi

    vmm_addr=$rtos_end_addr

else

    vmm_addr=$lx_end_addr
    lx_heap=$(printf 0x%x $(echo $[ $mma_size + $cma_size + $logo_size + $smf_size + $riscv_size + $fb_size ]))

    if [[ $lx_size -lt $lx_heap ]]; then
        echo "set lx_size is $lx_size not enough, mma_size will reserve $mma_size!"
        exit -1
    fi
    if [ $mma_size != "0" ]; then
        update_out_config KERNEL_RESERVED_MMA "mma_heap=mma_heap_name0,miu=0,sz=$mma_size mma_memblock_remove=1"
    fi

    mma_addr=$(printf 0x%x $(echo $[ $lx_end_addr - $lx_heap ]))

fi

uboot_defconfig=$(sed -n "/^UBOOT_CONFIG\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
if [[ "$uboot_defconfig" =~ "alkaid_defconfig" ]];then
    uboot_defconfig=$(sed -n "/^UBOOT_ORIGIN_CONFIG\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
fi
uboot_curconfig=$uboot_defconfig.alkaid_defconfig
UBOOT_DEFCONFIG=$PROJ_ROOT/../boot/configs/$uboot_curconfig
cp $PROJ_ROOT/../boot/configs/$uboot_defconfig $UBOOT_DEFCONFIG

#get the max value of uboot code size
uboot_size=$(sed -n "/^UBOOT_MEM_SIZE\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')

uboot_ramsize=$(sed -n "/^CONFIG_SSTAR_RAM_SIZE\b/p" $UBOOT_DEFCONFIG | awk -F'=' '{print $2}')
if [ "$uboot_ramsize" ] && [ $(($lx_size)) -gt $(($uboot_ramsize)) ]; then
    echo "Uboot mem is safe ($lx_size > $uboot_ramsize), keep current config."
else
    uboot_ramsize_new=$(printf 0x%x $(echo $[ $mma_addr + $mma_size - $dram_base_addr ]))
    # In dram size little board: the lx_mem maybe less then uboot ram,
    # then in dualos scense + uboot load rtos maybe cause uboot code text section overwrite by rtos
    # so we try use lx_mem+mma memory for uboot reloate to mma end -> hope rtos dont wirte the uboot code v_v
    echo "Change uboot ramsize (${uboot_ramsize}) => (${uboot_ramsize_new})"
    update_uboot_config CONFIG_SSTAR_RAM_SIZE "${uboot_ramsize_new}"
fi

uboot_addr=$(sed -n "/^CONFIG_SYS_TEXT_BASE\b/p" $UBOOT_DEFCONFIG | awk -F'=' '{print $2}')
uboot_end_addr=$(printf 0x%x $(echo $[ $uboot_addr + $uboot_size ]))

update_out_config UBOOT_ORIGIN_CONFIG "$uboot_defconfig"
update_out_config UBOOT_CONFIG "$uboot_curconfig"
eval "sed -i '/MODULE:/a UBOOT                            ${uboot_addr}            ${uboot_end_addr}             ${uboot_size}' $DRAM_LAYOUT"

if [ $riu_recorder_size != "0" ]; then
    if [ $(($riu_recorder_size % 0x1000)) != "0" ]; then
        echo "set riu_recorder_size is $riu_recorder_size not 4K alignmen!"
        exit -1
    fi
    kernel_riu_record=riu_record=$riu_recorder_size
fi

vmm_end_addr=$(printf 0x%x $(echo $[ $vmm_addr + $vmm_size ]))
vmm_load_addr=$(printf 0x%x $(echo $[ $vmm_addr + 0x8000 ]))
vmm_no_access_size=$(printf 0x%x $(echo $[ $optee_size + $tfa_size ]))

if [ $vmm_size != "0" ]; then
    if [ $(($vmm_addr % 0x4000)) != "0" ]; then
        echo "set vmm_addr is $vmm_addr not 16K alignmen!"
        exit -1
    fi
    echo "VMM                              $vmm_addr            $vmm_end_addr              $vmm_size" >> $DRAM_LAYOUT
    update_out_config VMM_LOAD_ADDR "$vmm_load_addr"
    update_out_config VMM_LIMIT_MEM_SIZE "$dram_size"
    update_out_config VMM_NO_ACCESS_MEM_SIZE "$vmm_no_access_size"
fi

optee_addr=$vmm_end_addr
optee_end_addr=$(printf 0x%x $(echo $[ $optee_addr + $optee_size ]))

if [ $optee_size != "0" ]; then
    if [ $(($optee_size % 0x1000)) != "0" ]; then
        echo "set optee_size is $optee_size not 4K alignmen!"
        exit -1
    fi
    echo "OPTEE                             $optee_addr            $optee_end_addr              $optee_size" >> $DRAM_LAYOUT
    OPTEE_DEFCONFIG=$PROJ_ROOT/../optee/optee_os/core/arch/arm/plat-$CHIP/conf.mk
    sed -i 's/CFG_TZDRAM_START ?= .*/CFG_TZDRAM_START ?= '"${optee_addr}"'/g'  $OPTEE_DEFCONFIG
    sed -i 's/CFG_TZDRAM_SIZE ?= .*/CFG_TZDRAM_SIZE ?= '"${optee_size}"'/g'  $OPTEE_DEFCONFIG
fi

tfa_addr=$optee_end_addr
tfa_end_addr=$(printf 0x%x $(echo $[ $tfa_addr + $tfa_size ]))

if [ $tfa_size != "0" ]; then
    if [ $(($tfa_size % 0x1000)) != "0" ]; then
        echo "set tfa_size is $tfa_size not 4K alignmen!"
        exit -1
    fi
    echo "TFA                                 $tfa_addr             $tfa_end_addr             $tfa_size" >> $DRAM_LAYOUT
fi

if [ $tfa_end_addr != $dram_end_addr ]; then
    echo "dram layout err tfa_end_addr $tfa_end_addr != dram_end_addr $dram_end_addr !"
    exit -1
fi

kernel_lx_mem=$(sed -n "/^KERNEL_LX_MEM\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
kernel_reserved_mma=$(sed -n "/^KERNEL_RESERVED_MMA\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
kernel_reserved_cma=$(sed -n "/^KERNEL_RESERVED_CMA\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
kernel_reserved_logo=$(sed -n "/^KERNEL_RESERVED_LOGO\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
kernel_reserved_smf=$(sed -n "/^KERNEL_RESERVED_SMF\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
kernel_reserved_riscv=$(sed -n "/^KERNEL_RESERVED_RISCV\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
kernel_reserved_fb=$(sed -n "/^KERNEL_RESERVED_FB\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')

if [ -n "$kernel_reserved_smf" ]; then
    if [ "$kernel_reserved_env" == "mmap_reserved=" ]; then
        kernel_reserved_env="$kernel_reserved_smf "
    else
        kernel_reserved_env+="$kernel_reserved_smf "
    fi
fi

if [ -n "$kernel_reserved_riscv" ]; then
    if [ "$kernel_reserved_env" == "mmap_reserved=" ]; then
        kernel_reserved_env="$kernel_reserved_riscv "
    else
        kernel_reserved_env+="$kernel_reserved_riscv "
    fi
fi

if [ -n "$kernel_reserved_logo" ]; then
    if [ "$kernel_reserved_env" == "mmap_reserved=" ]; then
        kernel_reserved_env="$kernel_reserved_logo"
    else
        kernel_reserved_env+="$kernel_reserved_logo"
    fi
fi

update_out_config KERNEL_BOOT_ENV "$kernel_lx_mem $kernel_reserved_mma $kernel_reserved_cma  $kernel_riu_record $kernel_reserved_fb"

dual_os=$(sed -n "/^DUAL_OS\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
if [ "${dual_os}" == "on" ]; then
    update_out_config KERNEL_ONLY_LX_MEM "LX_MEM=$dram_size"
    update_out_config KERNEL_ONLY_RESERVED_MMA "mma_heap=mma_heap_name0,miu=0,sz=$mma_size mma_memblock_remove=1"
    kernel_only_lx_mem=$(sed -n "/^KERNEL_ONLY_LX_MEM\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
    kernel_only_reserved_mma=$(sed -n "/^KERNEL_ONLY_RESERVED_MMA\b/p" $OUTPUT_CONFIG | awk -F' = ' '{print $2}')
    update_out_config KERNEL_ONLY_BOOT_ENV "$kernel_only_lx_mem $kernel_only_reserved_mma $kernel_reserved_cma  $kernel_riu_record"
fi

if [ "$kernel_reserved_env" != "mmap_reserved=" ]; then
    sed -i "s/KERNEL_BOOT_ENV.*/& \$(KERNEL_RESERVED_ENV)/g" $OUTPUT_CONFIG
    update_out_config KERNEL_RESERVED_ENV "$kernel_reserved_env"
fi

check_param_empty_and_delete DOWNLOADADDR KERNELBOOTADDR INITRAMFSLOADADDR SPLIT_EACH_FILE_SIZE

cat $OUTPUT_CONFIG
