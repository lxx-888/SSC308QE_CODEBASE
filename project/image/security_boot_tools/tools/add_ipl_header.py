#!/usr/bin/python

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

def calc_crc16_modbus_file(file, length):
    crc = 0xFFFF
    while(length):
        bytes = file.read(1)
        data = struct.unpack("B", bytes)
        crc ^= data[0]
        for _ in range(0, 8):
            if(crc & 0x01):
                crc = (crc >> 1) ^ 0xA001
            else:
                crc = crc >> 1
        length -= 1
    return crc

def get_IPL_realy_size(hdr_ver, file_size):
    if hdr_ver >= 5:
        return file_size<<4
    else:
        return file_size

def main():
    f = open(sys.argv[1], 'rb')
    out = open(sys.argv[2], 'wb+')
    plainText = f.read()
    out.write(plainText)

    _, fileSize, _, _, _, HeaderVersion, _,_,_  = struct.unpack('IH2BH2B2H', plainText[4:20])
    realy_size = get_IPL_realy_size(HeaderVersion, fileSize)
    if(realy_size == 0):
        ipl_size = os.path.getsize(sys.argv[1])
    else:
        ipl_size = realy_size

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
    if f.read(3) != b'IPL':
        print('ERROR! NO IPL HEADER!')
        f.close()
        out.close()
        exit()
    bytes = f.read(1)
    magic = struct.unpack("b", bytes)
    print('%18s=IPL%c' % ('magic', magic[0]))

#OFFSET_08: Size 2bytes
    size_08 = ipl_size
    out.seek(8, 0)
    if hdr_version >= 0x05:
        out.write(b'%c' % (size_08>>4 & 0xFF))
        out.write(b'%c' % (size_08>>12 & 0xFF))
    else:
        out.write(b'%c' % (size_08 & 0xFF))
        out.write(b'%c' % (size_08>>8 & 0xFF))
    print('%18s=0x%04X(%d)' % ('SIZE',size_08, size_08))

#OFFSET_0A: CID 1bytes
    out.seek(10, 0)
    print('%18s=0x%s' % ('CID',sys.argv[3]))
    # write 0 to CID is invalid
    if(int(sys.argv[3], 16) != 0):
        out.write(b'%c' % int(sys.argv[3], 16))

#OFFSET_0B: AUTH 1bytes
    out.seek(11, 0)
    if(sys.argv[4]=='1'):
        AUTH = 0xFA # Debug mode
    elif(sys.argv[4]=='2'):
        AUTH = 0x01 # Fault Injection protect mode
    elif(sys.argv[4]=='3'):
        AUTH = 0xFB # Debug mode + Fault Injection protect mode
    elif(sys.argv[4]=='4'):
        AUTH = 0x2 # RSA sig len is 256 byte. for normal boot used sig image
    elif(sys.argv[4]=='5'):
        AUTH = 0x4 # RSA sig len is 512 byte. for normal boot used sig image
    else:
        AUTH = 0x00
    out.write(b'%c' % AUTH)
    print('%18s=0x%X' % ('AUTH',AUTH))

#OFFSET_13: len keyN and keyAES
    out.seek(18,0)
    if (len(sys.argv)>6):
        if(sys.argv[6]=='1'):
            print('%18s=0x%X' % ('AES', int(sys.argv[6], 16)))
            out.write(b'%c' % int(sys.argv[6], 16))
            out.flush()

#OFFSET_0C: Checksum|CRC16 2bytes
    check_sum=0
    out.seek(12,0)
    size_without_version = ipl_size-16
    if (size_without_version%4) != 0:
        print('ERROR! file size no 4-bytes alignment!')
        f.close()
        out.close()
        exit()
    out.seek(16,0)
    check_sum = calc_crc16_modbus_file(out, size_without_version)
    #check_sum = cal_checksum4b_file(f, size_without_version)

    out.seek(12,0)
    print('%18s=0x%04X' % ('CHKSUM',check_sum ))
    out.write(b'%c' % (check_sum& 0xff))
    out.write(b'%c' % (check_sum>>8& 0xff))

#OFFSET_0F: Anti-rollback
    out.seek(0xf,0)
    if (len(sys.argv)>5):
        print('%18s=0x%s' % ('Anti-rollback',sys.argv[5]))
        out.write(b'%c' % int(sys.argv[5], 16))

    f.close()
    out.close()

if __name__ == '__main__':
    main()
