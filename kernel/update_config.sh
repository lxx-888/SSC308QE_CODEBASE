#!/bin/sh

myname=${0##*/}
logname=".${myname}.log"

usage() {
    cat >&2 <<EOL
Update options of defconfig through compile two .config
Usage:
$myname command ...
commands:
    --new-config|-n option        specify new .config modified manually
    --old-config|-o option        specify old .config not modified never
    --defconfig|-d option         specify defconfig to update
    --defconfig-list|-l option    specify defconfig list file(list defconfig file path line by line)

    new-config option and old-config option must be specified
    defconfig option or defconfig-list option must be specified
EOL
    exit 1
}

update_one_defconfig() {
    if [[ ! $3 =~ "arch/" ]]
    then
        return
    fi

    arch=${3#*arch/}
    arch=${arch%%/*}
    export ARCH=$arch

    make $(basename $3) >> $logname
    config_diff=$(scripts/diffconfig $2 $1)

    IFS=$'\n'

    for diff in $config_diff
    do
        if [ "${diff:0:1}" == "-" ]
        then
            option=$(echo ${diff:1} | awk '{print $1}')
            scripts/config --undefine $option
            printf "%-10s %-48s %-48s\n" "[undef]" $3 $option >> $logname
        elif [ "${diff:0:1}" == "+" ]
        then
            option=$(echo ${diff:1} | awk '{print $1}')
            result=$(echo ${diff:1} | awk '{print $2}')
            if [ "${result:0:1}" == "\"" ]
            then
                scripts/config --set-str $option $result
                printf "%-10s %-48s %-48s %-32s\n" "[add-s]" $3 $option $result >> $logname
            else
                scripts/config --set-val $option $result
                printf "%-10s %-48s %-48s %-32s\n" "[add-v]" $3 $option $result >> $logname
            fi
        elif [ "${diff:0:1}" == " " ]
        then
            option=$(echo ${diff:1} | awk '{print $1}')
            origin=$(echo ${diff:1} | awk '{print $2}')
            result=$(echo ${diff:1} | awk '{print $4}')
            if [ "${origin:0:1}" == "\"" ]
            then
                scripts/config --set-str $option $result
                printf "%-10s %-48s %-48s %-32s %-32s\n" "[mod-s]" $3 $option $origin $result >> $logname
            else
                scripts/config --set-val $option $result
                printf "%-10s %-48s %-48s %-32s %-32s\n" "[mod-v]" $3 $option $origin $result >> $logname
            fi
        fi
    done

    sh -e save_config.sh >> $logname
}

ARGS=`getopt -o o:n:d:l: --long old-config:,new-config:,defconfig:,defconfig-list: -n "$0" -- "$@"`
if [ $? != 0 ]; then
    echo "getopt error exit..."
    exit 1
fi

eval set -- "${ARGS}"

while true
do
    case "$1" in
        -o|--old-config)
            old_config=$2
            shift 2
            ;;

        -n|--new-config)
            new_config=$2
            shift 2
            ;;

        -d|--defconfig)
            defconfig=$2
            shift 2
            ;;

        -l|--defconfig-list)
            defconfig_list=$2
            shift 2
            ;;

        --)
            shift
            break
            ;;

        *)
            echo "getopt internal error exit..."
            exit 1
            ;;
    esac
done

if [ "$new_config" == "" ] || [ "$old_config" == "" ]
then
    usage
fi

if [ ! -e $new_config ]
then
    echo "--new-config not exist exit..."
    exit 1
fi

if [ ! -e $old_config ]
then
    echo "--old-config not exist exit..."
    exit 1
fi

if [ "$defconfig_list" != "" ]
then
    if [ ! -e $defconfig_list ]
    then
        echo "$defconfig_list not exist exit..."
        exit 1
    fi
    if [ -e $logname ]
    then
        rm $logname
    else
        touch $logname
    fi
    count=0
    defconfigs=$(cat $defconfig_list)
    total=$(cat $defconfig_list | egrep -v '^\s*$' | wc -l)
    for one in $defconfigs
    do
        if [ ! -e $one ]
        then
            echo "$one not exist continue..."
            continue
        fi
        update_one_defconfig $new_config $old_config $one
        count=$((count+1))
        printf "\rupdating %-64s [%d]%%" "$one ..." $((count*100/total))
    done
    echo ""
elif [ "$defconfig" != "" ]
then
    if [ ! -e $defconfig ]
    then
        echo "$defconfig not exist exit..."
        exit 1
    fi
    if [ -e $logname ]
    then
        rm $logname
    else
        touch $logname
    fi
    update_one_defconfig $new_config $old_config $defconfig
else
    usage
fi


