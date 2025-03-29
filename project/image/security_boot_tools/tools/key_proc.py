#!/usr/bin/python3
#SigmaStar trade secret
import os, sys, getopt, struct, hashlib
from Crypto.Cipher import AES
from Crypto.Hash import SHA256
from Crypto.Signature import PKCS1_v1_5
from Crypto.Signature import PKCS1_PSS
from Crypto.PublicKey import RSA
from Crypto.Util.number import long_to_bytes
from Crypto.Util import Counter
import binascii

image_headers = {'IPL': 0x5F4C5049, 'IPL_CUST': 0x434C5049, 'IPL_CUSTK': 0x4e4C5049, 'uImage': 0x27051956, 'RTK': 0x5F4B5452}
otp_cmds = ['wriu -w 0x101806','wait 10','wriu -w 0x101800 0x0006','wait 10','wriu -w 0x101808','wait 10','wriu -w 0x10180A 0x00FF','wait 10','wriu -w 0x101802 0x8000','wait 10','wriu -w 0x101802 0x8001','wait 10','wriu -w 0x101802 0x8000','wait 10','']
rsa_eKeys = [0x00,0x01,0x00,0x01]

enable_log=1

def sstar_print_string(args):
    global enable_log
    if enable_log:
        print(args)

def sstar_print_val(args, val):
    global enable_log
    if enable_log:
        print(args, val)


def printRed(mess):
    print("\033[1;31;40m",mess,"\033[0m ")

def usage():
    print("Usage: key_proc.py [--encrypt|--insert|--sign|--burn|--signusb] [--aes=cipher] [--rsa=public_key] [-f bin_file]")
    print("When encryption, [--encrypt] [--aes=cipher] [-f bin_file].")
    print("When key insertion, [--insert] [--aes=cipher] [--rsa=public_key] [-f bin_file]")
    print("When generate signature, [--sign] [--rsa=private_key] [-f bin_file]")
    print("When split IPLX.bin, [--split] [-f bin_file]")
    print("key to bin for inserting key and generating digest, [--keyN] [-f key]")
    sys.exit(0)

def version():
    print("Version S1.4\r\n")
    print("Version Info: ")
    print("S1.0: Applied to the OTP chip, and match to ALL IPL Share code")
    print("S1.1: Support ALL chip split IPLX")
    print("S1.2: IPL for usb boot supports encryption and signature")
    print("S1.3: Add export rsakey command of IC Opear.")
    print("S1.4: Added the judgment of IPL Header version and Key2sha1 func.")
    sys.exit(0)

def get_IPL_realy_size(hdr_ver, file_size):
    if hdr_ver >= 5:
        return file_size<<4
    else:
        return file_size

def SetIPL_Header_size(hdr_ver, file_size):
    if hdr_ver >= 5:
        return file_size>>4
    else:
        return file_size

def indentify_header(message):
    hash = message
    if 0 < len(message):
        for name, header in image_headers.items():
            if header == struct.unpack('I', message[4:8])[0]:
                sstar_print_val("Binary name ",name)
                if 'IPL_CUSTK' == name:
                    lenN, lenAES = struct.unpack('2H', message[16:20])
                    if 0 == lenN and 0 == (lenAES&0xfffE):
                        printRed('Need to insert key first. break')
                        return
                #IPL
                elif 0x5f4C5049 == header:
                    _, fileSize, _, _, _, HeaderVersion, _,_,_  = struct.unpack('IH2BH2B2H', message[4:20])
                    realy_size = get_IPL_realy_size(HeaderVersion, fileSize)
                    if(len(message) > realy_size):
                        hash = message[0:realy_size]
                    else:
                        hash = message[0:]
            elif header == struct.unpack('>I', message[0:4])[0]:
                sstar_print_val("Binary name ",name)
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
    bcbText = None
    realy_size = 0
    try:
        with open(os.path.abspath(bin), 'rb') as msg:
            message = msg.read()
            hash = indentify_header(message)

            #IPL
            if 0x5f4C5049 == struct.unpack('I', message[4:8])[0]:
                _, fileSize, _, _, _, HeaderVersion, _,_,_  = struct.unpack('IH2BH2B2H', message[4:20])
                realy_size = get_IPL_realy_size(HeaderVersion, fileSize)
                if(len(message) > realy_size):
                    bcbText = message[realy_size:]
                    print("has bcbText\r\n")

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
                if(bcbText):
                    sig.write(message[:realy_size] + PKCS1_v1_5.new(privateKey).sign(SHA256.new(hash))+bcbText)
                else:
                    sig.write(message + PKCS1_v1_5.new(privateKey).sign(SHA256.new(hash)))
            sstar_print_val("PKCS1_v1_5 : ",sigFileName)
            with open(pssFile, 'wb') as sig:
                if(bcbText):
                    sig.write(message[:realy_size] + PKCS1_PSS.new(privateKey).sign(SHA256.new(hash))+bcbText)
                else:
                    sig.write(message + PKCS1_PSS.new(privateKey).sign(SHA256.new(hash)))
            sstar_print_val("PKCS1_PSS : ",pssFile)
            #modulusFile = os.path.abspath(sys.argv[1])[:os.path.abspath(sys.argv[1]).rindex('/')] + '/modulus.bin'
            #print 'modulus: %s' % modulusFile
            #if False == os.path.exists(modulusFile):
            #    with open(modulusFile, 'wb') as modulus:
            #        modulus.write(long_to_bytes(privateKey.n))
    except BaseException as e:
        sstar_print_string(str(e))
        pass

