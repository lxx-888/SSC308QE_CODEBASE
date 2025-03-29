import sys
import re
import io
import pandas as pd
undefine = "UNDEFINE"

class AddressBuilder():
    def __init__(self, bank,high_address,low_address,loc_msb,loc_lsb):
        self.bank = bank
        self.high_address = high_address
        self.low_address = low_address
        self.loc_msb = loc_msb
        self.loc_lsb = loc_lsb

    def description(self):
        return "{{.bank={},.high_address={},.low_address={},.loc_msb={},.loc_lsb={}}}".format(self.bank,self.high_address,self.low_address,self.loc_msb,self.loc_lsb)

class CategoryBuilder():
    def __init__(self,ip,category,otp_addr,ipctrl_addr,def_val=undefine,activateefuse=AddressBuilder(undefine,undefine,undefine,undefine,undefine)):
        self.ip = ip
        self.category = category
        self.otp_addr = otp_addr
        self.ipctrl_addr = ipctrl_addr
        self.def_val = def_val
        self.activateefuse = activateefuse

    def description(self):
        return "{{\n    .ip=\"{}\",\n    .category=\"{}\",\n    .otp_addr={},\n    .ipctrl_addr={},\n    .def_val={},\n    .activateefuse={}\n}}".format(self.ip,self.category,self.otp_addr.description(),self.ipctrl_addr.description(),self.def_val,self.activateefuse.description())

class SpecialNoteBuilder(AddressBuilder):
    def __init__(self,ip,category, bank,high_address,low_address,loc_msb,loc_lsb,def_val):
        super().__init__(bank,high_address,low_address,loc_msb,loc_lsb)
        self.ip = ip
        self.category = category
        self.def_val = def_val
    def description(self):
        return "{{\n    .ip=\"{}\",\n    .category=\"{}\",\n    .bank={},\n    .high_address={},\n    .low_address={},\n    .loc_msb={},\n    .loc_lsb={},\n    .def_val={}\n}}".format(self.ip,self.category,self.bank,self.high_address,self.low_address,self.loc_msb,self.loc_lsb,self.def_val)

class TryIcBuilder(AddressBuilder):
    def __init__(self,ip,category, bank,high_address,low_address,loc_msb,loc_lsb,def_val):
        super().__init__(bank,high_address,low_address,loc_msb,loc_lsb)
        self.ip = ip
        self.category = category
        self.def_val = def_val
    def description(self):
        return "{{\n    .ip=\"{}\",\n    .category=\"{}\",\n    .bank={},\n    .high_address={},\n    .low_address={},\n    .loc_msb={},\n    .loc_lsb={},\n    .def_val={}\n}}".format(self.ip,self.category,self.bank,self.high_address,self.low_address,self.loc_msb,self.loc_lsb,self.def_val)

def get_bitaddress(offset,bit):
    return offset*16+bit

def ipctrl_parse(ipctrl):
    ipctrl_lst = []
    bank_pattern = r'(0[xX][A-Fa-f0-9]+)_'
    lst = ipctrl.split('\n')
    for text in lst:
        bank_match = re.search(bank_pattern, text)

        offset_pattern = r"\_([A-Fa-f0-9]+)\["
        offset_match = re.search(offset_pattern, text)

        msb_lsb_pattern=r"\[([0-9]+)\]"
        msb_lsb_match = re.search(msb_lsb_pattern, text)

        msb_pattern = r"\[([0-9]+):"
        msb_match = re.search(msb_pattern, text)

        lsb_pattern = r":([0-9]+)\]"
        lsb_match = re.search(lsb_pattern, text)

        val_pattern_2 = r"=[A-Fa-f0-9]'b([0-1]+).*"
        val_pattern_16 = r"=[A-Fa-f0-9]'h([A-Fa-f0-9]+).*"
        val_match_2 = re.search(val_pattern_2, text)
        val_match_16 = re.search(val_pattern_16, text)
        # if(val_match_2):
        #     print("val_match_2")
        # if(val_match_16):
        #     print("val_match_16")
        if bank_match and offset_match and (msb_lsb_match or (msb_match and lsb_match)) and (val_match_2 or val_match_16):
            bank_data = bank_match.group(1)
            offset_data = offset_match.group(1)
            offset_data = '0x'+offset_data
            if(msb_match and lsb_match):
                msb_data = msb_match.group(1)
                lsb_data = lsb_match.group(1)
            else:
                msb_data = msb_lsb_match.group(1)
                lsb_data = msb_lsb_match.group(1)
            if(val_match_2):
                val_data = val_match_2.group(1)
                val_data = '0b'+val_data
            elif(val_match_16):
                val_data = val_match_16.group(1)
                val_data = '0x'+val_data
            else:
                print('can not find val_data,exit')
                exit()
            ipctrl_lst.append((AddressBuilder(bank_data,offset_data,offset_data,msb_data,lsb_data),val_data))
        else:
            print(text,"parse fail,please check format")
            exit()

    return ipctrl_lst

