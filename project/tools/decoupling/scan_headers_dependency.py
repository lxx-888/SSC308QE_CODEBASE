#!/usr/bin/python3
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

import sys, os, re
import json
from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("-f", help="cmd files", dest="cmd_files", nargs='+', type=str)
parser.add_argument("-k", help="kbuild dir", dest="kbuild_dir", type=str)
parser.add_argument("-w", help="white list json file", dest="white_list_json", type=str)

class cmd_file_scanner:
    def __init__(self, cmd_files, kbuild_dir, white_list_json=""):
        self.cmd_files = cmd_files
        self.kbuild_dir = kbuild_dir
        self.header_whitelist = self.parse_header_file_whitelist(white_list_json)
        self.header_pattern = re.compile(r'[^\s]*\.h[\s]')
        self.scan_result = {}

    def parse_header_file_whitelist(self, jsonfile):
        header_whitelist = []
        if os.path.isfile(jsonfile):
            with open(jsonfile, mode='r') as fd:
                jsondata = json.load(fd)
                for k, v in jsondata.items():
                    if k == "HDR_WHITELIST":
                        header_whitelist.extend(v)
                        break
        return header_whitelist

    def check_header_file(self, filename, kbuild_dir):
        if not os.path.isabs(filename):
            filename = os.path.join(kbuild_dir, filename)

        rel_path = os.path.relpath(os.path.normpath(filename), kbuild_dir)
        if rel_path.startswith('..'):
            return True

        if rel_path in self.header_whitelist:
            return True
        else:
            return False

    def do_scan(self):
        for cmd_file in self.cmd_files:
            with open(cmd_file, mode='r') as fd:
                result_header_files = []
                deps = False
                while True:
                    line = fd.readline()
                    if not line:
                        break
                    line = line.strip()
                    if not deps:
                        if line.startswith('deps_'):
                            deps = True
                            continue
                        else:
                            continue
                    if line.startswith('$(wildcard'):
                        continue
                    m = self.header_pattern.search(line)
                    if not m:
                        continue
                    if not self.check_header_file(m.group(0).strip(), self.kbuild_dir):
                        result_header_files.append(m.group(0).strip())
            if len(result_header_files) != 0:
                self.scan_result[cmd_file] = result_header_files


def main():
    args = parser.parse_args()
    cmd_files = args.cmd_files
    kbuild_dir = args.kbuild_dir
    white_list_json = args.white_list_json

    if not cmd_files or not kbuild_dir:
        print("invalid args !!!")
        sys.exit(1)

    scanner = cmd_file_scanner(cmd_files, kbuild_dir, white_list_json)
    scanner.do_scan()

    if len(scanner.scan_result) == 0:
        sys.exit(0)
    else:
        print("\033[31m[check decoupling failed, module contain linux header files !!!]\033[0m")
        for cf, header_files in scanner.scan_result.items():
            print("\033[31m===========<{}>============\033[0m".format(cf))
            for hf in header_files:
                print(hf)
        sys.exit(1)


if __name__ == '__main__':
    main()
