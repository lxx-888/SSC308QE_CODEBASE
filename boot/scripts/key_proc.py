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

import os, sys, getopt, struct
from Crypto.Cipher import AES
from Crypto.Hash import SHA256
from Crypto.Signature import PKCS1_v1_5
from Crypto.Signature import PKCS1_PSS
from Crypto.PublicKey import RSA
from Crypto.Util.number import long_to_bytes

image_headers = {'IPL': 0x5F4C5049, 'IPL_CUST': 0x434C5049, 'IPL_CUSTK': 0x4e4C5049, 'uImage': 0x27051956, 'RTK': 0x5F4B5452}
otp_cmds = ['wriu -w 0x101806','wait 10','wriu -w 0x101800 0x0006','wait 10','wriu -w 0x101808','wait 10','wriu -w 0x10180A 0x00FF','wait 10','wriu -w 0x101802 0x8000','wait 10','wriu -w 0x101802 0x8001','wait 10','wriu -w 0x101802 0x8000','wait 10','']
rsa_eKeys = [0x00,0x01,0x00,0x01]

def printRed(mess):
    print '\033[1;31;40m%s\033[0m ' % mess

def usage():
    print 'Usage: key_proc.py [--encrypt|--insert|--sign|--burn|--signusb] [--aes=cipher] [--rsa=public_key] [-f bin_file]'
    print 'When encryption, [--encrypt] [--aes=cipher] [-f bin_file].'
    print 'When key insertion, [--insert] [--aes=cipher] [--rsa=public_key] [-f bin_file]'
    print 'When generate signature, [--sign] [--rsa=private_key] [-f bin_file]'
    print 'When split IPLX.bin, [--split] [-f bin_file]'
    print 'key to bin for inserting key and generating digest, [--keyN] [-f key]'
    sys.exit(0)

def version():
    print 'Version S1.2\r\n'
    print 'Version Info: '
    print 'S1.0: Applied to the OTP chip, and match to ALL IPL Share code'
    print 'S1.1: Support ALL chip split IPLX'
    print 'S1.2: IPL for usb boot supports encryption and signature'
    sys.exit(0)

def indentify_header(message):
    hash = message

    if 0 < len(message):
        for name, header in image_headers.items():
            if header == struct.unpack('I', message[4:8])[0]:
                print '%s' % name
                if 'IPL_CUSTK' == name:
                    lenN, lenAES = struct.unpack('2H', message[16:20])
                    if 0 == lenN and 0 == lenAES:
                        printRed('Need to insert key first. break')
                        return
            elif header == struct.unpack('>I', message[0:4])[0]:
                print '%s' % name
                if 0x4D362323 == struct.unpack('>I', message[38:42])[0]:
                    hash = message[64:]
                elif 0x49364523 == struct.unpack('>I', message[38:42])[0]:
                    hash = message[64:]
                elif 0x50332323 == struct.unpack('>I', message[38:42])[0]:
                    hash = message[64:]
                else:
                    hash = message[0:]

    return hash

def do_keyN(bin):
    with open(bin, 'rb') as private:
        privateKey = RSA.importKey(private.read())
    with open(os.path.abspath(bin)[:os.path.abspath(bin).rindex('.')] + '.bin', 'wb') as key:
        key.write(long_to_bytes(privateKey.n))

