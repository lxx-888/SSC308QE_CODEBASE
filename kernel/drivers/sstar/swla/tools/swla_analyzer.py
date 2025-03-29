#!/usr/bin/python3

import sys, getopt
import base64
import os
import json
from enum import IntEnum
from subprocess import Popen, PIPE, STDOUT
from operator import itemgetter, attrgetter

class bcolors:
    RED   = '\033[31m'
    GREEN = '\033[32m'
    BLUE  = '\033[34m'
    RESET = '\033[0m'

# reference from RTOS sys_swla.h or Linux sstar_swla.h
############################################################
class entry_class:
    def __init__(self, log_type, cpu_id, log_name):
        self.log_type = log_type
        self.cpu_id = cpu_id
        self.log_name = log_name

class E_OSType(IntEnum):
    SS_SWLA_OS_RTOS = 0
    SS_SWLA_OS_Linux = 1

class E_LogType(IntEnum):
    SS_SWLA_LOG_START = 0
    SS_SWLA_LOG_STOP = 1
    SS_SWLA_LOG_LABEL = 2
    SS_SWLA_LOG_SWITCH_IN = 3

class LogHeader:
    def __init__(self):
        self.magic_number = 0
        self.header_size = 0
        self.major_version = 0
        self.minor_version = 0
        self.tick_freq = 0
        self.start_tick = 0
        self.log_format = 0
        self.os_type = 0
        self.buf_kb_shift = 0
############################################################

HEADER_MAGIC_NUMBER = b'2vALWSSS'
PARSER_SWLA_LOG_MAJOR_VERSION = 2
PARSER_SWLA_LOG_MINOR_VERSION = 0
DUALOS_RTOS_FILE_PREFIX = "ss_rtos_"
DUALOS_RTOS_LOG_PREFIX = "R-"
DUALOS_LINUX_FILE_PREFIX = "ss_linux_"
DUALOS_LINUX_LOG_PREFIX = "L-"
JSON_FILENAME_EXTENSION = ".json"
USER_EVENT_PID = "EVENT"
MIN_TIME_GAP = 0.001

# global variable
g_debug_enable = False
g_log_header = LogHeader()
log_file_list = []
overall_entry_list = []
event_entry_list = []
isr_entry_list = []
label_entry_list = []
swla_binary_data_len = 16
swla_log_name_start = 8
swla_log_name_end = 16
max_support_cpus = 16
max_cpu_id = 0
cur_task_each_cpu = ["" for i in range(max_support_cpus)]
first_log_time_each_cpu = [0 for i in range(max_support_cpus)]
higher_pri_list = [[] for i in range(max_support_cpus)]
wait_stop_list = []
json_list = []
temp_file_list = []

def reset_global_vars():
    global first_log_time_each_cpu, cur_task_each_cpu, wait_stop_list, json_list

    cur_task_each_cpu = ["" for i in range(max_support_cpus)]
    first_log_time_each_cpu = [0 for i in range(max_support_cpus)]
    wait_stop_list.clear()
    json_list.clear()

def add_to_temp_file_list(temp_file):
    temp_file_list.append(temp_file)

def remove_temp_files():
    for file in temp_file_list:
        if os.path.exists(file):
            os.remove(file)
            #print("    Remove " + file)

def print_log_hader_info():
    print("==============================")
    print("log header size: " + str(g_log_header.header_size))
    print("log version: v" + str(g_log_header.major_version) + '.' + str(g_log_header.minor_version))
    print("ticks per second: " + str(g_log_header.tick_freq))
    print("start tick: " + str(g_log_header.start_tick))
    print("log format: " + str(g_log_header.log_format))
    print("OS type: " + str(g_log_header.os_type))
    print("buf kb shift: " + str(g_log_header.buf_kb_shift))
    print("==============================")

