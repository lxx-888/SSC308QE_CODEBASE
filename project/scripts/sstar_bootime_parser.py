#-*- coding: utf-8 -*-
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
import sys,os
import enum
import re
from typing import Dict, List
import json
from datetime import datetime

ID = -1
def get_id():
    global ID
    ID += 1
    return ID
# google trace viewer
class TraceViewer:
    def __init__(self, tag:str="SigmaStar boot time"):
        self.records: list = []
        self.tag = tag

    def rgb(r, g, b):
        pass
    # 颜色定义, 装一个colot highlight 插件可以看到颜色
    # from https://github.com/catapult-project/catapult/blob/master/tracing/tracing/base/color_scheme.html
    color = {
        "thread_state_uninterruptible" : rgb(182, 125, 143),
        "thread_state_iowait" : rgb(255, 140, 0),
        "thread_state_running" : rgb(126, 200, 148),
        "thread_state_runnable" : rgb(133, 160, 210),
        "thread_state_sleeping" : rgb(240, 240, 240),
        "thread_state_unknown" : rgb(199, 155, 125),

        "background_memory_dump" : rgb(0, 180, 180),
        "light_memory_dump" : rgb(0, 0, 180),
        "detailed_memory_dump" : rgb(180, 0, 180),

        "vsync_highlight_color" : rgb(0, 0, 255),
        "generic_work" : rgb(125, 125, 125),

        "good" : rgb(0, 125, 0),
        "bad" : rgb(180, 125, 0),
        "terrible" : rgb(180, 0, 0),

        "black" : rgb(0, 0, 0),
        "grey" : rgb(221, 221, 221),
        "white" : rgb(255, 255, 255),
        "yellow" : rgb(255, 255, 0),
        "olive" : rgb(100, 100, 0),

        "rail_response" : rgb(67, 135, 253),
        "rail_animation" : rgb(244, 74, 63),
        "rail_idle" : rgb(238, 142, 0),
        "rail_load" : rgb(13, 168, 97),
        "startup" : rgb(230, 230, 0),

        "heap_dump_stack_frame" : rgb(128, 128, 128),
        "heap_dump_object_type" : rgb(0, 0, 255),
        "heap_dump_child_node_arrow" : rgb(204, 102, 0),

        "cq_build_running" : rgb(255, 255, 119),
        "cq_build_passed" : rgb(153, 238, 102),
        "cq_build_failed" : rgb(238, 136, 136),
        "cq_build_abandoned" : rgb(187, 187, 187),

        "cq_build_attempt_runnig" : rgb(222, 222, 75),
        "cq_build_attempt_passed" : rgb(103, 218, 35),
        "cq_build_attempt_failed" : rgb(197, 81, 81)
    }

    def trace_event(self, thread:str, name:str, time_us:str, color:str=None, args=''):
        node = {
            "name": name,
            "ph": "i",
            "pid": self.tag,
            "tid": thread,
            "ts": int(time_us),
        }
        if color and color in self.color:
            node["cname"] = color
        self.records.append(node)
    def snapshot_event(self, thread:str, name:str, time_us:str, color:str=None, args=''):
        node = {
            "name": name,
            "id": name,
            "ph": "O",
            "pid": self.tag,
            "tid": thread,
            "ts": int(time_us),
            "args": args
        }
        if color and color in self.color:
            node["cname"] = color
        self.records.append(node)
    def flow_event(self, thread:str, name:str, time_us:str, time_end_us:str, id = None, color:str=None, args=''):
        if id is None:
            id = get_id()
        node = [{
            "name": name,
            "ph": "s",
            "pid": self.tag,
            "tid": thread,
            "ts": int(time_us),
            "cat": "flow",
            "id": id,
            "args": args
        },{
            "name": name,
            "ph": "f",
            "pid": self.tag,
            "tid": thread,
            "ts": int(time_end_us),
            "cat": "flow",
            "id": id,
            "bp": "e",
            "args": args,
        },
        ]
        if color and color in self.color:
            for item in node:
                item["cname"] = color
        self.records += node
    def find_snapshot_event(self, thread:str, name:str):
        for item in self.records:
            if item["ph"] == "O" and item["tid"] == thread and item["name"] == name:
                return True
        return False
    def process_event(self, thread:str, name:str, time_us:str, time_end_us:str, color:str=None, args=''):
        node = {
            "name": name,
            "ph": "X", # or: B E
            "pid": self.tag,
            "tid": thread,
            "ts": int(time_us),
            "dur": (int(time_end_us) - int(time_us)),
        }
        for item in self.records:
            if "cname" in item: # 不比较color
                item = dict(item)
                del item["cname"]
            if item == node:
                return
        #时间很短的直接设置为灰色
        if node["dur"] < 1000 and color is None:
            color = "grey"
        if color and color in self.color:
            node["cname"] = color
        self.records.append(node)
    def is_in_process(self, thread:str, time_us:str, time_end_us:str):
        for item in self.records:
            start = item["ts"]
            end = start + item.get("dur", 0)
            if thread == item["tid"] and start <= int(time_us) and end >= int(time_end_us):
                return True
    def __repr__ (self):
        return self.__str__()
    def __str__(self):
        self.records.sort(key=lambda x: x["ts"])
        str_data = json.dumps(self.records, indent=2)
        return str_data