def do_sign(key, bin):
    message = None
    privateKey = None
    hash = None
    try:
        with open(os.path.abspath(bin), 'rb') as msg:
            message = msg.read()
            hash = indentify_header(message)

        with open(key, 'rb') as private:
            try:
                privateKey = RSA.importKey(private.read())
            except:
                keyType = ['n', 'e', 'd', 'p', 'q']
                private.seek(0, 0)
                for c in range(len(keyType)):
                    keyType[c] = long(private.readline().split('=')[1].strip(), 16)
                #print keydata[c]
                privateKey = RSA.construct(tuple(keyType))

        if privateKey is not None and hash is not None:
            sigFileName = os.path.abspath(bin)
            if '.' in os.path.basename(sigFileName):
                pssFile = sigFileName[:sigFileName.rindex('.')] + '.sigpss' + sigFileName[sigFileName.rindex('.'):]
                sigFileName = sigFileName[:sigFileName.rindex('.')] + '.sig' + sigFileName[sigFileName.rindex('.'):]
            else:
                pssFile = sigFileName + '.sigpss'
                sigFileName = sigFileName + '.sig'

            with open(sigFileName, 'wb') as sig:
                sig.write(message + PKCS1_v1_5.new(privateKey).sign(SHA256.new(hash)))
            print "PKCS1_v1_5: %s" % sigFileName
            with open(pssFile, 'wb') as sig:
                sig.write(message + PKCS1_PSS.new(privateKey).sign(SHA256.new(hash)))
            print "PKCS1_PSS: %s" % pssFile

            #modulusFile = os.path.abspath(sys.argv[1])[:os.path.abspath(sys.argv[1]).rindex('/')] + '/modulus.bin'
            #print 'modulus: %s' % modulusFile
            #if False == os.path.exists(modulusFile):
            #    with open(modulusFile, 'wb') as modulus:
            #        modulus.write(long_to_bytes(privateKey.n))
    except BaseException as e:
        print str(e)
        pass

def do_sign_usb(keyfile, binfile):
    plainText = None
    sigText = None
    try:
        if binfile is not None:
            with open(binfile, "rb") as src_file:
                plainText = src_file.read()
                ipl_magic, ipl_fileSize = struct.unpack('IH', plainText[4:10])
                if 0x5f4C5049 == ipl_magic:
                    ipl_bin = plainText[:ipl_fileSize]
                    ipl_file = open(binfile+"_cut.bin", "wb")
                    ipl_file.write(ipl_bin)
                    ipl_file.close()
                    do_sign(keyfile, binfile+"_cut.bin")

                    sigFileName = os.path.abspath(binfile+"_cut.bin")
                    sigpssFileName = sigFileName
                    sigusbFileName = os.path.abspath(binfile+"_append.bin")
                    sigpssusbFileName = sigusbFileName
                    if '.' in os.path.basename(sigFileName):
                        sigFileName = sigFileName[:sigFileName.rindex('.')] + '.sig' + sigFileName[sigFileName.rindex('.'):]
                        sigpssFileName = sigpssFileName[:sigpssFileName.rindex('.')] + '.sigpss' + sigpssFileName[sigpssFileName.rindex('.'):]
                        sigusbFileName = sigusbFileName[:sigusbFileName.rindex('.')] + '.sig' + sigusbFileName[sigusbFileName.rindex('.'):]
                        sigpssusbFileName = sigpssusbFileName[:sigpssusbFileName.rindex('.')] + '.sigpss' + sigpssusbFileName[sigpssusbFileName.rindex('.'):]
                    else:
                        sigFileName = sigFileName + '.sig'
                        sigpssFileName = sigpssFileName + '.sigpss'
                        sigusbFileName = sigusbFileName + '.sig'
                        sigpssusbFileName = sigpssusbFileName + '.sigpss'

                    with open(sigFileName, "rb") as sig:
                        sigText = sig.read()

                    with open(sigpssFileName, "rb") as sigpss:
                        sigpssText = sigpss.read()

                    with open(sigusbFileName, 'wb') as sigusb:
                        sigusb.write(sigText + plainText[ipl_fileSize+256:])
                        while((len(plainText)) < (64*1024)):
                            sigusb.write('\0')
                            ipl_fileSize+=1

                    with open(sigpssusbFileName, 'wb') as sigpssusb:
                        sigpssusb.write(sigpssText + plainText[ipl_fileSize+256:])

                    print 'Sign IPL for usb done >>>'
                else:
                    printRed('Sign IPL for usb ERR: IPL magic checkout fail!')
    except:
        printRed('ERR: open file fail!')
        pass

