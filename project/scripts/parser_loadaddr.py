#!/usr/bin/python

import  os, sys, argparse

def parser_ipl_elf_header(filepath, outfile):

    ipl_cus_elf=filepath.replace("IPL_CUST","ipl_cust_elf_header").replace("bin","txt")
    if not os.path.isfile(ipl_cus_elf):
        ipl_cus_elf=filepath.replace("IPLX","ipl_cust_elf_header").replace("bin","txt")
        if not os.path.isfile(ipl_cus_elf):
            path = os.path.dirname(filepath)
            ipl_cus_elf = os.path.join(path, "IPL_CUST_ELF.txt")
            if not os.path.isfile(ipl_cus_elf):
                print ('can not find elf file %s', ipl_cus_elf)
                sys.exit(1)
    print ('ipl_cus_load elf: %s' %(ipl_cus_elf))
    # Program Headers:
    #   Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
    #   LOAD           0x000000 0x20800000 0x20800000 0x00000 0xefff00 RW  0x10000
    #   LOAD           0x010000 0x21f00000 0x21f00000 0x078e0 0x078e0 RWE 0x10000
    #   LOAD           0x000000 0x21f20000 0x21f20000 0x00000 0x10c70 RW  0x10000
    #   LOAD           0x000000 0x21f40000 0x21f40000 0x00000 0xc0000 RW  0x10000
    #   GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10
    #
    #  Section to Segment mapping:
    #   Segment Sections...
    #    00     .dram_shmem
    #    01     .rom
    #    02     .ram
    #    03     .dram_heap
    #    04
    LoadTexts = os.popen('grep LOAD %s' %(ipl_cus_elf)).readlines()
    NameTexts = os.popen('grep "\.[a-z]"  %s' %(ipl_cus_elf)).readlines()
    #print (LoadTexts)
    #print (NameTexts)

    out = open(outfile, "a+")
    for i in range(len(LoadTexts)):
        name = filepath + NameTexts[i].split()[1]
        ih_size = LoadTexts[i].split()[5]
        ih_load = LoadTexts[i].split()[3]
        ih_load_end =  hex(int(ih_load,16)+int(ih_size,16))
        section_range = ih_load + "-" + ih_load_end
        out.write(name + "\n")
        out.write(ih_load + "-" + ih_load_end + "\n")
    out.close()

def parser_u_image(filepath, outfile):
    infile = open(filepath, "rb")
    out = open(outfile, "a+")
    infile.seek(12)
    ih_size = infile.read(4).hex()
    ih_load = infile.read(4).hex()
    ih_load_end =  hex(int(ih_load,16)+int(ih_size,16))
    out.write(filepath + "\n")
    out.write("0x" + ih_load + "-" + ih_load_end + "\n")
    infile.close()
    out.close()

def parser_tee_bin_image(filepath, outfile):
    infile = open(filepath, "rb")
    out = open(outfile, "a+")
    infile.seek(8)
    ih_size = infile.read(4).hex()
    ih_size = "".join(reversed([ih_size[i:i+2] for i in range(0, len(ih_size), 2)]))
    infile.seek(16)
    ih_load = infile.read(4).hex()
    ih_load = "".join(reversed([ih_load[i:i+2] for i in range(0, len(ih_load), 2)]))
    ih_load_end =  hex(int(ih_load,16)+int(ih_size,16))
    out.write(filepath + "\n")
    out.write("0x" + ih_load + "-" + ih_load_end + "\n")
    infile.close()
    out.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='check load addr from image head')
    parser.add_argument("--infile", help="in")
    parser.add_argument("--out", help="out")
    args=parser.parse_args()

    filename = os.path.basename(args.infile)
    if 'IPL' in filename:
        parser_ipl_elf_header(args.infile, args.out)
    elif 'tee.bin' == filename:
        parser_tee_bin_image(args.infile, args.out)
    else:
        parser_u_image(args.infile, args.out)