class TraceViewerList:
    def __init__(self):
        self.tracers: List[TraceViewer] = []
    def append(self, node:TraceViewer):
        self.tracers.append(node)
    def __str__(self):
        members = []
        for item in self.tracers:
            members = members + [_ for _ in item.records]
        json_data = {
            "traceEvents":members,
            "displayTimeUnit": "ms",
            "systemTraceEvents": "",
            "otherData": {
                "version": "v1.0",
                "copyright": "SigmaStar",
                "product": "BootTimeTraceData"
            },
        }
        # print(json_data)
        str_data = json.dumps(json_data, indent=2)
        return str_data
    def generate_trace_viewer_json(self, output_file:str):
        with open(output_file, "w") as f:
            f.write(self.__str__())

class RecordStatus(enum.Enum):
    Idle = 0
    StartSearch = 1
    Processing = 2
    Complete = 3

class RecordInfo:
    """
    RecordInfo: 探针点的信息
    """
    def __init__(self, index:int, time:int, diff:int, tag:str, mark:int=None):
        """
        index: 记录的索引(位于当前记录块中的索引)
        time: 记录的时间戳(自启动而来的us数值)
        diff: 记录的时间差(和上一个记录的时间差)
        tag: 记录的标签(方便区分不同阶段)
        mark: 记录的标记(一般就是代码的行号)
        """
        self.index = index
        self.time = time
        self.diff = diff
        self.tag = tag.strip()
        self.mark = mark
    def __repr__ (self):
        return self.__str__() + "\n"
    def __str__(self):
        return "RecordInfo(index:{self.index}, time:{self.time}us, diff:{self.diff}, tag: {self.tag} , mark:{self.mark})".format(self=self)

class RecordStage:
    """
    记录块，存储某一阶段的所有记录
    """
    def __init__(self, stage_name:str, trace_number:int, log_line:int):
        self.stage_name = stage_name #阶段名称
        self.trace_number = trace_number #记录个数
        self.log_line = log_line #位于log中的行号
        # print(f"create RecordStage({stage_name}, {trace_number}, {log_line})")
        self.records: List[RecordInfo] = []

    def append(self, record:RecordInfo):
        self.records.append(record)

    def __repr__ (self):
        return self.__str__() + "\n"
    def __str__(self):
        return "RecordStage(stage_name={self.stage_name}, start_line={self.log_line}, trace_number={self.trace_number}, records=\n{self.records})".format(self=self)