def do_sign_usb(keyfile, binfile):
    plainText = None
    sigText = None
    append_len = 0
    try:
        if binfile is not None:
            with open(binfile, "rb") as src_file:
                plainText = src_file.read()
                ipl_magic, ipl_fileSize = struct.unpack('IH', plainText[4:10])
                HDR_Ver = struct.unpack('B', plainText[14:15])
                ipl_fileSize = get_IPL_realy_size(HDR_Ver[0], ipl_fileSize)
                if 0x5f4C5049 == ipl_magic:
                    ipl_bin = plainText[:ipl_fileSize]
                    if '.' in os.path.basename(binfile):
                        cut_binfile_name = binfile[:binfile.rindex('.')] + '.cut' + binfile[binfile.rindex('.'):]
                    else:
                        cut_binfile_name = binfile + '.cut'

                    ipl_file = open(cut_binfile_name, "wb")
                    ipl_file.write(ipl_bin)
                    ipl_file.close()
                    do_sign(keyfile, cut_binfile_name)

                    if '.' in os.path.basename(cut_binfile_name):
                        sig_cut_binfile = cut_binfile_name[:cut_binfile_name.rindex('.')] + '.sig' + cut_binfile_name[cut_binfile_name.rindex('.'):]
                        sigpss_cut_binfile = cut_binfile_name[:cut_binfile_name.rindex('.')] + '.sigpss' + cut_binfile_name[cut_binfile_name.rindex('.'):]
                        sig_append_binfile = cut_binfile_name[:cut_binfile_name.rindex('.')] + '.append.sig' + cut_binfile_name[cut_binfile_name.rindex('.'):]
                        sigpss_append_binfile = cut_binfile_name[:cut_binfile_name.rindex('.')] + '.append.sigpss' + cut_binfile_name[cut_binfile_name.rindex('.'):]
                    else:
                        sig_cut_binfile = cut_binfile_name + '.sig'
                        sigpss_cut_binfile = cut_binfile_name + '.sigpss'
                        sig_append_binfile = cut_binfile_name + '.append.sig'
                        sigpss_append_binfile = cut_binfile_name + '.append.sigpss'

                    with open(sig_cut_binfile, "rb") as sig:
                        sigText = sig.read()

                    with open(sigpss_cut_binfile, "rb") as sigpss:
                        sigpssText = sigpss.read()
                    sstar_print_val("HDR_Ver: ",HDR_Ver[0])
                    if HDR_Ver[0] == 0x05:
                        append_size = 104*1024
                    else:
                        append_size = 64*1024

                    append_len=0
                    with open(sig_append_binfile, "wb") as sigusb:
                        sigusb.write(sigText)
                        while((len(sigText) + append_len) < append_size):
                            sigusb.write(b'\0')
                            append_len=append_len+1

                    append_len=0
                    with open(sigpss_append_binfile, "wb") as sigpssusb:
                        sigpssusb.write(sigpssText)
                        while((len(sigpssText) + append_len) < append_size):
                            sigpssusb.write(b'\0')
                            append_len=append_len+1

                    sstar_print_string("Sign IPL for usb done >>>");
                else:
                    printRed("Sign IPL for usb ERR: IPL magic checkout fail!")
    except:
        printRed("ERR: open file fail!")
        pass

