# -*- coding: utf-8 -*-
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

import os
import sys
import math
# ---------------------------------------------------------------------------------------------------------------
# 构造一个简单的svg生成类，仅支持集中简单的图形元素，如矩形、圆形、文本等。
class Svg:
    def __init__(self):
        self.elements = []

    def add(self, element):
        self.elements.append(element)

    def save(self, filename):
        content = str(self)
        with open(filename, 'w') as f:
            f.write(content)

    def _header(self):
        return '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" id="sstar_load_layout">\n'

    def _footer(self):
        # 显示详细信息的js脚本。获取 sstar_load_layout中所有class为layout_graph的元素的detail_info属性，显示在detail_info元素中。
        # 并不支持多行文本
        js = '''
<script type="text/javascript">
const svg = document.getElementById("sstar_load_layout");
const detail_info = document.getElementById("detail_info");

svg.addEventListener("mouseover", function(event) {
    const target = event.target;
    if(detail_info) {
        if (target.classList.contains("layout_graph")) {
            detail_info.textContent = target.getAttribute("detail_info");
        }
    }
});

svg.addEventListener("mouseout", function(event) {
    const target = event.target;
    if(detail_info) {
        if (target.classList.contains("layout_graph")) {
            // detail_info.textContent = "";
        }
    }
});
</script>
        '''
        return '{}\n</svg>\n'.format(js)

    def __str__(self):
        return self._header() + "".join([str(_) for _ in self.elements]) + self._footer()

class Rect:
    def __init__(self, insert, size=(200,100), fill='black', args = {}):
        self.x = insert[0]
        self.y = insert[1]
        self.width = size[0]
        self.height = size[1]
        self.fill = fill
        self.detail_info = "#".join(["{:8s}: {}".format(k, v) for k,v in args.items()])

    def __str__(self):
        out = ''
        for k,v in self.__dict__.items():
            if k.startswith('_') or callable(v) or v == None:
                continue
            if isinstance(v, float):
                out += '{}="{:.3f}" '.format(k, v)
            else:
                out += '{}="{}" '.format(k, v)

        return '<rect class="layout_graph" {} rx="2" ry="2"/>\n'.format(out)


class Circle:
    def __init__(self, center=(100,100), r=50, fill='black'):
        self.cx = center[0]
        self.cy = center[1]
        self.r = r
        self.fill = fill

    def __str__(self):
        out = ''
        for k,v in self.__dict__.items():
            if k.startswith('_') or callable(v) or v == None:
                continue
            out += '{}="{}" '.format(k, v)
        return '<circle {}/>\n'.format(out)


class Text:
    def __init__(self, content, insert=(200,0), fill="black", font_size=16, id=None):
        self.x = insert[0]
        self.y = insert[1]
        self.fill = fill
        self.content = content
        self.font_size = font_size
        if id:
            self.id = id

    def __str__(self):
        out = ''
        for k,v in self.__dict__.items():
            if k.startswith('_') or callable(v) or v == None or k == 'content':
                continue
            k = k.replace('_', '-')
            if isinstance(v, float):
                out += '{}="{:.3f}" '.format(k, v)
            else:
                out += '{}="{}" '.format(k, v)
        return '<text {}>{}</text>\n'.format(out, self.content)
# ---------------------------------------------------------------------------------------------------------------
DRAM_BASE_ADDR = 0X20000000
#从前往后依次使用颜色，保证多次运行颜色不会变动
COLOR_IDX = -1
def color():
    #颜色从暖色到冷色依次排列
    value = [
            "#FF4500",  # 橙红色
            "#FF8C00",  # 深橙色
            "#FFDA44",  # 明亮金色
            "#FF69B4",  # 热粉色
            "#FF1493",  # 深粉色
            "#FFB6C1",  # 亮粉色
            "#FF7F50",  # 珊瑚色
            "#32CD32",  # 鲜绿色
            "#ADFF2F",  # 黄绿色
            "#00FA9A",  # 中春绿色
            "#00BFFF",  # 深天蓝
            "#1E90FF",  # 道奇蓝
            "#00CED1",  # 暗青色
            "#7B68EE"   # 中蓝色
    ]
    global COLOR_IDX
    COLOR_IDX = (COLOR_IDX + 1) % len(value)
    return value[COLOR_IDX]

