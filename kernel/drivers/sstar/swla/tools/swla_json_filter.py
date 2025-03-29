#!/usr/bin/python3

import sys, getopt
import base64
import os
import json
from subprocess import Popen, PIPE, STDOUT

def help():
    print ('{} -i <log_file> [-o <filtered_log_file>] -r <remove_log_name>'.format(sys.argv[0]))
    print ('{} -i <log_file> -l'.format(sys.argv[0]))

def list_all_item(input_log_file):
    name_list = []
    json_obj  = json.load(open(input_log_file))

    for i in range(len(json_obj)):
        if json_obj[i]["name"] not in name_list:
            name_list.append(json_obj[i]["name"])
            
    print("[ " + ','.join(str(x) for x in name_list) + " ]")

def filter_remove_items(input_log_file, output_log_file, remove_items):
    json_obj  = json.load(open(input_log_file))
    filtered_json_obj = []

    for i in range(len(json_obj)):
        if json_obj[i]["name"] not in remove_items:
            filtered_json_obj.append(json_obj[i])
    
    open(output_log_file, "w").write(json.dumps(filtered_json_obj, indent=4)
)
    
def main(argv):
    input_log_file=""
    output_log_file="swla_filtered_log.json"
    list_entry_only=False
    remove_items = []

    try:
        opts, args = getopt.getopt(argv,"lho:i:r:")
    except getopt.GetoptError:
        help()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            help()
            sys.exit()
        elif opt == '-i':
            input_log_file = arg
        elif opt == '-o':
            output_log_file = arg
        elif opt == '-r':
            remove_items = arg.split(',')
        elif opt == '-l':
            list_entry_only = True
    
    if input_log_file == "" or (list_entry_only == False and not remove_items):
        help()
        sys.exit(2)
    
    if list_entry_only == True:
        list_all_item(input_log_file)    
    else:
        filter_remove_items(input_log_file, output_log_file, remove_items)

if __name__ == "__main__":
   main(sys.argv[1:])
