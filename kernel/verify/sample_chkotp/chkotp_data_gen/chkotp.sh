#!/bin/bash

conda activate py3.7.3
python chkotp.py I6D_OTP_MappingList_20240102.xlsx
cp chkotp_data.c ..
cp chkotp.h ../include
conda deactivate

