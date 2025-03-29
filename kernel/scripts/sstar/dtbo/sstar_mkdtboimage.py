#!/usr/bin/python3
#
# Copyright (c) [2019~2020] SigmaStar Technology.
#
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#

import sys
import os
import argparse
import errno
import json
import time
import subprocess

DEBUG = False
SCRIPT_PATH = os.path.abspath(os.path.dirname(__file__))
SRC_TREE = ""
OBJ_TREE = ""
OUTPUT_DIR = ""

def exec_cmd(command):
    subp = subprocess.Popen(command,shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    subp.wait(1024)
    if DEBUG:
        print(command)
        print(subp.communicate()[0], end="")
    if subp.poll() != 0:
        print("Command:'{}' fail. Err code:{}".format(command, subp.returncode))
        exit(1)

def make_dtbo_image(configs, out_dir):
    command = "{} {} {} ".format(
            os.path.join(SCRIPT_PATH, "tools", "mkdtimg"),
            "create",
            os.path.join(out_dir, "dtbo.img"))

    if ("global_options" in configs) and (len(configs["global_options"])):
        for opt_key in configs["global_options"]:
            command = command + "--{}={} ".format( opt_key, configs["global_options"][opt_key])

    for entry_name in configs["entries"]:
        # get 'dtbo' prop which fill by function 'make_all_dtbo', append dtbo path to command
        command = command + configs["entries"][entry_name]["dtbo"] + " "

        # get 'options' prop, append extenal options for current dtbo entry to command
        if ("options" in configs["entries"][entry_name]) and (len(configs["entries"][entry_name]["options"])):
            for opt_key in configs["entries"][entry_name]["options"]:
                command = command + "--{}={} ".format(opt_key, configs["entries"][entry_name]["options"][opt_key])

    print("Building {} ...".format(os.path.join(out_dir, "dtbo.img")))
    exec_cmd(command)

def make_all_dtbo(configs):
    dtc_path = os.path.join(SCRIPT_PATH, "tools", "dtc")
    dtx_path = os.path.join(SCRIPT_PATH, "tools", "dtx_diff")

    for entry_name in configs["entries"]:
        if not "dts" in configs["entries"][entry_name]:
            print("Not 'dts' at entry: {}\nexit...".format(configs["entries"][entry_name]))
            exit(1)

        # dts_path_prop: arch/arm64/boot/dts/sstar/dtbo/pioneer5/pioneer5-ssc028a-s01a-padmux.dts(from json dts prop)
        # dts_dir: arch/arm64/boot/dts/sstar/dtbo/pioneer5
        # dts_path: $(OBJ_TREE)/arch/arm64/boot/dts/sstar/dtbo/pioneer5/pioneer5-ssc028a-s01a-padmux.dts(from json dts prop)
        # dts_name: pioneer5-ssc028a-s01a-padmux suffix: dts
        # temp_dts_path: $(OBJ_TREE)/$(dts_dir)/.$(dts_name).dts.tmp
        # dtbo_path: $(OBJ_TREE)/$(dts_dir)/$(dts_name).dtbo
        dts_path_prop = configs["entries"][entry_name]["dts"]
        dts_dir = os.path.dirname(dts_path_prop)
        dts_path = os.path.join(SRC_TREE, dts_path_prop)
        dts_name, suffix = os.path.splitext(os.path.basename(dts_path_prop))
        temp_dts_path = os.path.join(OBJ_TREE, dts_dir, ".{}.dts.tmp".format(dts_name))
        dtbo_path = os.path.join(OBJ_TREE, dts_dir, "{}.dtbo".format(dts_name))

        if not os.path.exists(os.path.dirname(temp_dts_path)):
            os.makedirs(os.path.dirname(temp_dts_path), 0o755)

        if not os.path.exists(os.path.dirname(dtbo_path)):
            os.makedirs(os.path.dirname(dtbo_path), 0o755)

        # add 'dtbo' prop which will read by function 'make_dtbo_image'
        configs["entries"][entry_name].update({"dtbo":dtbo_path})


        print("Building {} ...".format(dtbo_path))
        # use tool dtx to expand marco for .h
        exec_cmd("{dtx} {dts} > {temp_dts}".format(dtx=dtx_path, dts=dts_path, temp_dts=temp_dts_path))
        # use tool dtc to complie dt binary
        exec_cmd("{dtc} -@ -I dts -O dtb -o {dtbo} {temp_dts}".format(dtc=dtc_path, temp_dts=temp_dts_path, dtbo=dtbo_path))

def parse_args():
    parser = argparse.ArgumentParser(description='SigmaStar build dtbo image tool')
    parser.add_argument("-c", '--config', help="specify config file", required=True)
    parser.add_argument("-o", '--out', help="specify output path")
    args = parser.parse_args()

    if not os.path.exists(args.config):
        print("Not such file: {}\nexit ...".format(args.config))
        exit(1)

    return args

def main():
    global SRC_TREE
    global OBJ_TREE
    global OUTPUT_DIR

    args = parse_args()

    # Init global vars
    if os.getenv('srctree'):
        SRC_TREE = os.path.abspath(os.getenv('srctree'))
    else:
        SRC_TREE = os.path.abspath(".")

    if os.getenv('objtree'):
        OBJ_TREE = os.path.abspath(os.getenv('objtree'))
    else:
        OBJ_TREE = os.path.abspath(".")

    if None != args.out:
        OUTPUT_DIR = os.path.abspath(args.out)
    else:
        OUTPUT_DIR = os.path.abspath(".")

    if DEBUG:
        print("config:", args.config)
        print("srctree:", SRC_TREE)
        print("objtree:", OBJ_TREE)
        print("output dir:", OUTPUT_DIR)

    # Open config file
    with open(args.config) as config_file_handle:
        configs = json.load(config_file_handle)

    if (not "entries" in configs) or (not len(configs["entries"])):
        print("Not entries at config file: {}\nexit...".format(args.config))
        exit(1)

    # Build all dtbo
    make_all_dtbo(configs)

    # Build dtbo.img
    make_dtbo_image(configs, OUTPUT_DIR)

if __name__ == '__main__':
    main()