class RecordBoot:
    """
    记录一次启动的所有阶段
    """
    def __init__(self, file:str):
        self.file = file
        self.log_line = -1
        self.stages: Dict[str, RecordStage] = {}

    def get(self, stage_name:str, false_value=None) -> RecordStage:
        return self.stages.get(stage_name, false_value)
    def __getitem__(self, stage_name:str) -> RecordStage:
        return self.stages[stage_name]

    def __setitem__(self, stage_name:str, stage:RecordStage):
        if self.log_line == -1:
            self.log_line = stage.log_line
        self.stages[stage_name] = stage

    def items(self):
        for key, value in self.stages.items():
            yield key, value

    def __repr__ (self):
        return self.__str__() + "\n"
    def __str__(self):
        return "RecordBoot(log_line={self.log_line}, stages=\n{self.stages})".format(self=self)

class BootTimeParser:
    def __init__(self, file:str):
        self.file = file
        self.boots: List[RecordBoot]  = []

    def parse(self) -> bool:
        status:RecordStatus = RecordStatus.Idle
        boot_once = None
        stage_records:RecordStage = None
        #遍历所有log行
        with open(self.file, "r", encoding="utf-8") as f:
            for idx,line in enumerate(f):
                if "Total cost" in line: #某一阶段的boot time记录结束
                    status = RecordStatus.Complete
                if "/msys/booting_time" in line:
                    status = RecordStatus.StartSearch
                    if boot_once:
                        self.boots.append(boot_once)
                    boot_once = RecordBoot(self.file) #记录一次启动的所有阶段

                if status == RecordStatus.Idle:
                    pass
                elif status == RecordStatus.StartSearch:
                    pattern = r"(\w+) \((\d+) records\)" # 匹配开始行 eg: IPL (13 records)
                    matches = re.match(pattern, line)
                    if matches:
                        # print(matches.groups())
                        stage_records = RecordStage(*matches.groups(), idx)
                        status = RecordStatus.Processing
                elif status == RecordStatus.Processing:
                    #匹配处理中间的行: 最后的mark字段可选
                    #000 time:    6710, diff:       0, IPL+, 0
                    #000 time:   57020, diff:       0, Rtos Premain Start
                    pattern = r"(\d+) time: +(\d+), diff: +(\d+),\s+([^,]+)(?:,\s+(\d+))?"
                    matches = re.findall(pattern, line)
                    if matches:
                        stage_records.append(RecordInfo(*matches[0]))
                elif status == RecordStatus.Complete: #当前记录块的所有 records 都处理完了
                    status = RecordStatus.StartSearch
                    boot_once[stage_records.stage_name] = stage_records
        if boot_once: #最后一个记录块的记录
            self.boots.append(boot_once)
        if len(self.boots) == 0:
            print("No boot time record found in file: " + self.file)
            return False
        return True

    def convert(self, draw_type:str="duration") -> List[TraceViewer]:
        def detect_process() -> bool: #检查区间匹配的事件
            if record not in refence: #已经被删除的记录算作已经处理过了
                return True
            start_flag = ("+", "start", "begin", "setup" , ">")
            end_flag = ("-", "end", "done", "exit", "<")
            ret = [len(_) for _ in start_flag if tag.lower().endswith(_)]
            if ret:
                prefix = tag[:-ret[0]]
                # print(f"tag: {tag}, ret: {ret}, prefix {prefix}")
                end = [_ for _ in stage.records if _.tag.lower().startswith(prefix.lower()) and _.tag.lower().endswith(end_flag)]
                if end:
                    end = end[0]
                    end_time = end.time
                    # print(f"add process event: {stage_name} {record.tag} -> {end.tag} {record.time}us -> {end_time}us")
                    tracer.process_event(stage_name,  "{} -> {}".format(record.tag, end.tag), record.time, end_time)
                    refence.remove(record) #移除已经匹配了的记录
                    refence.remove(end)
                    return True
            return False
        def detect_snapshot() -> bool: #检查重要的事件，TTFF and TTCL的flag
            filter = lambda pattern: bool(re.match(pattern, tag))
            if filter(r"^Vif_FirstUnmask$"):
                args = {"snapshot": "Vif is ready to receive sensor data"}
                tracer.snapshot_event(stage_name, "VIF Ready", record.time, color="cq_build_attempt_passed", args=args)
            elif filter(r"^VIF ch\d+ int 0$"):
                if tracer.find_snapshot_event(stage_name, "VIF Ready"):
                    args = {"snapshot": "This is the first frame after VIF ready to receive sensor data"}
                    tracer.snapshot_event(stage_name, "TTFF", record.time, color="cq_build_attempt_passed", args=args)
                else:
                    args = {"snapshot": "Vif is not reay to receive sensor data, wait for next frame"}
                    tracer.snapshot_event(stage_name, "TTFF", record.time, color="rail_animation", args=args)
                return True
            elif filter(r"^ramdisk_execute_command.*"):
                args = {"snapshot": "Linux is init done, its time to execute command"}
                tracer.snapshot_event(stage_name, "TTCL", record.time, color="cq_build_attempt_passed", args=args)
                return True
            return False

        now = datetime.now()
        tracer_list = []
        #搜索匹配的区间将其加到list里面去
        for boot_once in self.boots:
            time_string = now.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
            tracer = TraceViewer("{} [{} +{}] {}".format(time_string, boot_once.file, boot_once.log_line, get_id()))
            for stage_name, stage in boot_once.items():
                refence = stage.records[:] #复制一份记录用作标记
                for record in stage.records:
                    tag:str = record.tag
                    detect_snapshot()
                    if not detect_process() and draw_type == "instant":
                        tracer.trace_event(stage_name, record.tag, record.time)
            #每个区间都将其加到list里面去
            for stage_name, stage in boot_once.items():
                for idx in range(len(stage.records) - 1):
                    start = stage.records[idx]
                    end = stage.records[idx+1]
                    if draw_type == "duration":
                        tracer.process_event(stage_name,  "{start.tag} -> {end.tag}".format(start=start, end=end), start.time, end.time)
                    elif draw_type == "instant":
                        if not tracer.is_in_process(stage_name, start.time, end.time): #已经有一个完整的区间块的话就不必要加flow了
                            tracer.flow_event(stage_name,  "{start.tag} -> {end.tag}".format(start=start, end=end), start.time, end.time)
            tracer_list.append(tracer)
        return tracer_list

    def __repr__ (self):
        return self.__str__() + "\n"
    def __str__(self):
        return "BootTimeRecord(file={self.file}, records=\n{self.records})".format(self=self)



def usage():
    print("Usage: python {} <input_file0> [... <input_fileN>]  [<duration/instant]".format(sys.argv[0]))
    print("  <input_file>: the boot time log file cat on board")
    print("  [duration/、instant]: the type of draw, duration or instant, default is duration")
def main():
    if len(sys.argv) < 2:
        usage()
        return -1
    input_files = sys.argv[1:]
    if not input_files:
        usage()
        return -1
    if os.path.exists(sys.argv[-1]):
        #如果参数是文件名，则认为是duration模式
        draw_type = "duration"
    else:
        draw_type = input_files.pop()
    if draw_type not in ["instant", "duration"]:
        print("Invalid draw type: " + draw_type)
        usage()
        return -1
    viewer = TraceViewerList()
    for file in input_files:
        if not os.path.exists(file):
            print("Error:Input file not found: " + file)
            continue
        print("Parsing file: " + file)
        #解析log文件
        parser = BootTimeParser(file)
        if not parser.parse():
            return -1
        for trace in parser.convert(draw_type):
            viewer.append(trace)
    viewer.generate_trace_viewer_json("boot_time.json")
    return 0
if __name__ == '__main__':
    exit(main())