def sha1xor(cipher):
    checksum = 0
    c = 0
    size = 40
    result = hashlib.sha1(cipher).hexdigest()
    sstar_print_val("sha1 : ",result)

    while size != c:
        checksum ^= (int(result[c:c+8],16))
        c += 8
    return checksum

def key2sha1_xor(cipherfile,reverse):
    cipher = None
    checksum = 0
    if cipherfile is not None:
        if os.path.isfile(cipherfile):
            with open(cipherfile, "rb") as key:
                cipher = key.read()
                try:
                    rsa = RSA.importKey(cipher)
                    cipher = long_to_bytes(rsa.n)
                    pubkey_rev = cipher[::-1]
                    sstar_print_string("RSA Pem Key");
                    checksum = sha1xor(pubkey_rev)
                except:
                    sstar_print_string("Binary Key");
                    if reverse == True:
                        pubkey_rev = cipher[::-1]
                        checksum = sha1xor(pubkey_rev)
                    else:
                        checksum = sha1xor(cipher)
                    pass
                sha1_xor_bin = cipherfile + "_sha1.txt"
                sstar_print_val("checksum: ",hex(checksum))
                sstar_print_val("sha1_xor_bin: ",sha1_xor_bin)
                with open(sha1_xor_bin, 'wb') as sha1:
                    sha1.write(hex(checksum).encode())

def do_aes_cipher(infile, outfile, keyfile, IVtext, mode, is_encrypt):
    with open(infile, 'rb') as f:
        plaintext = f.read()
        filesize = len(plaintext)
        if 0 < len(plaintext):
            align = len(plaintext) % 16
            if 0 < align:
                print ('total size: %d -> ' % len(plaintext),)
                align = 16 - align
                for c in range(align):
                    plaintext += struct.pack('b', 0)
                print ('%d' % len(plaintext))

    with open(keyfile, 'rb') as f:
        key = f.read()
        if mode == 'ecb' :
            cipher = AES.new(key, AES.MODE_ECB)
        elif mode == 'cbc' :
            with open(IVtext, 'rb') as text:
                IV = text.read()
                IVtext = IV[::-1]
                cipher = AES.new(key, AES.MODE_CBC, IVtext)
        if is_encrypt == 'decrypt':
            ciphertext = cipher.decrypt(plaintext)
        else:
            ciphertext = cipher.encrypt(plaintext)

    with open(outfile, 'wb') as f:
        f.write(ciphertext)


