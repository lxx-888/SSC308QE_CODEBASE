#!/bin/sh

echo "######control in/out count=1024, vary=1 max_size=1024#####"
./testusb -a -t14 -v1 -c1024 -s1024


echo "#####iso out count=3072, vary=1, max_size=3072#####"
./testusb -a -t15  -c3072  -s3072  -g1 -v1

echo "#####iso in count=3072, vary=1, max_size=3072#####"
./testusb -a -t16  -c3072  -s3072  -g1 -v1

echo "#####bulk out count=30720, vary=1, max_size=30720#####"
./testusb -a -t3  -c30720  -s30720  -g1 -v1

echo "#####bulk in count=30720, vary=1, max_size=30720#####"
./testusb -a -t4  -c30720  -s30720  -g1 -v1


echo "#####interrupt out count=64, vary=1, max_size=64#####"
./testusb -a -t25  -c64  -s64 -v1


echo "#####interrupt in count=64, vary=1, max_size=64#####"
./testusb -a -t26  -c64  -s64 -v1