def do_encrypt(cipherfile,binfile):
    cipher = None
    plaintext = None
    encryptor = None
    IVtext = None
    IV = None
    try:
        with open(cipherfile, 'rb') as key:
            cipher = key.read()
        with open(binfile, 'rb') as text:
            plaintext = text.read()
            filesize = len(plaintext)
        if 0 < len(cipher) and  0 < len(plaintext):
            align = len(plaintext) % 16
            if 0 < align:
                print 'total size: %d -> ' % len(plaintext),
                align = 16 - align
                for c in range(align):
                    plaintext += struct.pack('b', 0)
                print '%d' % len(plaintext)
            #print 'AES len %d' % len(cipher)
            if '.' in os.path.basename(binfile):
                binfile = binfile[:binfile.rindex('.')] + '.aes' + binfile[binfile.rindex('.'):]
            else:
                binfile += '.aes'
            print 'out: %s' % binfile
            with open(binfile, 'wb') as out:
                if 0x27051956 == struct.unpack('>I', plaintext[0:4])[0]:
                    headersize  = filesize - (struct.unpack('>I', plaintext[12:16])[0])
                    encryptor = AES.new(cipher, AES.MODE_ECB)
                    print 'uImage header size:%d' % (headersize)
                    ciphertext = encryptor.encrypt(plaintext[headersize:])
                    ciphertext = plaintext[0:headersize] + ciphertext
                elif 0x5F4B5452== struct.unpack('I', plaintext[32:36])[0]:
                    print 'RTK header'
                    encryptor = AES.new(cipher, AES.MODE_ECB)
                    ciphertext = encryptor.encrypt(plaintext[48:])
                    ciphertext = plaintext[0:48] + ciphertext
                elif 0x49504C5F == struct.unpack('>I', plaintext[4:8])[0]:
                    IV =  plaintext[16:32]
                    IVtext = IV[::-1]
                    encryptor = AES.new(cipher, AES.MODE_CBC, IVtext)
                    ciphertext = encryptor.encrypt(plaintext[32:])
                    ciphertext = plaintext[0:32] +ciphertext
                else:
                    printRed('Not a standard binary. continue >>>')
                    encryptor = AES.new(cipher, AES.MODE_ECB)
                    ciphertext = encryptor.encrypt(plaintext)
                out.write(ciphertext)
    except BaseException as e:
        print str(e)
        pass

def calc_checksum(plaintext, size):
    checksum = 0
    c = 0
    while size != c:
        checksum += struct.unpack('I', plaintext[c:c+4])[0]
        c += 4
    checksum &= 0xffff
    return checksum

def calc_checksum_key(key, size):
    checksum = 0
    c = 0
    while size != c:
        checksum += struct.unpack('I', key[c:c+4])[0]
        c += 4
    checksum &= 0xffffffff
    return checksum

def calc_crc16_checksum(plaintext, length):
    j =0
    cnt = 0
    reg_crc = 0xffff
    checksum = 0
    while length != cnt:
        checksum = struct.unpack('B', plaintext[cnt:cnt+1])[0]
        reg_crc ^= checksum
        for j in range(8):
            if(reg_crc  &  0x01):
                reg_crc=(reg_crc>>1)  ^  0xA001
            else:
                reg_crc=reg_crc >>1
        cnt+=1
    return reg_crc

def calc_checksum_sel(plaintext, length,crc_flag):
    if 0 == crc_flag:
        return calc_checksum(plaintext,length)
    else:
        return calc_crc16_checksum(plaintext,length)