def do_encrypt(cipherfile, binfile, IVfile, mode):
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
                sstar_print_val("total size: -> ",len(plaintext))
                align = 16 - align
                for c in range(align):
                    plaintext += struct.pack('b', 0)
            #print 'AES len %d' % len(cipher)
            if '.' in os.path.basename(binfile):
                binfile = binfile[:binfile.rindex('.')] + '.aes' + binfile[binfile.rindex('.'):]
            else:
                binfile += '.aes'
            sstar_print_val("out binary: ",binfile)

            sstar_print_val("mode: ",mode)

            if mode == 'cbc' :
                if 0x49504C5F == struct.unpack('>I', plaintext[4:8])[0]:
                    IV =  plaintext[16:32]
                    IV = IV[::-1]
                else:
                    with open(IVfile, 'rb') as text:
                        IV = text.read()
                encryptor = AES.new(cipher, AES.MODE_CBC, IV)
            elif mode == 'ctr' :
                with open(IVfile, 'rb') as text:
                    IV = text.read()

                    iv_int = int(binascii.hexlify(IV), 16)
                    ctr = Counter.new(AES.block_size * 8, initial_value=iv_int)
                    encryptor = AES.new(cipher, mode=AES.MODE_CTR, counter=ctr)

                    #encryptor = AES.new(cipher, AES.MODE_CTR, ctr)
            else:
                encryptor = AES.new(cipher, AES.MODE_ECB)

            with open(binfile, 'wb') as out:
                if 0x27051956 == struct.unpack('>I', plaintext[0:4])[0]:
                    headersize  = filesize - (struct.unpack('>I', plaintext[12:16])[0])
                    sstar_print_val("uImage header size:",headersize);
                    ciphertext = encryptor.encrypt(plaintext[headersize:])
                    ciphertext = plaintext[0:headersize] + ciphertext
                elif 0x5F4B5452== struct.unpack('I', plaintext[32:36])[0]:
                    sstar_print_string("RTK");
                    ciphertext = encryptor.encrypt(plaintext[48:])
                    ciphertext = plaintext[0:48] + ciphertext
                elif 0x5f4b5452 == struct.unpack('I', plaintext[1088:1088+4])[0]:
                    sstar_print_string("PM_RTK");
                    ciphertext = encryptor.encrypt(plaintext[1104:])
                    ciphertext = plaintext[0:1104] + ciphertext
                elif 0x4554504F == struct.unpack('I', plaintext[0:4])[0]:
                    sstar_print_string("OPTEE");
                    ciphertext = encryptor.encrypt(plaintext[32:])
                    ciphertext = plaintext[0:32] + ciphertext
                elif 0x49504C5F == struct.unpack('>I', plaintext[4:8])[0]:
                    ipl_magic, ipl_fileSize = struct.unpack('IH', plaintext[4:10])
                    HDR_Ver = struct.unpack('B', plaintext[14:15])
                    ipl_fileSize = get_IPL_realy_size(HDR_Ver[0],ipl_fileSize)
                    sstar_print_val('ipl_fileSize',ipl_fileSize);
                    ciphertext = encryptor.encrypt(plaintext[32:ipl_fileSize])
                    ciphertext = plaintext[0:32] + ciphertext
                else:
                    printRed('Not a standard binary. continue >>>')
                    sstar_print_val("uImage header : ", struct.unpack('I', plaintext[0:4])[0]);
                    encryptor = AES.new(cipher, AES.MODE_ECB)
                    ciphertext = encryptor.encrypt(plaintext)
                out.write(ciphertext)
    except BaseException as e:
        print(str(e))
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
    realy_size=0

    try:
        with open(binfile, 'rb') as bin:
            plainText = bin.read()
            #magic, fileSize, cid, auth, checksum, fileSize, lenN, lenAES = struct.unpack('IH2B4H', plainText[4:20])
            magic, fileSize, cid, auth, checksum, HeaderVersion, ANTI,lenN,lenAES  = struct.unpack('IH2BH2B2H', plainText[4:20])
        realy_size = get_IPL_realy_size(HeaderVersion, fileSize)
        if calc_checksum_sel(plainText[16:], len(plainText[16:realy_size]),0) == checksum :
            crcflag=0
        elif calc_checksum_sel(plainText[16:], len(plainText[16:realy_size]), 1) == checksum :
            crcflag=1
        else:
            crcflag=2
            sstar_print_val("header =",checksum);
            sstar_print_val("checksum =",calc_checksum_sel(plainText[16:], len(plainText[16:]), 0))
            sstar_print_val("CRC16 =",calc_checksum_sel(plainText[16:], len(plainText[16:]), 1))

        sstar_print_string("** Start insertion **");
        sstar_print_val("  fileSize =",realy_size)
        sstar_print_val("  checksum =",checksum)

        #IPL_CUSTK
        if 0x4e4C5049 == magic:
            sstar_print_val("  lenN =",lenN)
            sstar_print_val("  lenAES =",lenAES)
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
            realy_size += lenN
            sstar_print_string("** End insertion **");
            sstar_print_val("  fileSize =",realy_size)

            fileSize = SetIPL_Header_size(HeaderVersion, realy_size)
            plainText = plainText[0:8] + struct.pack('H2BH2B2H', fileSize, cid, auth, checksum, HeaderVersion, ANTI, lenN, lenAES) + plainText[20:]
            checksum = calc_checksum_sel(plainText[16:], len(plainText[16:realy_size]),crcflag)
            sstar_print_val("  checksum =",checksum)
            plainText = plainText[:12] + struct.pack('H', checksum) + plainText[14:realy_size]
            sstar_print_val("  lenN =",lenN)
            sstar_print_val("  lenAES =",lenAES)
        #IPL
        elif 0x5f4C5049 == magic:
            bcbText = None
            if(len(plainText) > realy_size):
                bcbText = plainText[realy_size:]
                print("has bcbText\r\n")

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
                        sstar_print_val("  fileSize =",realy_size)
                        tail_text = (struct.unpack('>I', plainText[realy_size-4:realy_size])[0])
                        if 0x5f4C5049 == tail_text:
                            tail = realy_size-0x10
                            plainText = plainText[0:tail] + lenN + plainText[tail:]
                        else:
                            plainText += lenN
                        lenN = len(lenN)
            realy_size+=lenN
            sstar_print_string("** End insertion **");
            sstar_print_val("  fileSize =",realy_size)

            fileSize = SetIPL_Header_size(HeaderVersion, realy_size)
            if IVfile is not None:
                plainText = plainText[0:8] + struct.pack('H2BH2B', fileSize, cid, auth, checksum, HeaderVersion,ANTI) + result[0:16] + plainText[32:]
            else:
                plainText = plainText[0:8] + struct.pack('H2BH2B', fileSize, cid, auth, checksum, HeaderVersion,ANTI) + plainText[16:]
            checksum = calc_checksum_sel(plainText[16:], len(plainText[16:realy_size]),crcflag)
            sstar_print_val("  checksum =",checksum)
            plainText = plainText[:12] + struct.pack('H', checksum) + plainText[14:realy_size]
            if(bcbText):
                plainText = plainText + bcbText

            sstar_print_val("  lenN =",lenN)
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
        sstar_print_val("binfile: ",binfile)
        with open(binfile, 'wb') as out:
            out.write(plainText)
    except:
        pass

