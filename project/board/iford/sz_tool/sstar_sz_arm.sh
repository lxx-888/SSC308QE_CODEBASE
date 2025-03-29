#!/bin/sh

XZ_DEFAULTS=--memlimit-compress=30MiB
export XZ_DEFAULTS

srctree=$(dirname "$0")

while getopts "d:b:" opt; do
  case $opt in
    d)
      DATA_FILE=$OPTARG
      ;;
    b)
      COMP_BLOCK_CNT=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

#echo DATA_FILE=$DATA_FILE
#echo COMP_BLOCK_CNT=$COMP_BLOCK_CNT

# Warning : If we want to compress file whitch is larger than 1024MiB, please modify file_size_max to fit your requestment.
file_size_max=8MiB

cp ${DATA_FILE} ${DATA_FILE}_tmp

input_file=${DATA_FILE}_tmp
output_file=${DATA_FILE}.sz

sz_enc_one_block=$($srctree/sz -z -f -k --check=crc32 --lzma2=dict=4KiB --block-list=$file_size_max --block-size=$file_size_max --threads=4 ${input_file})
if [ $? != 0 ]
then
    echo "sz comp fail !!!"
    exit 1
fi

if [[ "$COMP_BLOCK_CNT" -eq "1" ]]
then
    cp ${input_file}.xz ${output_file}
    rm -rf ${input_file}*
    exit 0
fi

block_split_size_list=$($srctree/szdec ${input_file}.xz ${input_file} --block-size=$file_size_max split_block=$COMP_BLOCK_CNT)
if [ $? != 0 ]
then
    echo "szdec faill !!!"
    exit 1
fi

sz_enc_multi_block=$($srctree/sz -z -f -k --check=crc32 --lzma2=dict=4KiB --block-list=$block_split_size_list$file_size_max --block-size=$file_size_max --threads=4 ${input_file})
if [ $? != 0 ]
then
    echo "sz comp fail !!!"
    exit 1
fi

sstar_xz_file=$($srctree/szsplit ${input_file}.xz ${input_file}.sz --block-size=$file_size_max)
if [ $? != 0 ]
then
    echo "szsplit faill !!!"
    exit 1
fi

cp ${input_file}.sz ${output_file}

rm -rf ${input_file}*

exit 0