class Drawer:
    def __init__(self):
        self.graph_width = 100 #渲染图形layout的宽度
        self.graph_stack = 0 #渲染图形layout的栈位置，方便元素的堆叠
        self.text_h = 8 #文本的高度，实际测试调整而来，调整font_size这里也要调整
        self.margin = 5 #每个元素之间的间隔，调大可以让元素更分散
        self.addr_text_w = 100 #地址test文字的宽度这里设置成100，可以根据需要调整
        self.graph_x = self.margin + self.addr_text_w #渲染图形layout的起始x坐标
        self.svg = Svg()

        self._draw_head() #绘制头部预留内存

    def add_section(self, section):
        self.draw_section(section["name"], section)

    def _draw_head(self):
        # 0x00000000->0x20000000 #把预留的空白区域用矩形表示
        x = self.graph_x
        y = self.graph_stack
        h = 30
        rect = Rect(insert=(x,y), size=(self.graph_width, h), fill='grey')
        self.layout_top = DRAM_BASE_ADDR
        self.svg.add(rect)
        self.layout_end = self.layout_top
        self.graph_stack = y + h

        x = self.margin
        y += h
        text = Text(content=hex(self.layout_top), insert=(x,y), fill='black')  #终止地址文字
        self.svg.add(text)

    #调整到人类可读的格式
    def h_size(self, size):
        if size > 1024*1024*1024:
            return "{:.3f}G".format(size / 1024 / 1024 / 1024)
        elif size > 1024*1024:
            return "{:.3f}M".format(size / 1024 / 1024)
        elif size > 1024:
            return "{:.3f}K".format(size / 1024)
        else:
            return "{}Bytes".format(size)

    def draw_section(self, name, section):
        section_name = os.path.basename(name)
        start = self.graph_stack
        end = start + section['normalized_size']
        x = self.graph_x
        y = start
        h = end - start
        detail_info ={
            "Memory": "{} [{} - {}] = {} = {} = {}".format(
                "UNUSED" if section['name'] == 'UNUSED' else "USED",
                hex(section['end']).upper(), hex(section['start']).upper(),
                section['size'], hex(section['size']).upper(),
                self.h_size(section['size']),)
        }
        rect = Rect(insert=(x,y), size=(self.graph_width, h), fill=section['color'], args=detail_info)
        self.svg.add(rect)
        self.graph_stack = end

        if section['name'] != 'UNUSED': #只有使用了的内存才显示地址信息
            illegal = section.get('illegal', False)
            fill = "red" if illegal else "black" #异常时显示红色
            x = self.margin
            y = start + self.text_h + self.text_h
            text = Text(content=hex(section['start']).upper(), insert=(x,y), fill=fill) #起始地址文字
            self.svg.add(text)

            x = self.margin
            y = end
            text = Text(content=hex(section['end']).upper(), insert=(x,y), fill=fill)  #终止地址文字
            self.svg.add(text)

            x = self.graph_x + self.graph_width + self.margin
            y = (start + end) / 2 + self.text_h
            text = Text(content=section_name, insert=(x,y), fill='black') #名称
            self.svg.add(text)

        #对于内存的大小如果是预留未使用的内存就先灰色显示
        fill = 'black' if section['name'] != 'UNUSED' else '#bbbbbb'
        x = self.graph_x + self.margin
        y = (start + end) / 2 + self.text_h
        text = Text(content=self.h_size(section['size']), insert=(x,y), fill=fill) #大小
        self.svg.add(text)

    #尾部区域信息
    def _draw_tail(self):
        #显示后面未被使用内存信息
        x = self.graph_x
        y = self.graph_stack
        h = 30
        rect = Rect(insert=(x,y), size=(self.graph_width, h), fill='grey')
        self.svg.add(rect)
        self.graph_stack += h

        #图标标题 位于下方
        x = self.margin
        y = self.graph_stack + self.margin * 5 + self.text_h * 2
        text = Text(content="SigmaStar Images Load Address Layout", insert=(x,y), fill='black', font_size=24)
        self.graph_stack = y + self.margin
        self.svg.add(text)

        #尾部的详细信息字段用于展示详细信息
        x = self.margin
        y = self.graph_stack + self.margin * 5 + self.text_h * 2
        text = Text(content="", insert=(x,y), fill='black', id="detail_info")
        self.graph_stack = y + self.margin
        self.svg.add(text)


    def draw(self, filename):
        self._draw_tail()
        self.svg.save(filename)