def parse_header(file_obj):
    bytes_read = file_obj.read(8)
    # check magic number and parse header
    if bytes_read == HEADER_MAGIC_NUMBER:
        global swla_binary_data_len, swla_log_name_end

        g_log_header.header_size = int.from_bytes(file_obj.read(2), byteorder='little')
        g_log_header.major_version = int.from_bytes(file_obj.read(1), byteorder='little')
        g_log_header.minor_version = int.from_bytes(file_obj.read(1), byteorder='little')
        g_log_header.tick_freq = int.from_bytes(file_obj.read(4), byteorder='little')
        g_log_header.start_tick = int.from_bytes(file_obj.read(8), byteorder='little')
        g_log_header.log_format = int.from_bytes(file_obj.read(1), byteorder='little')
        if g_log_header.log_format == 0: # little weight log format
            swla_binary_data_len = 16
            swla_log_name_end = 16
        elif g_log_header.log_format == 1: # full log format
            swla_binary_data_len = 32
            swla_log_name_end = 24
        else:
            print(bcolors.RED + 'Warning! not support log format ' + g_log_header.log_format + bcolors.RESET)

        g_log_header.os_type = int.from_bytes(file_obj.read(1), byteorder='little')
        g_log_header.buf_kb_shift = int.from_bytes(file_obj.read(1), byteorder='little')
        if g_log_header.major_version != PARSER_SWLA_LOG_MAJOR_VERSION or g_log_header.minor_version != PARSER_SWLA_LOG_MINOR_VERSION:
            print(bcolors.RED + 'Error! not support log version ' + g_log_header.major_version + '.' + g_log_header.minor_version + bcolors.RESET)
            sys.exit(2)
    else:
        print(bcolors.RED + 'Warning! This log not include header.' + bcolors.RESET)
    return 0

def get_file_size(file_obj):
    file_obj.seek(0, 2)
    fsize = file_obj.tell()
    file_obj.seek(0, 0)
    return fsize

def get_log_type(log_entry):
    return log_entry[0] & 0x3

def get_cpu_id(log_entry):
    return (log_entry[0] >> 2) & 0xF

def get_timestamp_ns(log_entry):
    timestamp_ticks = ((log_entry[0] >> 6) & 0x3) | (log_entry[1] << 2) | (log_entry[2] << 10) | (log_entry[3] << 18) | (log_entry[4] << 26) | (log_entry[5] << 34) | (log_entry[6] << 42) | (log_entry[7] << 50)
    if g_log_header.tick_freq == 0:
        timestamp_ns = timestamp_ticks
    else:
        timestamp_ns = (timestamp_ticks * 1000000000) // g_log_header.tick_freq
    return timestamp_ns

