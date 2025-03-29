#!/usr/bin/python
#
# SigmaStar trade secret.
# Copyright (c) [2019~2020] SigmaStar Technology.
# All rights reserved.
#
# Unless otherwise stipulated in writing, any and all information contained
# herein regardless in any format shall remain the sole proprietary of
# SigmaStar and be kept in strict confidence
# (SigmaStar Confidential Information) by the recipient.
# Any unauthorized act including without limitation unauthorized disclosure,
# copying, use, reproduction, sale, distribution, modification, disassembling,
# reverse engineering and compiling of the contents of SigmaStar Confidential
# Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
# rights to any and all damages, losses, costs and expenses resulting therefrom.
#

import re, fnmatch, os, sys, struct

def cal_checksum4b_file(f, length):
    count =0
    sum = 0
    while (count != length):
        bytes = f.read(4)
        if bytes == "":
            break;
        data = struct.unpack("I", bytes)
        #print('0x%08X' % (data))
        sum += data[0]
        count+=4
    #print('count:0x%08X' % (count))
    return sum&0xffffffff

def calc_crc16_modbus_file(f, length):
    j =0
    count =0
    reg_crc = 0xffff
    while(length):
        bytes = f.read(1)
        data = struct.unpack("B", bytes)
        reg_crc ^= data[0]
        for j in range(8):
            if(reg_crc  &  0x01):
                reg_crc=(reg_crc>>1)  ^  0xA001
            else:
                reg_crc=reg_crc >>1
        count+=1
        length-=1
    #print('count:0x%08X' % (count))
    return reg_crc


def main():
    ipl_size = os.path.getsize(sys.argv[1])
    f = open(sys.argv[1], 'rb')
    out = open(sys.argv[2], 'wb')
    out.write(f.read())

#OFFSET_00: Header size by branch offset 4N+8
    f.seek(0x0,0)
    bytes = f.read(1)
    data = struct.unpack("B", bytes)
    hdr_size=4*data[0]+8
    print('%18s=0x%02x' % ('HDR_Size', hdr_size))

#OFFSET_0E: Header_Version 1bytes
    f.seek(0xE,0)
    bytes = f.read(1)
    data = struct.unpack("B", bytes)
    hdr_version =data[0]
    print('%18s=0x%02x' % ('HDR_ver.', hdr_version))

#OFFSET_04: Magic 4bytes
    f.seek(0x4,0)
    if f.read(3)!='IPL':
        print('ERROR! NO IPL HEADER!')
        f.close()
        out.close()
        exit()
    print('%18s=IPL%c' % ('magic',  f.read(1)))

#OFFSET_08: Size 2bytes
    size_08 = ipl_size
    out.seek(8, 0)
    out.write('%c' % (size_08 & 0xFF))
    out.write('%c' % (size_08>>8 & 0xFF))
    print('%18s=0x%04X(%d)' % ('SIZE',size_08, size_08))

#OFFSET_0A: CID 1bytes
    print('%18s=0x%s' % ('CID',sys.argv[3]))
    out.write('%c' % int(sys.argv[3], 16))

#OFFSET_0B: AUTH 1bytes
    if(sys.argv[4]=='1'):
        AUTH = 0xFA
    else:
        AUTH = 0x00
    out.write('%c' % AUTH)
    print('%18s=0x%X' % ('AUTH',AUTH))

#OFFSET_0C: Checksum|CRC16 2bytes
    check_sum=0
    size_without_version = ipl_size-16
    if (size_without_version%4) != 0:
        print('ERROR! file size no 4-bytes alignment!')
        f.close()
        out.close()
        exit()
    f.seek(16,0)
    check_sum = calc_crc16_modbus_file(f, size_without_version)
    #check_sum = cal_checksum4b_file(f, size_without_version)

    print('%18s=0x%04X' % ('CHKSUM',check_sum ))
    out.write('%c' % (check_sum& 0xff))
    out.write('%c' % (check_sum>>8& 0xff))

#OFFSET_0F: Anti-rollback
    out.seek(0xf,0)
    if (len(sys.argv)>5 and (hdr_version>=4)):
        print('%18s=0x%s' % ('Anti-rollback',sys.argv[5]))
        out.write('%c' % int(sys.argv[5], 16))


    f.close()
    out.close()

if __name__ == '__main__':
    main()