def insert_key_for_burn(binfile, pubkey):
    plainText = None
    KeyMagic = '\0\0\0\0\0\0\0\0\0\0\0\0'
    realy_size=0
    try:
        with open(binfile, 'rb') as bin:
            plainText = bin.read()
        #magic, fileSize, cid, auth, checksum, lenN, lenAES = struct.unpack('IH2BI2H', plainText[4:20])
        magic, fileSize, cid, auth, checksum, HeaderVersion, ANTI,lenN,lenAES  = struct.unpack('IH2BH2B2H', plainText[4:20])
        realy_size = get_IPL_realy_size(HeaderVersion, fileSize)
        if 0x4e4C5049 == magic:
            sstar_print_string("** Start insertion **");
            sstar_print_val(" fileSize = ",realy_size)
            sstar_print_val("checksum = ",checksum)
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
                        sstar_print_val(" checksumKey = ",checksumKey)
                        plainText += KeyMagic
                        plainText += struct.pack('I', checksumKey)
                        plainText += lenN
                        lenN = len(lenN)

            realy_size = len(plainText)
            sstar_print_string("** End insertion **");
            sstar_print_val(" fileSize = ",realy_size)
            fileSize = get_IPL_realy_size(HeaderVersion, realy_size)
            plainText = plainText[0:8] + struct.pack('H2BI', fileSize, cid, auth, checksum) + plainText[16:]
            checksum = calc_checksum(plainText[16:], len(plainText[16:]))
            checksum = (checksum + checksumKey) & 0xffff;
            sstar_print_val(" checksum = ",checksum)
            plainText = plainText[:12] + struct.pack('I', checksum) + plainText[16:]
            sstar_print_val(" lenN = ",lenN)
            if '.' in binfile:
                binfile = binfile[:binfile.rindex('.')] + '.burn' + binfile[binfile.rindex('.'):]
            else:
                binfile += '.burn'
            sstar_print_val("binfile: ",binfile)
            with open(binfile, 'wb') as out:
                out.write(plainText)
    except:
        pass