def add_start_log(cpu_id, log_name, timestamp_ms):
    tid_pre = cur_task_each_cpu[cpu_id]

    if log_name.startswith("ISR-"):
        if cur_task_each_cpu[cpu_id] != "": #add current task stop only in ISR
            json_list.append({"name": cur_task_each_cpu[cpu_id], "ph": "E", "pid": "CPU" + str(cpu_id), "tid": tid_pre, "ts": timestamp_ms-MIN_TIME_GAP})
            # add overall
            json_list.append({"name": cur_task_each_cpu[cpu_id], "ph": "E", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": timestamp_ms-MIN_TIME_GAP})
        json_list.append({"name": log_name, "ph": "B", "pid": "ISR" + "-CPU" + str(cpu_id), "tid": log_name + "(" + str(int(log_name[4:8], 16)) + ")", "ts": timestamp_ms})
        # add overall
        json_list.append({"name": log_name, "ph": "B", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": timestamp_ms})
    else:
        json_list.append({"name": log_name, "ph": "B", "pid": USER_EVENT_PID, "tid": log_name, "ts": timestamp_ms})
    wait_stop_list.append(entry_class(E_LogType.SS_SWLA_LOG_START, cpu_id, log_name))

def add_stop_log(cpu_id, log_name, timestamp_ms, check_start):
    tid_pre = cur_task_each_cpu[cpu_id]

    if log_name.startswith("ISR-"):
        if check_start == True and not any(log_name == list_obj.log_name and cpu_id == list_obj.cpu_id for list_obj in wait_stop_list): #lost SS_SWLA_LOG_START
            json_list.append({"name": log_name, "ph": "B", "pid": "ISR" + "-CPU" + str(cpu_id), "tid": log_name + "(" + str(int(log_name[4:8], 16)) + ")", "ts": first_log_time_each_cpu[cpu_id]})
            # add overall
            json_list.append({"name": log_name, "ph": "B", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": first_log_time_each_cpu[cpu_id]})
        json_list.append({"name": log_name, "ph": "E", "pid": "ISR" + "-CPU" + str(cpu_id), "tid": log_name + "(" + str(int(log_name[4:8], 16)) + ")", "ts": timestamp_ms})    
        # add overall
        json_list.append({"name": log_name, "ph": "E", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": timestamp_ms})
        if check_start == True and cur_task_each_cpu[cpu_id] != "":
            json_list.append({"name": cur_task_each_cpu[cpu_id], "ph": "B", "pid": "CPU" + str(cpu_id), "tid": tid_pre, "ts": timestamp_ms+MIN_TIME_GAP})
            # add overall
            json_list.append({"name": cur_task_each_cpu[cpu_id], "ph": "B", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": timestamp_ms+MIN_TIME_GAP})
    else:
        if check_start == True and not any(log_name == list_obj.log_name and cpu_id == list_obj.cpu_id for list_obj in wait_stop_list): #lost SS_SWLA_LOG_START
            json_list.append({"name": log_name, "ph": "B", "pid": USER_EVENT_PID, "tid": log_name, "ts": first_log_time_each_cpu[cpu_id]})
            # add overall
            json_list.append({"name": log_name, "ph": "B", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": first_log_time_each_cpu[cpu_id]})
        json_list.append({"name": log_name, "ph": "E", "pid": USER_EVENT_PID, "tid": log_name, "ts": timestamp_ms})

    for list_obj in wait_stop_list:
        if list_obj.log_name == log_name and list_obj.cpu_id == cpu_id and E_LogType.SS_SWLA_LOG_START == list_obj.log_type:
            wait_stop_list.remove(list_obj)
            break

def add_label_log(cpu_id, log_name, timestamp_ms):
    if log_name == "_LaStop_":
        while wait_stop_list:
            stop_log_entry=wait_stop_list.pop()
            if stop_log_entry.log_type == E_LogType.SS_SWLA_LOG_START:
                add_stop_log(stop_log_entry.cpu_id, stop_log_entry.log_name, timestamp_ms - 0.01, False)
            elif stop_log_entry.log_type == E_LogType.SS_SWLA_LOG_SWITCH_IN:
                add_switchin_log(stop_log_entry.cpu_id, stop_log_entry.log_name, timestamp_ms - 0.01, True)

        json_list.append({"name": "swla_stop", "ph": "I", "pid": "Label", "tid": "swla_stop", "ts": timestamp_ms})
    elif log_name == "_LaExce_":
        json_list.append({"name": "exception", "ph": "I", "pid": "Label", "tid": "exception", "ts": timestamp_ms})
    else:
        json_list.append({"name": log_name, "ph": "I", "pid": "Label", "tid": log_name, "ts": timestamp_ms})

def add_switchin_log(cpu_id, log_name, timestamp_ms, set_stop):
    tid_pre = cur_task_each_cpu[cpu_id]
    tid = log_name

    if set_stop != True and cur_task_each_cpu[cpu_id] != "":
        json_list.append({"name": cur_task_each_cpu[cpu_id], "ph": "E", "pid": "CPU" + str(cpu_id), "tid": tid_pre, "ts": timestamp_ms-MIN_TIME_GAP})
        # add overall
        json_list.append({"name": cur_task_each_cpu[cpu_id], "ph": "E", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": timestamp_ms-MIN_TIME_GAP})
        for list_obj in wait_stop_list:
            if list_obj.log_name == cur_task_each_cpu[cpu_id] and E_LogType.SS_SWLA_LOG_SWITCH_IN == list_obj.log_type:
                wait_stop_list.remove(list_obj)
                break
    if set_stop == True:
        json_list.append({"name": log_name, "ph": "E", "pid": "CPU" + str(cpu_id), "tid": tid, "ts": timestamp_ms})
        # add overall
        json_list.append({"name": log_name, "ph": "E", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": timestamp_ms})
    else:
        json_list.append({"name": log_name, "ph": "B", "pid": "CPU" + str(cpu_id), "tid": tid, "ts": timestamp_ms})
        # add overall
        json_list.append({"name": log_name, "ph": "B", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": timestamp_ms})
        cur_task_each_cpu[cpu_id] = log_name
        wait_stop_list.append(entry_class(E_LogType.SS_SWLA_LOG_SWITCH_IN, cpu_id, log_name))

def add_sorted_list_log(cpu_id, log_name, is_label):
    if log_name.startswith("ISR-"):
        json_list.append({"name": log_name, "ph": "D", "pid": "ISR" + "-CPU" + str(cpu_id), "tid": log_name + "(" + str(int(log_name[4:8], 16)) + ")"})
    elif log_name.startswith("Overall]"):
        json_list.append({"name": "Overall", "ph": "D", "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall"})
    else:
        if is_label:
            json_list.append({"name": log_name, "ph": "D", "pid": "Label", "tid": log_name})
        else:
            json_list.append({"name": log_name, "ph": "D", "pid": "CPU" + str(cpu_id), "tid": log_name})

def log_to_json(log_entry):
    log_type = get_log_type(log_entry)
    cpu_id = get_cpu_id(log_entry)
    timestamp_ms = get_timestamp_ns(log_entry) / 1000
    log_name = log_entry[swla_log_name_start:swla_log_name_end].decode("ASCII")
    log_name = log_name.rstrip(u'\u0000')
    #print("[" + str(log_type) + "," + str(cpu_id) + "," + log_name + "," + str(timestamp_ms) + "]")
    if first_log_time_each_cpu[cpu_id] == 0:
        if log_type == E_LogType.SS_SWLA_LOG_STOP: # drop first stop event 
            return
        first_log_time_each_cpu[cpu_id] = timestamp_ms

    if timestamp_ms == 0 and log_name == "": # drop empty log
        return

    if log_type == E_LogType.SS_SWLA_LOG_START:
        add_start_log(cpu_id, log_name, timestamp_ms)
    elif log_type == E_LogType.SS_SWLA_LOG_STOP:
        add_stop_log(cpu_id, log_name, timestamp_ms, True)
    elif log_type == E_LogType.SS_SWLA_LOG_LABEL:
        add_label_log(cpu_id, log_name, timestamp_ms)
    elif log_type == E_LogType.SS_SWLA_LOG_SWITCH_IN:
        add_switchin_log(cpu_id, log_name, timestamp_ms, False)

def generate_entry_list(file_obj, f_size):
    global max_support_cpus, max_cpu_id

    event_entry_list.clear()
    isr_entry_list.clear()
    label_entry_list.clear()

    file_obj.seek(g_log_header.header_size, 0)
    while True:
        if file_obj.tell() >= f_size:
            break;

        bytes_read = file_obj.read(swla_binary_data_len)
        log_type = get_log_type(bytes_read)
        cpu_id = get_cpu_id(bytes_read)
        if cpu_id > max_cpu_id:
            max_cpu_id = cpu_id
        log_name = bytes_read[swla_log_name_start:swla_log_name_end].decode("ASCII")
        log_name = log_name.rstrip(u'\u0000')
        entry_obj = entry_class(log_type, cpu_id, log_name)
        if log_type == E_LogType.SS_SWLA_LOG_LABEL:
            if not any(entry_obj.log_name == list_obj.log_name and entry_obj.cpu_id == list_obj.cpu_id for list_obj in label_entry_list):
                label_entry_list.append(entry_obj);
        elif log_name.startswith("ISR-"):
            if not any(entry_obj.log_name == list_obj.log_name and entry_obj.cpu_id == list_obj.cpu_id for list_obj in isr_entry_list):
                isr_entry_list.append(entry_obj);
        else:
            if log_type != E_LogType.SS_SWLA_LOG_START and log_type != E_LogType.SS_SWLA_LOG_STOP:
                if not any(entry_obj.log_name == list_obj.log_name and entry_obj.cpu_id == list_obj.cpu_id for list_obj in event_entry_list):
                    event_entry_list.append(entry_obj);

    if max_cpu_id + 1 >= max_support_cpus:
        print("get invalid cpu ID:" + str(max_cpu_id) + ", max support ID is " + str(max_support_cpus - 1))
        sys.exit(2)
    event_entry_list.sort(key=attrgetter('cpu_id', 'log_name'))
    isr_entry_list.sort(key=attrgetter('cpu_id', 'log_name'))
    label_entry_list.sort(key=attrgetter('cpu_id', 'log_name'))
    #for entry_obj in event_entry_list:
    #    print("[EVENT]" + entry_obj.log_name + '-' + str(entry_obj.cpu_id))
    #for entry_obj in isr_entry_list:
    #    print("[ISR]" + entry_obj.log_name + '-' + str(entry_obj.cpu_id))
    #for entry_obj in label_entry_list:
    #    print("[LABEL]" + entry_obj.log_name + '-' + str(entry_obj.cpu_id))

def add_sorted_entry_list():
    for cpu_id in range(max_cpu_id + 1):
        add_sorted_list_log(cpu_id, "Overall]", False)
    for list_obj in event_entry_list:
        add_sorted_list_log(list_obj.cpu_id, list_obj.log_name, False)
    for list_obj in isr_entry_list:
        add_sorted_list_log(list_obj.cpu_id, list_obj.log_name, False)
    for list_obj in label_entry_list:
        add_sorted_list_log(list_obj.cpu_id, list_obj.log_name, True)

def generate_json_file(file_obj, f_size, first_log_offset, json_file_name):
    reset_global_vars()
    add_sorted_entry_list()
    f_json = open(json_file_name, "w")
    file_obj.seek(first_log_offset, 0)
    while True:
        if file_obj.tell() > (f_size - swla_binary_data_len):
            if first_log_offset == g_log_header.header_size:
                break;
            file_obj.seek(g_log_header.header_size, 0)

        bytes_read = file_obj.read(swla_binary_data_len)
        log_to_json(bytes_read)

        if file_obj.tell() == first_log_offset:
            break

    #remove IDLE, NonSecur, and swapper/x
    json_list_final = []
    for entry in json_list:
        if not entry['name'].startswith("IDLE") and not entry['name'].startswith("NonSecur") and not entry['name'].startswith("swapper/"):
            json_list_final.append(entry)
    json_string = json.dumps(json_list_final, indent='\t')
    #print(json_string)
    f_json.write(json_string)
    f_json.close()

def get_first_log(file_obj, file_size):
    tmp_timestamp_ns = 0
    tmp_offset = g_log_header.header_size
    min_timestamp_ns = 0
    min_offset = 0
    file_obj.seek(tmp_offset, 0)
    while tmp_offset < file_size:
        bytes_read = file_obj.read(swla_binary_data_len)
        tmp_timestamp_ns = get_timestamp_ns(bytes_read)
        if min_timestamp_ns == 0 or tmp_timestamp_ns < min_timestamp_ns:
            min_timestamp_ns = tmp_timestamp_ns
            min_offset = tmp_offset
        tmp_offset += swla_binary_data_len

    file_obj.seek(min_offset, 0)
    return min_offset

def pre_process_log_file(log_file):
    print("Pre-process log file ...")
    with open(log_file, "rb") as f_log:
        # file size should be aligned to swla_binary_data_len
        f_size = get_file_size(f_log)
        if f_size % swla_binary_data_len:
            print('Size of {} is not {} alignment'.format(log_file, swla_binary_data_len))
            sys.exit(2)

        # check header
        parse_header(f_log)

        # more than one log file, split it
        if f_size > (((1 << g_log_header.buf_kb_shift) << 10) + g_log_header.header_size):
            if g_debug_enable:
                print("    It's DualOS log!")
            f_log.seek(0, 0)
            if g_log_header.os_type == E_OSType.SS_SWLA_OS_RTOS:
                new_file = DUALOS_RTOS_FILE_PREFIX + log_file
                with open(new_file, "wb") as f_rtos_log:
                    f_rtos_log.write(f_log.read((((1 << g_log_header.buf_kb_shift) << 10) + g_log_header.header_size)))
                log_file_list.append(new_file)
                add_to_temp_file_list(new_file)
                new_file = DUALOS_LINUX_FILE_PREFIX + log_file
                with open(new_file, "wb") as f_linux_log:
                    f_linux_log.write(f_log.read(f_size - (((1 << g_log_header.buf_kb_shift) << 10) + g_log_header.header_size)))
                log_file_list.append(new_file)
                add_to_temp_file_list(new_file)
            else:
                new_file = DUALOS_LINUX_FILE_PREFIX + log_file
                with open(new_file, "wb") as f_linux_log:
                    f_linux_log.write(f_log.read((((1 << g_log_header.buf_kb_shift) << 10) + g_log_header.header_size)))
                log_file_list.append(new_file)
                add_to_temp_file_list(new_file)
                new_file = DUALOS_RTOS_FILE_PREFIX + log_file
                with open(new_file, "wb") as f_rtos_log:
                    f_rtos_log.write(f_log.read(f_size - (((1 << g_log_header.buf_kb_shift) << 10) + g_log_header.header_size)))
                log_file_list.append(new_file)
                add_to_temp_file_list(new_file)
        else:
            log_file_list.append(log_file)

# parse log file and generate json file
def parse_log_file(log_file, json_output_file):
    with open(log_file, "rb") as f_log:
        # file size should be aligned to swla_binary_data_len
        f_size = get_file_size(f_log)
        if f_size % swla_binary_data_len:
            print('Size of {} is not {} alignment'.format(log_file, swla_binary_data_len))
            sys.exit(2)

        # check header
        print("Check header ...")
        parse_header(f_log)
        if g_debug_enable:
            print_log_hader_info()
        # find first item
        first_log_offset = get_first_log(f_log, f_size)
        # build entry list
        print("Build entry list ...")
        generate_entry_list(f_log, f_size)
        # generate json file for chrome trace viewer
        print("Generate json chart ...")
        generate_json_file(f_log, f_size, first_log_offset, json_output_file)

def generate_json_higher_priority_list(json_list):
    global higher_pri_list

    for obj in json_list:
        if obj['pid'].startswith('Overall'):
            cpu_id = int(''.join(filter(lambda x: x.isdigit(), obj['pid'])))
            higher_pri_list[cpu_id].append(obj)

arrange_filter_event_count = 0
arrange_filter_event_debug = False
def add_json_event_with_overall(cpu_id, json_obj):
    global higher_pri_list

    json_list_with_overall = []

    json_list_with_overall.append(json_obj)
    json_list_with_overall.append({"name": json_obj['name'], "ph": json_obj['ph'], "pid": "Overall-" + "CPU" + str(cpu_id), "tid": "Overall", "ts": json_obj['ts']})

    if arrange_filter_event_debug == True and len(higher_pri_list[cpu_id]) != 0:
        for obj in json_list_with_overall:
            print("--->" + str(obj))

    return json_list_with_overall

def arrange_lower_priority_event(cpu_id, json_start_obj, json_end_obj):
    global arrange_filter_event_count, arrange_filter_event_debug
    global higher_pri_list

    json_arranged_list = []
    higher_list_remove_count = 0

    if len(higher_pri_list[cpu_id]) != 0:
        if arrange_filter_event_debug == True:
            print('list len: ' + str(len(higher_pri_list[cpu_id])))
            arrange_filter_event_count = arrange_filter_event_count + 1
            print("[" + str(arrange_filter_event_count) + "]lower pri obj")
            print(json_start_obj)
            print(json_end_obj)

        if arrange_filter_event_debug == True and arrange_filter_event_count > 10000:
            json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_start_obj)
            json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_end_obj)
            return json_arranged_list

        json_higher_start_obj = None
        json_higher_end_obj = None
        odd_higher_obj = None
        residue_event_interval = True

        # process higher priority filter
        for higher_pri_obj in higher_pri_list[cpu_id]:
            if odd_higher_obj == None:
                odd_higher_obj = higher_pri_obj
            else:
                if odd_higher_obj == None:
                    print(bcolors.RED + 'Error! no match filter start ' + bcolors.RESET)
                    print(entry)
                    sys.exit(3)

                json_higher_start_obj = odd_higher_obj
                json_higher_end_obj = higher_pri_obj
                odd_higher_obj = None

                if (json_higher_start_obj['ph'] != 'B' or json_higher_end_obj['ph'] != 'E') or (json_higher_start_obj['ts'] >= json_higher_end_obj['ts']):
                    print(bcolors.RED + 'Error! no match filter interval ' + bcolors.RESET)
                    print(json_higher_start_obj)
                    print(json_higher_end_obj)
                    sys.exit(3)

                #                  |----low pri----|
                # |---high pri---|
                if json_higher_end_obj['ts'] <= json_start_obj['ts']:
                    if arrange_filter_event_debug == True:
                        print("filter event case 0")
                    if json_higher_end_obj['ts'] == json_start_obj['ts']:
                        json_start_obj['ts'] = json_start_obj['ts'] + MIN_TIME_GAP

                    higher_list_remove_count = higher_list_remove_count + 2
                    continue
                # |----low pri----|
                #                    |---high pri---|
                elif json_higher_start_obj['ts'] >= json_end_obj['ts']:
                    if arrange_filter_event_debug == True:
                        print("filter event case 1")
                    if json_higher_start_obj['ts'] == json_end_obj['ts']:
                        json_end_obj['ts'] = json_end_obj['ts'] - MIN_TIME_GAP
                    if json_end_obj['ts'] > json_start_obj['ts']:
                        json_temp_start_obj = json_start_obj.copy()
                        json_temp_end_obj = json_end_obj.copy()
                        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_temp_start_obj)
                        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_temp_end_obj)
                    residue_event_interval = False
                    break
                #      |----low pri----|
                #   |------high pri------|
                elif json_higher_start_obj['ts'] <= json_start_obj['ts'] and json_higher_end_obj['ts'] >= json_end_obj['ts']:
                    if arrange_filter_event_debug == True:
                        print("filter event case 2")
                    residue_event_interval = False
                    break
                #     |----low pri----|
                # |--high pri--|
                elif json_higher_start_obj['ts'] < json_start_obj['ts'] and json_higher_end_obj['ts'] < json_end_obj['ts']:
                    if arrange_filter_event_debug == True:
                        print("filter event case 3")
                    json_start_obj['ts'] = json_higher_end_obj['ts'] + MIN_TIME_GAP
                #  |----low pri-----|
                #    |--high pri--|
                elif json_higher_start_obj['ts'] >= json_start_obj['ts'] and json_higher_end_obj['ts'] <= json_end_obj['ts']:
                    if arrange_filter_event_debug == True:
                        print("filter event case 4")
                    json_temp_start_obj = json_start_obj.copy()
                    json_temp_end_obj = json_end_obj.copy()
                    json_temp_end_obj['ts'] = json_higher_start_obj['ts'] - MIN_TIME_GAP
                    if json_temp_end_obj['ts'] > json_temp_start_obj['ts']:
                        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_temp_start_obj)
                        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_temp_end_obj)
                    json_start_obj['ts'] = json_higher_end_obj['ts'] + MIN_TIME_GAP
                    if json_start_obj['ts'] >= json_end_obj['ts']:
                        if arrange_filter_event_debug == True:
                            print("filter event case 4-1")
                        residue_event_interval = False
                        break
                # |----low pri----|
                #       |--high pri--|
                elif json_higher_start_obj['ts'] >= json_start_obj['ts'] and json_higher_end_obj['ts'] >= json_end_obj['ts']:
                    if arrange_filter_event_debug == True:
                        print("filter event case 5")
                    json_temp_start_obj = json_start_obj.copy()
                    json_temp_end_obj = json_end_obj.copy()
                    json_temp_end_obj['ts'] = json_higher_start_obj['ts'] - MIN_TIME_GAP
                    if temp_obj['ts'] > json_start_obj['ts']:
                        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_temp_start_obj)
                        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_temp_end_obj)
                    residue_event_interval = False
                    break

        if residue_event_interval == True:
            json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_start_obj)
            json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_end_obj)

        if arrange_filter_event_debug == True:
            print("higher_list_remove_count = " + str(higher_list_remove_count))

        # remove too old higher priority filter
        for i in range(higher_list_remove_count):
            rm_obj = higher_pri_list[cpu_id].pop(0)
            if arrange_filter_event_debug == True:
                print("rm " + str(rm_obj))
    else:
        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_start_obj)
        json_arranged_list = json_arranged_list + add_json_event_with_overall(cpu_id, json_end_obj)

    if arrange_filter_event_debug == True and len(higher_pri_list[cpu_id]) != 0:
        for obj in json_arranged_list:
            print("===>" + str(obj))

    return json_arranged_list

def arrange_dualos_json_list(json_obj, os_type):
    json_sort_entry_overall = []
    json_sort_entry_event = []
    json_sort_entry_isr = []
    json_sort_entry_label = []
    json_list_dualos = []
    json_start_obj = [None for i in range(max_support_cpus)]

    get_first_ts = False
    if os_type == E_OSType.SS_SWLA_OS_RTOS:
        dualos_log_prefix = DUALOS_RTOS_LOG_PREFIX
        dualos_isr_prefix = 'R-ISR-'
        dualos_first_log_label = "R-first_log"
    else:
        dualos_log_prefix = DUALOS_LINUX_LOG_PREFIX
        dualos_isr_prefix = 'L-ISR-'
        dualos_first_log_label = "L-first_log"

    for entry in json_obj:
        if not entry['name'].startswith('Overall'):
            entry['name'] = dualos_log_prefix + entry['name']
        if not entry['tid'].startswith('Overall'):
            entry['tid'] = dualos_log_prefix + entry['tid']

        if entry['ph'] == 'D':
            if entry['name'].startswith('Overall'):
                json_sort_entry_overall.append(entry)
            elif entry['name'].startswith(dualos_isr_prefix):
                json_sort_entry_isr.append(entry)
            elif entry['pid'].startswith('Label'):
                json_sort_entry_label.append(entry)
            else:
                json_sort_entry_event.append(entry)
        else:
            if not get_first_ts:
                json_list_dualos.append({"name": dualos_first_log_label, "ph": "I", "pid": "Label", "tid": dualos_first_log_label, "ts" : str(entry['ts'] - 0.01)})
                get_first_ts = True

            if (entry['ph'] == 'B' or entry['ph'] == 'E') and entry['pid'] != USER_EVENT_PID:
                cpu_id = int(''.join(filter(lambda x: x.isdigit(), entry['pid'])))
                if entry['tid'] != "Overall":
                    if entry['ph'] == 'B':
                        json_start_obj[cpu_id] = entry
                    else:
                        if json_start_obj[cpu_id] == None:
                            print(bcolors.RED + 'Error! no match event start ' + bcolors.RESET)
                            print(entry)
                            sys.exit(3)
                        json_list_dualos = json_list_dualos + arrange_lower_priority_event(cpu_id, json_start_obj[cpu_id], entry)
                        json_start_obj[cpu_id] = None
            else:
                json_list_dualos.append(entry)

    return json_sort_entry_overall, json_sort_entry_event, json_sort_entry_isr, json_sort_entry_label, json_list_dualos

def merge_json_file(rtos_json_file, linux_json_file):
    global max_support_cpus
    json_dualos_sort_entry_overall = []
    json_dualos_sort_entry_event = []
    json_dualos_sort_entry_isr = []
    json_dualos_sort_entry_label = []
    json_dualos_list = []

    # load RTOS / Linux json file
    with open(rtos_json_file, "r") as f_json:
        rtos_json_obj = json.load(f_json)
    with open(linux_json_file, "r") as f_json:
        linux_json_obj = json.load(f_json)

    # handle RTOS json file
    json_sort_entry_overall, json_sort_entry_event, json_sort_entry_isr, json_sort_entry_label, json_list = arrange_dualos_json_list(rtos_json_obj, E_OSType.SS_SWLA_OS_RTOS)
    json_dualos_sort_entry_overall = json_sort_entry_overall
    json_dualos_sort_entry_event = json_sort_entry_event
    json_dualos_sort_entry_isr = json_sort_entry_isr
    json_dualos_sort_entry_label = json_sort_entry_label
    json_dualos_list = json_list
    generate_json_higher_priority_list(json_list)

    # handle Linux json file
    json_sort_entry_overall, json_sort_entry_event, json_sort_entry_isr, json_sort_entry_label, json_list = arrange_dualos_json_list(linux_json_obj, E_OSType.SS_SWLA_OS_Linux)
    json_dualos_sort_entry_overall += json_sort_entry_overall
    json_dualos_sort_entry_event += json_sort_entry_event
    json_dualos_sort_entry_isr += json_sort_entry_isr
    json_dualos_sort_entry_label += json_sort_entry_label
    json_dualos_list += json_list

    # save merged json list to file
    json_dualos_sort_entry_event.sort(key=lambda element: element['pid'])
    json_dualos_sort_entry_isr.sort(key=lambda element: element['pid'])
    json_dualos_list = json_dualos_sort_entry_overall + json_dualos_sort_entry_event + json_dualos_sort_entry_isr + json_dualos_sort_entry_label + json_dualos_list
    dualos_json_string = json.dumps(json_dualos_list, indent='\t')
    dualos_json_file = rtos_json_file.replace(DUALOS_RTOS_FILE_PREFIX, '')

    with open(dualos_json_file, "w") as f_json:
        f_json.write(dualos_json_string)

def main(argv):
    is_base64 = False
    log_file = ''
    log_base64_decode_file = ''
    json_output_file = ''
    rtos_json_file = ''
    linux_json_file = ''

    try:
        opts, args = getopt.getopt(argv,"hi:",["ifile="])
    except getopt.GetoptError:
        print ('{} -i <log_file>'.format(sys.argv[0]))
        sys.exit(1)
    for opt, arg in opts:
        if opt == '-h':
            print ('{} -i <log_file>'.format(sys.argv[0]))
            sys.exit()
        elif opt in ("-i", "--ifile"):
            log_file = arg

    if not os.path.exists(log_file):
        print ('Cannot access {}: No such file'.format(log_file))
        sys.exit(1)

    # try base64 decode
    log_base64_decode_file = log_file + ".tmp"
    if os.path.exists(log_base64_decode_file):
        os.remove(log_base64_decode_file)
    with open(log_file) as f_log:
        try:
            log_base64 = f_log.read()
        except:
            print (bcolors.BLUE + 'Not base64 format...' + bcolors.RESET)
        else:
            try:
                decoded = base64.b64decode(log_base64)
            except:
                print (bcolors.BLUE + 'Not base64 format...' + bcolors.RESET)
            else:
                is_base64 = True
                print ('Output base64 decode file:' + bcolors.BLUE + '{}'.format(log_base64_decode_file) + bcolors.RESET)
                open(log_base64_decode_file, "wb").write(decoded)
                json_output_file = log_file + JSON_FILENAME_EXTENSION
                if os.path.exists(json_output_file):
                    os.remove(json_output_file)

                log_file = log_base64_decode_file
                add_to_temp_file_list(log_base64_decode_file)

    # parse log file to json
    if is_base64 == True:
        parse_log_file(log_file, json_output_file)
    else:
        pre_process_log_file(log_file)
        for file in log_file_list:
            if file.startswith(DUALOS_RTOS_FILE_PREFIX):
                print("===== Parse RTOS log start  =====")
            elif file.startswith(DUALOS_LINUX_FILE_PREFIX):
                print("===== Parse Linux log start =====")
            json_output_file = file + JSON_FILENAME_EXTENSION
            if os.path.exists(json_output_file):
                os.remove(json_output_file)

            parse_log_file(file, json_output_file)

            if file.startswith(DUALOS_RTOS_FILE_PREFIX):
                print("===== Parse RTOS log end    =====")
                rtos_json_file = json_output_file
                add_to_temp_file_list(json_output_file)
            elif file.startswith(DUALOS_LINUX_FILE_PREFIX):
                print("===== Parse Linux log end   =====")
                linux_json_file = json_output_file
                add_to_temp_file_list(json_output_file)

        # more than one json file, merge it
        if len(log_file_list) > 1:
            print("Merge DualOS log ...")
            merge_json_file(rtos_json_file, linux_json_file)

    if not g_debug_enable:
        print("Remove temp files ...")
        remove_temp_files()

if __name__ == "__main__":
   main(sys.argv[1:])
   print ('SWLA parser done')