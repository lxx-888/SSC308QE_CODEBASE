
import  os, sys, argparse
import logging

class ColoredFormatter(logging.Formatter):
    COLORS = {
        'DEBUG': '\033[96m',  # Cyan
        'INFO': '\033[92m',   # Green
        'WARNING': '\033[93m', # Yellow
        'ERROR': '\033[91m',   # Red
        'RESET': '\033[0m',    # Reset color
    }

    def format(self, record):
        log_color = self.COLORS.get(record.levelname, self.COLORS['RESET'])
        message = super().format(record)
        return "{}{}{}".format(log_color, message, self.COLORS['RESET'])
log = logging.getLogger()
handler = logging.StreamHandler()
formatter = ColoredFormatter('%(levelname)s - %(filename)s:%(lineno)d - %(message)s')
handler.setFormatter(formatter)
log.addHandler(handler)
log.setLevel(logging.INFO)

DRAM_BASE = 0x20000000

def check_overlap(layout):
    for module_ref,(start_ref,end_ref) in layout.items():
        layout_temp = dict(layout)
        del layout_temp[module_ref] #del self
        for module,(start,end) in layout_temp.items():
            if end_ref <= start or start_ref >= end:
                continue
            # overlap: <   [  >   ]
            log.error("We detected address overlap between:[")
            log.error("\t [0x{:x}-0x{:x}] <={}".format(start_ref, end_ref, os.path.basename(module_ref)))
            log.error("\t [0x{:x}-0x{:x}] <={}".format(start, end, os.path.basename(module)))
            log.error("]. You can see detail info in load_addr.svg")
            return False
    return True


def check_item_illegal(layout):
    #check item
    for module,(start,end) in layout.items():
        if start >= end or start < DRAM_BASE:
            log.error("Detected a err address config:")
            log.error("\t [0x{:x}-0x{:x}] <={}".format(start, end, module))
        if "IPL" in  module:
            if end > linux_limit:
                log.error("Check load addr fail, ipl_cus[0x{:x},0x{:x}] out of range[0x{:x},0x{:x}]"
                        .format(start, end, DRAM_BASE, linux_limit))
                return False
            continue
        if "u-boot" in module:
            if end > linux_limit:
                # Only Dualos scense need to pay attention:
                #   Nomal boot flow: IPL boot rtos&linux
                #       It can be work well~~
                #   Upgrade flow: IPL boot uboot, if you dont press ENTER to interrupt uboot, uboot will boot rtos&linux
                #       U-Boot will run concurrently with the RTOS. if uboot code text out of LX_MEM(in mma),
                #       rtos streaming piple use mma, then uboot code text maybe overwrite by rtos,
                #       and then linux will never boot successful.
                log.warning("The end of uboot load addr 0x{:x}>0x{:x}, its maybe not work well in dualos with boot by uboot!"
                            .format(end, linux_limit))
            continue
    return True

def print_layout(layout):
    log.info("image load addr check over, everything is well")
    # for module,(start,end) in layout.items():
    #     log.info("[0x{:x}-0x{:x}] <= {}".format(start, end ,module))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='check load addr from image head')
    parser.add_argument("--lxaddr", help="The end of LX_MEM")
    parser.add_argument("--layout", help="The file of image load address layout info.")
    args=parser.parse_args()

    if int(args.lxaddr,16) == 0:
        log.Warning("LX_MEM [{}] unknow!, ignore check".format(args.lxaddr))
        exit(0)
    if not os.path.isfile(args.layout):
        log.error("layout file [{}] not found!".format(args.layout))
        exit(-1)
    linux_limit = int(args.lxaddr,16) + DRAM_BASE

    with open(args.layout, 'r') as file:
        lines = file.readlines()
        lines = [_ for _ in lines if _.strip()]
    if len(lines) % 2 != 0:
        log.error("{} format err!".format(args.layout))
        exit(-1)
    # get layout info
    """ layout文件格式:
    path/of/file1
    0xstart-0xend
    path/of/file2
    0xstart-0xend
    ....
    """
    layout_info = {}
    for i in range(0, len(lines), 2):
        module = lines[i].strip()
        try:
            layout_info[module] = [int(_.strip(), 16) for _ in lines[i + 1].split('-')]
        except Exception as e:
            log.error("{} format err at line {}! : {}".format(args.layout, i, e))
            exit(-1)

    if not check_item_illegal(layout_info):
        exit(-1)

    if not check_overlap(layout_info):
        exit(-1)
    print_layout(layout_info)