def export_rsakey(pubkey, reverse):
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
            sstar_print_val(" fileSize = ",fileSize)
            sstar_print_val(" lenN = ",lenN)

            binfile = 'rsaKey.bin'
            sstar_print_string(binfile);
            with open(binfile, 'wb') as out:
                out.write(plainText)

            #digest = SHA256.new(plainText).digest()
            #digestbin = 'rsaKeyDigest.bin'
            #sstar_print_string(digestbin);
            #with open(digestbin, 'wb') as out:
            #    out.write(digest)

            uboot_scripts = 'rsaKey.txt'
            if (lenN == 512):
                linecnt=32
            else:
                linecnt=64
            with open(uboot_scripts, 'w') as f1:
                f1.write('####### RSA N-Key ########\r\n')
                for j in range(0,lenN,linecnt):
                    for i in range(j,j+linecnt,4):
                        if(lenN == 512):
                            off = 0x200-4-i;
                            cmd = 'otpctrl -w' + ' ' + '0x0' + ' ' + str(hex(i)) +' ' + str("0x{:08x}".format(struct.unpack('>I',plainText[off:off+4])[0])).rstrip("L") + ';'
                        else:
                            if(reverse == True):
                                off=lenN-4-i;
                                cmd = 'otpctrl -w' + ' ' + '0x0' + ' ' + str(hex(i)) +' ' + str("0x{:08x}".format(struct.unpack('>I',plainText[off:off+4])[0])).rstrip("L") + ';'
                            else:
                                off=i
                                cmd = 'otpctrl -w' + ' ' + '0x0' + ' ' + str(hex(i)) +' ' + str("0x{:08x}".format(struct.unpack('I',plainText[off:off+4])[0])).rstrip("L") + ';'
                        f1.write(cmd)
                    f1.write('\r\n')
                f1.write('####### RSA E-Key ########\r\n')

                if(lenN == 512):
                    f1.write('otpctrl -w 0x1 0x0 0x00010001\r\n' )
                else:
                    if(reverse == True):
                        f1.write('otpctrl -w 0x1 0x0 0x00010001\r\n' )
                    else:
                        f1.write('otpctrl -w 0x1 0x0 0x01000100\r\n' )

            #uboot_scripts = 'rsaKeyDigest.txt'
            #with open(uboot_scripts, 'w') as f1:
            #    f1.write('//####### RSA Key Digest########\r\n')
            #    for j in range(0,32,8):
            #        for i in range(j,j+8,4):
            #            off=i;
            #            cmd = 'otpctrl -w' + ' ' + '0xe' + ' ' + str(hex(i)) +' ' + str("0x{:08x}".format(struct.unpack('I',digest[off:off+4])[0])).rstrip("L") + ';'
            #            f1.write(cmd)
            #        f1.write('\r\n')
    except:
        pass

def split_file_func(split_file):
    plainText = None
    realy_size=0
    sstar_print_val("split_file ",split_file)
    try:
        if split_file is not None:

            with open(split_file, "rb") as src_file:
                plainText = src_file.read()
                ipl_magic, ipl_fileSize = struct.unpack('IH', plainText[4:10])
                HDR_Ver = struct.unpack('B', plainText[14:15])
                realy_size = get_IPL_realy_size(HDR_Ver[0], ipl_fileSize)
                if 0x5f4C5049 == ipl_magic:
                    ipl_bin = plainText[:realy_size]
                    ipl_file = open("./IPL.bin", "wb")
                    ipl_file.write(ipl_bin)
                    ipl_file.close()
                    sstar_print_string("Split IPLX to IPL done  ")
                else:
                    printRed('split_file_func ERR: IPL magic checkout fail!')

                cust_magic, cust_fileSize = struct.unpack('IH', plainText[realy_size+256+4:realy_size+256+10])
                if 0x4E4C5049 == cust_magic:
                    realy_cust_fileSize = get_IPL_realy_size(HDR_Ver[0], cust_fileSize)
                    cust_bin = plainText[realy_size+256:realy_size+256+realy_cust_fileSize]
                    cust_file = open("./IPL_CUST.bin", "wb")
                    cust_file.write(cust_bin)
                    cust_file.close()
                    sstar_print_string("Split IPLX to IPL_CUST done >>>")
                else:
                    cust_magic, cust_fileSize = struct.unpack('IH', plainText[realy_size+512+4:realy_size+512+10])
                    realy_cust_fileSize = get_IPL_realy_size(HDR_Ver[0], cust_fileSize)
                    if 0x4E4C5049 == cust_magic:
                        cust_bin = plainText[ipl_fileSize+512:ipl_fileSize+512+realy_cust_fileSize]
                        cust_file = open("./IPL_CUST.bin", "wb")
                        cust_file.write(cust_bin)
                        cust_file.close()
                        sstar_print_string("Split IPLX to IPL_CUST done >>>")
                    else:
                        printRed('split_file_func ERR: IPL_CUST magic checkout fail!')
    except:
        printRed("ERR: open file fail!")
        pass