# 对数据进行归一化处理：
# 因为布局如果按照线性进行处理会导致一些无意义的空洞区域占据画布的大部分面积，
# 1. 使用对数处理过大的内存区域以获得较好的线性度
# 2. 归一化使得每个内存区域的大小缩放在一个合理的范围内
# 这样可以使得画布的布局更加紧凑。
def normalized(layout, min_display_size=32, max_display_size=100):
    def log(number):
        return math.log(number)
    memory_sizes = [log(section['size']) for section in layout]
    max_memory_size = max(memory_sizes)
    min_memory_size = min(memory_sizes)
    for idx,value in enumerate(memory_sizes):
        offset = value - min_memory_size
        size = max_memory_size - min_memory_size
        normalized_size = max(min_display_size, min(offset / size * max_display_size, max_display_size))
        layout[idx]["normalized_size"] = normalized_size
    return layout

#检查地址是否有重叠的异常
def check_illegal(layout):
    for i in range(len(layout)):
        for j in range(i + 1, len(layout)):
            if (layout[i]["end"] <= layout[j]["start"] or layout[i]["start"] >= layout[j]["end"]):
                continue
            layout[i]["illegal"] = True
            layout[j]["illegal"] = True
    return layout

def main():
    if len(sys.argv) < 2:
        print('Usage: python3 {} <load_layout_file> [svg_filename]', sys.argv[0])
        print('\tExample: python3 {} load_layout.txt example.svg'.format(sys.argv[0]))
        print('\tExample: python3 {} load_layout.txt # will save to load_layout.svg'.format(sys.argv[0]))
        exit(1)
    load_layout_file = sys.argv[1]
    svg_filename = sys.argv[2] if len(sys.argv) == 3 else os.path.splitext(load_layout_file)[0] + '.svg'
    # 读取DRAM布局文件
    used_mempry = []
    unused_memory = []
    """ layout文件格式:
    path/of/file1
    0xstart-0xend
    path/of/file2
    0xstart-0xend
    """
    key = None
    with open(load_layout_file, 'r') as f:
        while True:
            line = f.readline()
            if not line:
                break
            line = line.strip()
            if not line: #空行跳过
                continue
            if key == None:
                key = line
            else:
                value = line
                (start,end) = value.split('-')
                start = int(start.strip(), base=16)
                end = int(end.strip(), base=16)
                illegal = False
                if start >= end:
                    end = start + 1
                    illegal = True
                used_mempry.append({"name": key,'start': start, 'end': end, 'size': end - start, "illegal": illegal})
                key = None #开始处理下一个section

    used_mempry = sorted(used_mempry, key=lambda x: x['start'])
    used_mempry = check_illegal(used_mempry)
    # 计算未使用的内存区域
    for idx, section in enumerate(used_mempry[:-1]):
        if section['end'] < used_mempry[idx+1]['start']:
            size = used_mempry[idx + 1]['start'] - section['end']
            unused_memory.append({
                'name': 'UNUSED',
                'start': section['end'],
                'end': used_mempry[idx+1]['start'],
                'size': size,
                "color": '#dddddd'
                })
    #手动加一下最开头的那段未使用的内存
    if used_mempry[0]['start'] > DRAM_BASE_ADDR:
        unused_memory.append({
            "name": 'UNUSED',
            'start': DRAM_BASE_ADDR,
            'end': used_mempry[0]['start'],
            'size': used_mempry[0]['start'] - DRAM_BASE_ADDR,
            "color": '#dddddd'
            })
    #从颜色list里面按照size降序排列依次给予一个颜色
    used_mempry = sorted(used_mempry, key=lambda x: x['size'], reverse=True)
    for section in used_mempry:
        section['color'] = color()
    used_mempry = sorted(used_mempry, key=lambda x: x['start'])
    unused_memory = sorted(unused_memory, key=lambda x: x['start'])

    #分别进行归一化处理，分开处理的原因是因为内存大部分区域都是未使用的，如果一起归一化会使得未使用的区域占据大部分面积
    #分开处理可以通过调整其各自显示的min->max的区间范围，使得已使用的区域分配到更多面积
    used_mempry = normalized(used_mempry)
    unused_memory = normalized(unused_memory, 16, 100)

    memory = sorted(used_mempry + unused_memory, key=lambda x: x['start'])
    # 生成SVG文件
    dwg = Drawer()
    for section in memory:
        dwg.add_section(section)
    dwg.draw(svg_filename)
    return 0


if __name__ == '__main__':
    #因为这是一个附加的福利型功能，所以我们不希望其在运行异常时散播异常至调用者，但我们还是需要捕获异常并打印出来以便排查问题。
    #当然我们还是会返回一个非0的返回值以便于调用者可以自行选择是否捕获并处理
    try:
        ret = main()
    except Exception as e:
        print("{} gen graph failed: {}\n".format(sys.argv[0], e))
        exit(0)
    exit(ret)