args = sys.argv
print(args)
file_name = args[1]

# read excel
AutoLoad_TryIC = pd.read_excel(file_name,sheet_name='Analog_Config',dtype=str,skiprows=[0],
usecols=[
0,          # 'IP',
1,          # 'Category',
15,         # 'Bank',
16,         # 'High \nAddress\n(Hex)',
17,         # 'low\nAddress\n(Hex)',
18,         # 'Loc MSB',
19,         # 'Loc LSB',
20,21,22,23,# 'Try A','Try B','Try C','Try D',
24,         # '1. Format: 0x”bank”_”16-bit-address”[bits]\n\
            #     e.g.: 0x1025_5B[12:8]\n\
            #            (a) 1025 => bank\n\
            #            (b) 5B => 16-bit address\n\
            # 2. Key = value:\n\
            #      e.g.: 0x103D_08[15:12] = 4\'b0000',
25          #'Special Note'])
])

AutoLoad_TryIC = AutoLoad_TryIC.rename(columns={AutoLoad_TryIC.columns[0]: 'IP', AutoLoad_TryIC.columns[1]: 'Category'})
AutoLoad_TryIC[['IP','Bank']]=AutoLoad_TryIC[['IP','Bank']].fillna(method='ffill')

print(AutoLoad_TryIC)
rows, cols = AutoLoad_TryIC.shape
category_list = []
specialnote_list = []
tryic_a_list = []
tryic_b_list = []
tryic_c_list = []
tryic_d_list = []

for i in range(rows-1):
    ip = AutoLoad_TryIC.loc[i,'IP'].replace('\n',' ')
    category = AutoLoad_TryIC.loc[i,'Category']
    bank = AutoLoad_TryIC.loc[i,'Bank']
    high_address = AutoLoad_TryIC.loc[i,'High \nAddress\n(Hex)']
    low_address = AutoLoad_TryIC.loc[i,'low\nAddress\n(Hex)']
    loc_msb = AutoLoad_TryIC.loc[i,'Loc MSB']
    loc_lsb = AutoLoad_TryIC.loc[i,'Loc LSB']
    tryic_a = AutoLoad_TryIC.loc[i,'Try A']
    tryic_b = AutoLoad_TryIC.loc[i,'Try B']
    tryic_c = AutoLoad_TryIC.loc[i,'Try C']
    tryic_d = AutoLoad_TryIC.loc[i,'Try D']

    ip_control_register = AutoLoad_TryIC.loc[i,\
'1. Format: 0x”bank”_”16-bit-address”[bits]\n\
    e.g.: 0x1025_5B[12:8]\n\
           (a) 1025 => bank\n\
           (b) 5B => 16-bit address\n\
2. Key = value:\n\
     e.g.: 0x103D_08[15:12] = 4\'b0000']
    special_note = AutoLoad_TryIC.loc[i,'Special Note']
    high_address = '0x'+high_address
    low_address = '0x'+low_address
    tryic_a = '0b'+tryic_a
    tryic_b = '0b'+tryic_b
    tryic_c = '0b'+tryic_c
    tryic_d = '0b'+tryic_d

#ignore DDR
    if(ip == 'DDR_ATOP (P0)'):
        continue

#IPCTRL action1
    if(ip_control_register == 'flag only' and "Activateefuse" not in category):
        print("flag do not contain Activateefuse category=",category)

#IPCTRL action2
    elif(ip_control_register == 'flag only' and "Activateefuse" in category):
        print("flag contain Activateefuse category=",category)
        for category_ins in reversed(category_list):
            if(ip==category_ins.ip):
                category_ins.activateefuse=AddressBuilder(bank,high_address,low_address,loc_msb,loc_lsb)
            else:
                break
        #add flag to these categories

#IPCTRL action3
    else:
        ipctrl_list = ipctrl_parse(ip_control_register)
        for ipctrl,def_val in ipctrl_list:
            #build ipctrl_list
            #category_ins = CategoryBuilder(category,AddressBuilder(bank,high_address,low_address,loc_msb,loc_lsb))
            category_ins = CategoryBuilder(ip,category,AddressBuilder(bank,high_address,low_address,loc_msb,loc_lsb),ipctrl,def_val)
            category_list.append(category_ins)