def insert_key(binfile, pubkey, cipher, IVfile):
    plainText = None
    IV = None
    result = None

    try:
        with open(binfile, 'rb') as bin:
            plainText = bin.read()
            #magic, fileSize, cid, auth, checksum, fileSize, lenN, lenAES = struct.unpack('IH2B4H', plainText[4:20])
            magic, fileSize, cid, auth, checksum, HeaderVersion, ANTI,lenN,lenAES  = struct.unpack('IH2BH2B2H', plainText[4:20])

        if calc_checksum_sel(plainText[16:], len(plainText[16:fileSize]),0) == checksum :
            crcflag=0
            print 'checksum'
        elif calc_checksum_sel(plainText[16:], len(plainText[16:fileSize]), 1) == checksum :
            print 'CRC16'
            crcflag=1
        else:
            crcflag=2
            print 'header(%x) ' % checksum
            print 'checksum(%x)' % calc_checksum_sel(plainText[16:], len(plainText[16:]), 0)
            print 'CRC16(%x)' % calc_checksum_sel(plainText[16:], len(plainText[16:]), 1)

        print '** Start insertion **'
        print ' fileSize = %08x' % fileSize
        print ' checksum = %08x' % checksum
        if 0x4e4C5049 == magic:
            print ' lenN = %d' % lenN
            print ' lenAES = %d' % lenAES
            if crcflag == 2 :
                printRed('file crc check error. pls check>>>>')
                return

            if pubkey is not None:
                if os.path.isfile(pubkey):
                    with open(pubkey, "rb") as key:
                        lenN = key.read()
                        try:
                            rsa = RSA.importKey(lenN)
                            lenN = long_to_bytes(rsa.n)
                        except:
                            pass
                        plainText += lenN
                        lenN = len(lenN)
            if cipher is not None:
                if os.path.isfile(cipher):
                    with open(cipher, "rb") as aes:
                        lenAES = aes.read()
                        plainText += lenAES
                        lenAES = len(lenAES)
            fileSize += lenN
            print '** End insertion **'
            print ' fileSize = %08x' % fileSize
            plainText = plainText[0:8] + struct.pack('H2BH2B2H', fileSize, cid, auth, checksum, HeaderVersion, ANTI, lenN, lenAES) + plainText[20:]
            checksum = calc_checksum_sel(plainText[16:], len(plainText[16:fileSize]),crcflag)
            print ' checksum = %08x' % checksum
            plainText = plainText[:12] + struct.pack('H', checksum) + plainText[14:]
            print ' lenN = %d' % lenN
            print ' lenAES = %d' % lenAES
        elif 0x5f4C5049 == magic:
            if crcflag == 2 :
                printRed('file crc check error. pls check >>>')
                return
            if IVfile is not None:
                with open(IVfile, 'rb') as IV_V:
                    IV = IV_V.read()
                    result = IV[::-1]

            if pubkey is not None:
                if os.path.isfile(pubkey):
                    with open(pubkey, "rb") as key:
                        lenN = key.read()
                        try:
                            rsa = RSA.importKey(lenN)
                            lenN = long_to_bytes(rsa.n)
                        except:
                            pass
                        print 'fileSize:%d' % (fileSize)
                        tail_text = (struct.unpack('>I', plainText[fileSize-4:fileSize])[0])
                        if 0x5f4C5049 == tail_text:
                            tail = fileSize-0x10
                            plainText = plainText[0:tail] + lenN + plainText[tail:]
                        else:
                            plainText += lenN
                        lenN = len(lenN)
            fileSize+=lenN
            print '** End insertion **'
            print ' fileSize = %08x' % fileSize
            if IVfile is not None:
                plainText = plainText[0:8] + struct.pack('H2BH2B', fileSize, cid, auth, checksum, HeaderVersion,ANTI) + result[0:16] + plainText[32:]
            else:
                plainText = plainText[0:8] + struct.pack('H2BH2B', fileSize, cid, auth, checksum, HeaderVersion,ANTI) + plainText[16:]
            checksum = calc_checksum_sel(plainText[16:], len(plainText[16:fileSize]),crcflag)
            print ' checksum = %08x' % checksum
            plainText = plainText[:12] + struct.pack('H', checksum) + plainText[14:]
            print ' lenN = %d' % lenN
        else:
            printRed("Not a IPL_CUSTK image. continue >>>")
            if pubkey is not None:
                if os.path.isfile(pubkey):
                    with open(pubkey, "rb") as key:
                        lenN = key.read()
                        try:
                            rsa = RSA.importKey(lenN)
                            lenN = long_to_bytes(rsa.n)
                        except:
                            pass
                        plainText += lenN
                        lenN = len(lenN)
        if '.' in binfile:
            binfile = binfile[:binfile.rindex('.')] + '.cipher' + binfile[binfile.rindex('.'):]
        else:
            binfile += '.cipher'
        print binfile
        with open(binfile, 'wb') as out:
            out.write(plainText)
    except:
        pass