def do_secure_append(binfile, mode, anti, aeskeylen, aeskeynum,IVfile):
    magic = b"SSTARSBT"
    pend_version = 0
    sstar_pend_size = 32
    sel_aes_key = 0
    aes_enable = 0
    aes_type = 0
    result=b"\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"

    if IVfile is not None:
        with open(IVfile, 'rb') as IV_V:
            result = IV_V.read()

    if aeskeynum != 0 and aeskeylen != 0:
        aes_enable=1
        if aeskeylen == 256:
            sel_aes_key |= (1<<2)
        elif aeskeylen == 128:
            sel_aes_key |= (1<<3)
        sel_aes_key = sel_aes_key|(aeskeynum-1)

    if mode == "cbc":
        aes_type = 2
    elif mode == "ctr":
        aes_type = 1
    else:
        aes_type = 0

    binary_data = struct.pack(">8sBBBBB3s", magic, pend_version, sstar_pend_size, sel_aes_key, aes_type<<1|aes_enable,
                              anti, b"\x00" * 3 )

    with open(binfile, 'rb') as bin:
        plainText = bin.read()
        dir_path = os.path.dirname(binfile)
        stem, ext = os.path.splitext(os.path.basename(binfile))
        if ext != '':
            output_path = os.path.join(dir_path, stem + '.append' + ext)
        else:
            output_path = binfile + '.append'
        with open(output_path, 'wb') as out:
            out.write(plainText+binary_data+result[0:16])

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
    keyN = None
    key2sha1 = None
    reverse = False
    append = False
    mode="ecb"
    aeskeylen=0
    aeskeynum=0
    anti=0
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'vhf:',['version', 'IV=', 'aes=', 'rsa=', 'help', 'encrypt', 'insert', 'burn', 'sign', 'exportkey', \
            'exportkey_reverse','split', 'signusb', 'keyN', 'key2sha1', 'pwd2sha1', 'mode=', 'append', 'anti=', 'aeskeylen=', 'aeskeynum='])
        for o, a in opts:
            if o == "-f":
                binfile = os.path.abspath(a)
                sstar_print_val("image:",binfile)
            elif o == '-h' or o == '--help':
                usage()
            elif o == '-v' or o == '--version':
                version()
            elif o == '--IV':
                IVfile = os.path.abspath(a)
                sstar_print_val("IV:",IVfile)
            elif o == '--encrypt':
                encrypt = True
            elif o == '--mode':
                mode = str(a)
            elif o == '--insert':
                insert = True
            elif o == '--burn':
                burn = True
            elif o == '--sign':
                sign = True
            elif o == '--exportkey':
                exportkey = True
                reverse = False
            elif o == '--exportkey_reverse':
                exportkey = True
                reverse = True
            elif o == '--aes':
                cipherfile = os.path.abspath(a)
                sstar_print_val("aes:",cipherfile)
            elif o == "--rsa":
                keyfile = os.path.abspath(a)
                sstar_print_val("rsa:",keyfile)
            elif o == "--split":
                splitflag = True
            elif o == "--signusb":
                signusb = True
            elif o == '--keyN':
                keyN = True
            elif o == '--key2sha1':
                reverse = True
                key2sha1 = True
            elif o == '--pwd2sha1':
                reverse = False
                key2sha1 = True
            elif o == "--append":
                append = True
            elif o == "--anti":
                anti = int(a)
            elif o == "--aeskeylen":
                aeskeylen = int(a)
            elif o == "--aeskeynum":
                aeskeynum = int(a)
            else:
                assert False, "unhandled option %s" % o

        if 3 > len(sys.argv):
            usage()

        if True == encrypt:
            do_encrypt(cipherfile, binfile, IVfile, mode)
        elif True == insert:
            insert_key(binfile, keyfile, cipherfile,IVfile)
        elif True == burn:
            insert_key_for_burn(binfile, keyfile)
        elif True == sign:
            do_sign(keyfile, binfile)
        elif True == exportkey:
            export_rsakey(keyfile, reverse)
        elif True == splitflag:
            split_file_func(binfile)
        elif True == signusb:
            do_sign_usb(keyfile, binfile)
        elif True == keyN:
            do_keyN(binfile)
        elif True == key2sha1:
            key2sha1_xor(binfile,reverse)
        elif True == append:
            do_secure_append(binfile, mode, anti, aeskeylen, aeskeynum,IVfile)
        else:
            usage()
    except getopt.GetoptError as err:
        # print help information and exit:
        print(err) # will print something like "option -a not recognized"
        usage()

if __name__ == '__main__':
    main()