#special node action
    if(not pd.isna(special_note)):
        print(ip)
        snparse_list = ipctrl_parse(special_note)
        for sn,def_val in snparse_list:
            specialnote_list.append(SpecialNoteBuilder(ip,category,sn.bank,sn.high_address,sn.low_address,sn.loc_msb,sn.loc_lsb,def_val))
#TRYIC action
    tryic_a_list.append(TryIcBuilder(ip,category,bank,high_address,low_address,loc_msb,loc_lsb,tryic_a))
    tryic_b_list.append(TryIcBuilder(ip,category,bank,high_address,low_address,loc_msb,loc_lsb,tryic_b))
    tryic_c_list.append(TryIcBuilder(ip,category,bank,high_address,low_address,loc_msb,loc_lsb,tryic_c))
    tryic_d_list.append(TryIcBuilder(ip,category,bank,high_address,low_address,loc_msb,loc_lsb,tryic_d))



# open chkotp_data.c as std output
with open('chkotp_data.c', 'w') as f:
    old_stdout = sys.stdout
    sys.stdout = f

    print("#include \"include/chkotp.h\"")

    print("struct Category category_arr[CATEGORY_NUM] = {")
    for i,c in enumerate(category_list):
        if i == len(category_list)-1:
            print(c.description())
            print('};')
        else:
            print(c.description()+',')
    print("")
    print("struct SpecialNote specialnote_arr[SPECIALNOTE_NUM] = {")
    for i,s in enumerate(specialnote_list):
        if i == len(specialnote_list)-1:
            print("    "+s.description())
            print('};')
        else:
            print("    "+s.description()+',')
    print("")
    print("struct TryIc tryic_a_list[] = {")
    for i,t in enumerate(tryic_a_list):
        if i == len(tryic_a_list)-1:
            print("    "+t.description())
            print('};')
        else:
            print(t.description()+',')
    print("struct TryIc tryic_b_list[] = {")
    for i,t in enumerate(tryic_b_list):
        if i == len(tryic_b_list)-1:
            print("    "+t.description())
            print('};')
        else:
            print(t.description()+',')
    print("struct TryIc tryic_c_list[] = {")
    for i,t in enumerate(tryic_c_list):
        if i == len(tryic_c_list)-1:
            print("    "+t.description())
            print('};')
        else:
            print(t.description()+',')
    print("struct TryIc tryic_d_list[] = {")
    for i,t in enumerate(tryic_d_list):
        if i == len(tryic_d_list)-1:
            print("    "+t.description())
            print('};')
        else:
            print(t.description()+',')
    sys.stdout = old_stdout

# open chkotp_data.h as std output
with open('chkotp.h', 'w') as f:
    old_stdout = sys.stdout
    sys.stdout = f

    print("#ifndef _CHKOTP_H")
    print("#define _CHKOTP_H")

    print("#define U8  unsigned char      // 1 byte")
    print("#define U16 unsigned short     // 2 byte")
    print("#define U32 unsigned int       // 4 byte")
    print("#define U64 unsigned long long // 8 byte")

    # save date size & list num as macro
    print("#define","CATEGORY_NUM",len(category_list))
    print("#define","SPECIALNOTE_NUM",len(specialnote_list))
    print("#define","TRYIC_A_NUM",len(tryic_a_list))
    print("#define","TRYIC_B_NUM",len(tryic_b_list))
    print("#define","TRYIC_C_NUM",len(tryic_c_list))
    print("#define","TRYIC_D_NUM",len(tryic_d_list))

    print("#define UNDEFINE -1")

    # structure defination
    print("struct Address")
    print("{")
    print("    U16 bank;")
    print("    U16 high_address;")
    print("    U16 low_address;")
    print("    U16 loc_msb;")
    print("    U16 loc_lsb;")
    print("};")

    print("struct Category")
    print("{")
    print("    char ip[40];")
    print("    char category[30];")
    print("    struct Address otp_addr;")
    print("    struct Address ipctrl_addr;")
    print("    U16 def_val;")
    print("    struct Address activateefuse;")
    print("};")

    print("struct SpecialNote")
    print("{")
    print("char ip[40];")
    print("char category[30];")
    print("    U16 bank;")
    print("    U16 high_address;")
    print("    U16 low_address;")
    print("    U16 loc_msb;")
    print("    U16 loc_lsb;")
    print("    U16 def_val;")
    print("};")

    print("struct TryIc")
    print("{")
    print("    char ip[40];")
    print("    char category[30];")
    print("    U16 bank;")
    print("    U16 high_address;")
    print("    U16 low_address;")
    print("    U16 loc_msb;")
    print("    U16 loc_lsb;")
    print("    U16 def_val;")
    print("};")
    print("#endif")
    print("")

#endif
    sys.stdout = old_stdout