def insert_key_for_burn(binfile, pubkey):
    plainText = None
    KeyMagic = '\0\0\0\0\0\0\0\0\0\0\0\0'
    try:
        with open(binfile, 'rb') as bin:
            plainText = bin.read()
        magic, fileSize, cid, auth, checksum, lenN, lenAES = struct.unpack('IH2BI2H', plainText[4:20])
        if 0x4e4C5049 == magic:
            print '** Start insertion **'
            print ' fileSize = %08x' % fileSize
            print ' checksum = %08x' % checksum

            if pubkey is not None:
                if os.path.isfile(pubkey):
                    with open(pubkey, "rb") as key:
                        lenN = key.read()
                        try:
                            rsa = RSA.importKey(lenN)
                            lenN = long_to_bytes(rsa.n)
                        except:
                            pass

                        checksumKey = calc_checksum_key(lenN, len(lenN))
                        print ' checksumKey = %08x' % checksumKey
                        plainText += KeyMagic
                        plainText += struct.pack('I', checksumKey)
                        plainText += lenN
                        lenN = len(lenN)

            fileSize = len(plainText)
            print '** End insertion **'
            print ' fileSize = %08x' % fileSize
            plainText = plainText[0:8] + struct.pack('H2BI', fileSize, cid, auth, checksum) + plainText[16:]
            checksum = calc_checksum(plainText[16:], len(plainText[16:]))
            checksum = (checksum + checksumKey) & 0xffff;
            print ' checksum = %08x' % checksum
            plainText = plainText[:12] + struct.pack('I', checksum) + plainText[16:]
            print ' lenN = %d' % lenN
            if '.' in binfile:
                binfile = binfile[:binfile.rindex('.')] + '.burn' + binfile[binfile.rindex('.'):]
            else:
                binfile += '.burn'
            print binfile
            with open(binfile, 'wb') as out:
                out.write(plainText)
    except:
        pass

def export_rsakey(pubkey):
    plainText = None
    try:
            if pubkey is not None:
                if os.path.isfile(pubkey):
                    with open(pubkey, "rb") as key:
                        lenN = key.read()
                        try:
                            rsa = RSA.importKey(lenN)
                            lenN = long_to_bytes(rsa.n)
                        except:
                            pass
                        plainText = lenN
                        lenN = len(lenN)

            fileSize = len(plainText)
            print ' fileSize = %08x' % fileSize
            print ' lenN = %d' % lenN

            binfile = 'rsaKey.bin'
            print binfile
            with open(binfile, 'wb') as out:
                out.write(plainText)

            digest = SHA256.new(plainText).digest()
            digestbin = 'rsaKeyDigest.bin'
            print digestbin
            with open(digestbin, 'wb') as out:
                out.write(digest)

            uboot_scripts = 'rsaKey.txt'
            with open(uboot_scripts, 'w') as f1:
                f1.write('//####### RSA N-Key ########\r\n')
                for j in xrange(0,lenN,64):
                    for i in xrange(j,j+64,4):
                        
                        if(lenN == 512):
                            off = 0x200-4-i;
                            cmd = 'otpctrl -w' + ' ' + '0x0' + ' ' + str(hex(i)) +' ' + str("0x{:08x}".format(struct.unpack('>I',plainText[off:off+4])[0])).rstrip("L") + ';'
                        else:
                            off=i;
                            cmd = 'otpctrl -w' + ' ' + '0x0' + ' ' + str(hex(i)) +' ' + str("0x{:08x}".format(struct.unpack('I',plainText[off:off+4])[0])).rstrip("L") + ';'
                        f1.write(cmd)
                    f1.write('\r\n')
                f1.write('//####### RSA E-Key ########\r\n')
                
                if(lenN == 512):
                    f1.write('otpctrl -w 0x1 0x0 0x00010001\r\n' )
                else:
                    f1.write('otpctrl -w 0x1 0x0 0x01000100\r\n' )

            uboot_scripts = 'rsaKeyDigest.txt'
            with open(uboot_scripts, 'w') as f1:
                f1.write('//####### RSA Key Digest########\r\n')
                for j in xrange(0,32,8):
                    for i in xrange(j,j+8,4):
                        off=i;
                        cmd = 'otpctrl -w' + ' ' + '0xe' + ' ' + str(hex(i)) +' ' + str("0x{:08x}".format(struct.unpack('I',digest[off:off+4])[0])).rstrip("L") + ';'
                        f1.write(cmd)
                    f1.write('\r\n')
    except:
        pass

