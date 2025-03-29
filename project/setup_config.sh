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

if [[ ${INPUT_CONFIG:0-9} == "defconfig" ]]; then
    $PROJ_ROOT/setup_defconfig.sh $INPUT_CONFIG
    exit
fi

OUTPUT_CONFIG=$PROJ_ROOT/configs/current.configs
if [ $# -ge 2 ]; then
    OUTPUT_CONFIG=$2
fi
ALKAID_MHAL_UT=0
if [ $# -ge 3 ]; then
    ALKAID_MHAL_UT=$3
fi
echo ======================================
echo ALKAID_MHAL_UT=$ALKAID_MHAL_UT
echo ======================================

CLANG_PREBUILT_BIN=kernel/prebuilts-master/clang/host/linux-x86/clang-r416183b/bin
LINUX_GCC_CROSS_COMPILE_PREBUILTS_BIN=kernel/prebuilts/gas/linux-x86
export PATH=$PATH:/tools/toolchain/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin

if [ -e configs ]; then

    TOOLCHAIN=$(sed -n "/^TOOLCHAIN\b/p"  $1 | awk '{print $3}')
    if [ "$TOOLCHAIN" != "llvm" ]; then
        export PATH=/tools/bin/:$PATH
    else
        export PATH=/tools/bin:$PROJ_ROOT/../$CLANG_PREBUILT_BIN:$PROJ_ROOT/../$LINUX_GCC_CROSS_COMPILE_PREBUILTS_BIN:$PATH
    fi

    setup_chip=$(sed -n "/^CHIP\b/p"  $1 | awk '{print $3}')
    setup_toolchain_v=$(sed -n "/^TOOLCHAIN_VERSION\b/p"  $1 | awk '{print $3}')
    setup_cross_compile=$(sed -n "/^TOOLCHAIN_REL\b/p"  $1 | awk '{print $3}')
    if [ "$TOOLCHAIN" != "llvm" ]; then
        cur_toolchain_v=$(${setup_cross_compile}-gcc -dumpversion)
    else
        cur_toolchain_v=$(clang -dumpversion)
    fi

    linux_arch=$(sed -n "/LINUX_ARCH\b/p" $1 | awk '{print $3}')

    if [ -f /tools/bin/clang-13/clang-tidy ]; then
        clang_tidy=/tools/bin/clang-13/clang-tidy
        clang_format=/tools/bin/clang-13/clang-format
    else
        clang_tidy=$PROJ_ROOT/tools/clang-13/clang-tidy
        clang_format=$PROJ_ROOT/tools/clang-13/clang-format
    fi
    setup_clang_version=13.0.0
    current_clang_format_version=$($clang_format --version | grep 'version' | sed -r "s/.*version ([0-9]+\.[0-9]+.*).*/\1/g")
    current_clang_tidy_version=$($clang_tidy --version | grep 'version' | sed -r "s/.*version ([0-9]+\.[0-9]+.*).*/\1/g")

    if [ -f "$clang_format" ]; then

        version_check=$(echo ${current_clang_format_version} | grep ${setup_clang_version})
        if [[ "$version_check" != "" ]];then
            echo "current clang-format version:$current_clang_format_version"
        else
            echo "*********************************************************************"
            echo -e "\033[5;41;33m" "clang_format VERSION IS NOT MATCHED FOR  [ $setup_clang_version ]!" "\033[0m"
            echo "config request clang-format version is:[ $setup_clang_version ], but server clang-format version is:[ $current_clang_format_version ]."
            echo "*********************************************************************"
            exit
        fi
    else
        echo "$clang_format not exists."
        clang_format=''
    fi

    if [ -f "$clang_tidy" ]; then

        version_check=$(echo ${current_clang_tidy_version} | grep ${setup_clang_version})
        if [[ "$version_check" != "" ]];then
            echo "current clang-tidy version:$current_clang_tidy_version"
        else
            echo "*********************************************************************"
            echo -e "\033[5;41;33m" "clang_tidy VERSION IS NOT MATCHED FOR  [ $setup_clang_version ]!" "\033[0m"
            echo "config request clang-tidy version is:[ $setup_clang_version ], but server clang-tidy version is:[ $current_clang_tidy_version ]."
            echo "*********************************************************************"
            exit
        fi
    else
        echo "$clang_tidy not exists."
        clang_tidy=''
    fi

    if [ "$setup_toolchain_v" != "$cur_toolchain_v" ]; then
        echo "*********************************************************************"
        echo -e "\033[5;41;33m" "TOOLCHAIN VERSION IS NOT MATCHED FOR CHIP [ $setup_chip ]!" "\033[0m"
        echo "config request toolchain version is:[ $setup_toolchain_v ], but server toolchain version is:[ $cur_toolchain_v ]."
        echo "*********************************************************************"
        exit -1
    fi

    echo PROJ_ROOT = $PROJ_ROOT > $OUTPUT_CONFIG
    echo CONFIG_NAME = config_module_list.mk >> $OUTPUT_CONFIG
    echo KBUILD_MK = kbuild/kbuild.mk >> $OUTPUT_CONFIG
    echo SOURCE_MK = ../sdk/sdk.mk >> $OUTPUT_CONFIG
    echo UBOOT_MK = ./board/uboot/uboot.mk >> $OUTPUT_CONFIG
    echo RTOS_MK = ./board/rtos/rtos.mk >> $OUTPUT_CONFIG
    echo PM_RTOS_MK = ./board/pm_rtos/pm_rtos.mk >> $OUTPUT_CONFIG
    echo OPTEE_MK = ./board/optee/optee.mk >> $OUTPUT_CONFIG
    echo "KERNEL_MEMADR = \$(shell $PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/\$(CHIP)/mmap/\$(MMAP) \$(CHIP) E_LX_MEM phyaddr)" >> $OUTPUT_CONFIG
    echo "KERNEL_MEMLEN = \$(shell $PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/\$(CHIP)/mmap/\$(MMAP) \$(CHIP) E_LX_MEM size)" >> $OUTPUT_CONFIG
    echo "KERNEL_MEMADR2 = \$(shell $PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/\$(CHIP)/mmap/\$(MMAP) \$(CHIP) E_LX_MEM2 phyaddr)" >> $OUTPUT_CONFIG
    echo "KERNEL_MEMLEN2 = \$(shell $PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/\$(CHIP)/mmap/\$(MMAP) \$(CHIP) E_LX_MEM2 size)" >> $OUTPUT_CONFIG
    echo "KERNEL_MEMADR3 = \$(shell $PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/\$(CHIP)/mmap/\$(MMAP) \$(CHIP) E_LX_MEM3 phyaddr)" >> $OUTPUT_CONFIG
    echo "KERNEL_MEMLEN3 = \$(shell $PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/\$(CHIP)/mmap/\$(MMAP) \$(CHIP) E_LX_MEM3 size)" >> $OUTPUT_CONFIG
    echo "LOGO_ADDR = \$(shell $PROJ_ROOT/image/makefiletools/bin/mmapparser $PROJ_ROOT/board/\$(CHIP)/mmap/\$(MMAP) \$(CHIP) \$(BOOTLOGO_ADDR) miuaddr)" >> $OUTPUT_CONFIG
    cat $1 >> $OUTPUT_CONFIG
    if [ "$linux_arch" != "" ]; then
        echo "ARCH=$linux_arch" >> $OUTPUT_CONFIG
    else
        echo "ARCH=arm" >> $OUTPUT_CONFIG
    fi
    echo "CROSS_COMPILE=$(sed -n "/TOOLCHAIN_REL\b/p" $OUTPUT_CONFIG | awk '{print $3}')-" >> $OUTPUT_CONFIG
    echo "PYTHON=python3" >> $OUTPUT_CONFIG

    c=$(sed -n "/^CHIP\b/p" $OUTPUT_CONFIG | awk '{print $3}')
    echo "CHIP_FULL_NAME = $c" >> $OUTPUT_CONFIG
    echo "CHIP_ALIAS = $c" >> $OUTPUT_CONFIG

    echo "PREFIX =\$(TOOLCHAIN_REL)-" >> $OUTPUT_CONFIG

    if [ "$TOOLCHAIN" != "llvm" ]; then
        echo "AS = \$(PREFIX)as" >> $OUTPUT_CONFIG
        echo "CC = \$(PREFIX)gcc" >> $OUTPUT_CONFIG
        echo "CXX = \$(PREFIX)g++" >> $OUTPUT_CONFIG
        echo "CPP = \$(PREFIX)cpp" >> $OUTPUT_CONFIG
        echo "LD = \$(PREFIX)ld" >> $OUTPUT_CONFIG
        echo "AR = \$(PREFIX)ar" >> $OUTPUT_CONFIG
        echo "STRIP = \$(PREFIX)strip" >> $OUTPUT_CONFIG
        echo "OBJCOPY = \$(PREFIX)objcopy" >> $OUTPUT_CONFIG
        echo "OBJDUMP = \$(PREFIX)objdump" >> $OUTPUT_CONFIG
    else
        ndk_clang_prefix=$(sed -n "/^NDK_CLANG_PREFIX\b/p" $OUTPUT_CONFIG | awk '{print $3}')
        ndk_binutils_prefix=$(sed -n "/^NDK_BINUTILS_PREFIX\b/p" $OUTPUT_CONFIG | awk '{print $3}')
        ndk_api_version=$(sed -n "/^NDK_API_VERSION\b/p" $OUTPUT_CONFIG | awk '{print $3}')
        ndk_clang_version=$(sed -n "/^NDK_CLANG_VERSION\b/p" $OUTPUT_CONFIG | awk '{print $3}')

        cur_ndk_clang_version=$(${ndk_clang_prefix}${ndk_api_version}-clang -dumpversion)

        if [ "${cur_ndk_clang_version}" != "${ndk_clang_version}" ];then
            echo "*********************************************************************"
            echo -e "\033[5;41;33m" "ndk clang version is not matched !!!" "\033[0m"
            echo "config request ndk clang version is:[ $ndk_clang_version ], but server ndk clang version is:[ $cur_ndk_clang_version ]."
            echo "*********************************************************************"
            exit -1
        fi

        ndk_path=$(dirname $(which ${ndk_clang_prefix}${ndk_api_version}-clang))

        echo "CC = ${ndk_path}/${ndk_clang_prefix}${ndk_api_version}-clang" >> $OUTPUT_CONFIG
        echo "CXX = ${ndk_path}/${ndk_clang_prefix}${ndk_api_version}-clang++" >> $OUTPUT_CONFIG
        echo "CPP = ${ndk_path}/${ndk_clang_prefix}${ndk_api_version}-clang++" >> $OUTPUT_CONFIG
        echo "AS = ${ndk_path}/${ndk_binutils_prefix}-as" >> $OUTPUT_CONFIG
        echo "LD = ${ndk_path}/${ndk_binutils_prefix}-ld" >> $OUTPUT_CONFIG
        echo "AR = ${ndk_path}/${ndk_binutils_prefix}-ar" >> $OUTPUT_CONFIG
        echo "NM = ${ndk_path}/${ndk_binutils_prefix}-nm" >> $OUTPUT_CONFIG
        echo "STRIP = ${ndk_path}/${ndk_binutils_prefix}-strip" >> $OUTPUT_CONFIG
        echo "OBJCOPY = ${ndk_path}/${ndk_binutils_prefix}-objcopy" >> $OUTPUT_CONFIG
        echo "OBJDUMP = ${ndk_path}/${ndk_binutils_prefix}-objdump" >> $OUTPUT_CONFIG

        echo "export KBUILD_CONFIGS = LLVM=1 LLVM_IAS=1 CC=clang CPP=\"clang -E\" LD=ld.lld AR=llvm-ar NM=llvm-nm OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump READELF=llvm-readelf STRIP=llvm-strip" >> $OUTPUT_CONFIG
    fi

    echo "export ARCH CROSS_COMPILE" >> $OUTPUT_CONFIG
    echo "export PATH=$PATH" >> $OUTPUT_CONFIG

    if [[ "$clang_tidy" != '' && "$clang_format" != '' ]]; then
        echo "CLANG_TIDY = $clang_tidy" >> $OUTPUT_CONFIG
        echo "CLANG_FORMAT = $clang_format" >> $OUTPUT_CONFIG
        echo "export CLANG_TIDY CLANG_FORMAT" >> $OUTPUT_CONFIG
    fi

else
    echo "can't found configs directory!"
    exit -1
fi

p=$(sed -n "/^PRODUCT\b/p" $OUTPUT_CONFIG | awk '{print $3}')
if [[ "$p" == "android" ]]; then
    KERNEL_ROOT=$PROJ_ROOT/../kernel/common
else
    KERNEL_ROOT=$PROJ_ROOT/../kernel
fi
echo "KERNEL_ROOT = $KERNEL_ROOT" >> $OUTPUT_CONFIG

if [[ ! "$p" =~ "purertos" ]];then
    if [ -e "../kernel/" ];then
        KERNEL_MK="$KERNEL_ROOT/Makefile"
        VERSION=`sed -n "/^VERSION/p" $KERNEL_MK | tr -cd "[0-9]"`
        PATCHLEVEL=`sed -n "/^PATCHLEVEL/p" $KERNEL_MK | tr -cd "[0-9]"`
        SUBLEVEL=`sed -n "/^SUBLEVEL/p" $KERNEL_MK | tr -cd "[0-9]"`
        CUR_KERNEL_VERSION="$VERSION.$PATCHLEVEL.$SUBLEVEL"
        echo "CUR_KERNEL_VERSION = $CUR_KERNEL_VERSION" >> $OUTPUT_CONFIG
    else
        echo "can't find kernel directory!"
        exit -1
    fi
fi

#The next line(ALKAID_MHAL_UT) is the key flag of alkaid release script detect. Please dont delete
echo "ALKAID_MHAL_UT = $ALKAID_MHAL_UT" >> $OUTPUT_CONFIG

