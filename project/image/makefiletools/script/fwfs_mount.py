import sys, os, struct
import binascii
from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("device_name", help="mount device")
parser.add_argument("dir_name", help="mount directory")
parser.add_argument("-p", "--part_size", help="parttion size (Byte)", dest="part_size", type=str)
parser.add_argument("-r", "--rcs_dir", help="rcS directory", dest="rcs_dir", type=str)
parser.add_argument("-t", "--flash_type", help="flash type", dest="flash_type", type=str)
parser.add_argument("-g", "--page_size", help="page size", dest="page_size", type=str, default="0x800")
parser.add_argument("-b", "--block_size", help="block size", dest="block_size", type=str, default="0x20000")
parser.add_argument("-u","--compact_on_unmount", help="force compact on unmount", dest="compact_on_unmount", type=str, default="0x43554d54")

def main():
    args = parser.parse_args()
    block_size = 131072;
    subblock_size = 32768;
    cache_pool_size = 2;
    cache_size = 32768;
    file_cache_size = 32768;
    read_size = 2048;
    prog_size = 2048;
    cycle_count = 500;
    lookahead_size = 8;
    ucompact = 0x43554d54;
    part_size  = int(args.part_size, 16);
    flash_type = args.flash_type;
    page_size  = int(args.page_size, 16);

    if (flash_type == "spinand"):
        read_size = page_size;
        prog_size = page_size;
        block_size = int(args.block_size, 16);

    if (flash_type == "nor"):
        block_size = 65536
    block_count = part_size / block_size;

    fwfs_mount_command = 'fwfs' + ' --block_size=' + str(block_size) + ' --subblock_size=' + str(subblock_size) \
                    + ' --block_cycles=' + str(cycle_count) \
                    + ' --read_size=' + str(read_size) + ' --prog_size=' + str(prog_size) \
                    + ' --cache_size=' + str(cache_size) + ' --file_cache_size=' + str(file_cache_size) \
                    + ' --cache_pool_size=' + str(cache_pool_size) + ' --block_count=' + str(block_count) \
                    + ' --compact_on_unmount=' + str(ucompact) \
                    + ' --lookahead_size=' + str(lookahead_size) + ' ' + str(args.device_name) + ' ' + str(args.dir_name)


    echo_rcS_command = 'echo' + ' ' + fwfs_mount_command + ' ' + '>> ' + args.rcs_dir

    os.system(echo_rcS_command)
    print ('fwfs_mount_command is %s' %echo_rcS_command)

if __name__ == '__main__':
    main()