def split_file_func(split_file):
    plainText = None
    try:
        if split_file is not None:
            with open(split_file, "rb") as src_file:
                plainText = src_file.read()
                ipl_magic, ipl_fileSize = struct.unpack('IH', plainText[4:10])
                if 0x5f4C5049 == ipl_magic:
                    ipl_bin = plainText[:ipl_fileSize]
                    ipl_file = open("./IPL.bin", "wb")
                    ipl_file.write(ipl_bin)
                    ipl_file.close()
                    print 'Split IPLX to IPL done >>>'
                else:
                    printRed('split_file_func ERR: IPL magic checkout fail!')

                cust_magic, cust_fileSize = struct.unpack('IH', plainText[ipl_fileSize+256+4:ipl_fileSize+256+10])
                if 0x4E4C5049 == cust_magic:
                    cust_bin = plainText[ipl_fileSize+256:ipl_fileSize+256+cust_fileSize]
                    cust_file = open("./IPL_CUST.bin", "wb")
                    cust_file.write(cust_bin)
                    cust_file.close()
                    print 'Split IPLX to IPL_CUST done >>>'
                else:
                    cust_magic, cust_fileSize = struct.unpack('IH', plainText[ipl_fileSize+512+4:ipl_fileSize+512+10])
                    if 0x4E4C5049 == cust_magic:
                        cust_bin = plainText[ipl_fileSize+512:ipl_fileSize+512+cust_fileSize]
                        cust_file = open("./IPL_CUST.bin", "wb")
                        cust_file.write(cust_bin)
                        cust_file.close()
                        print 'Split IPLX to IPL_CUST done >>>'
                    else:
                        printRed('split_file_func ERR: IPL_CUST magic checkout fail!')
    except:
        printRed('ERR: open file fail!')
        pass

def main():
    binfile = None
    cipherfile = None
    keyfile = None
    encrypt = False
    insert = False
    burn = False
    sign = False
    exportkey = False
    splitflag = False
    signusb = False
    IVfile = None

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'vhf:',['version', 'IV=', 'aes=', 'rsa=', 'help', 'encrypt', 'insert', 'burn', 'sign', 'exportkey', 'split', 'signusb', 'keyN', ])
        for o, a in opts:
            if o == "-f":
                binfile = os.path.abspath(a)
                print 'image: %s' % binfile
            elif o == '-h' or o == '--help':
                usage()
            elif o == '-v' or o == '--version':
                version()
            elif o == '--IV':
                IVfile = os.path.abspath(a)
                print 'IV: %s' % IVfile
            elif o == '--encrypt':
                encrypt = True
            elif o == '--insert':
                insert = True
            elif o == '--burn':
                burn = True
            elif o == '--sign':
                sign = True
            elif o == '--exportkey':
                exportkey = True
            elif o == '--aes':
                cipherfile = os.path.abspath(a)
                print 'aes: %s' % cipherfile
            elif o == "--rsa":
                keyfile = os.path.abspath(a)
                print 'rsa: %s' % keyfile
            elif o == "--split":
                splitflag = True
            elif o == "--signusb":
                signusb = True
            elif o == '--keyN':
                keyN = True
            else:
                assert False, "unhandled option %s" % o

        if 3 > len(sys.argv):
            usage()

        if True == encrypt:
            do_encrypt(cipherfile, binfile)
        elif True == insert:
            insert_key(binfile, keyfile, cipherfile,IVfile)
        elif True == burn:
            insert_key_for_burn(binfile, keyfile)
        elif True == sign:
            do_sign(keyfile, binfile)
        elif True == exportkey:
            export_rsakey(keyfile)
        elif True == splitflag:
            split_file_func(binfile)
        elif True == signusb:
            do_sign_usb(keyfile, binfile)
        elif True == keyN:
            do_keyN(binfile)
        else:
            usage()
    except getopt.GetoptError as err:
        # print help information and exit:
        print(err) # will print something like "option -a not recognized"
        usage()

if __name__ == '__main__':
    main()
