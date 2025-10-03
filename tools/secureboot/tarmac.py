from __future__ import print_function
import sys
import re

symbols=[]
reg_name=[]
start=0
stack=[]
last_name='unknown'
last_offset=0
last_line=' '
def mysort(e):
    return e[1]
    
def read_sym(file):
    global symbols
    fp=open(file,'r')
    for line in fp:
        info=line.split()
        if len(info)<2:
            continue
        if (info[1]=='T') and (int(info[0],16)!=0):
            symbols+=[[info[2],int(info[0],16)-1]]
        if (info[1]=='D') and (int(info[0],16)!=0):
            symbols+=[[info[2],int(info[0],16)]]
    fp.close()

def read_regs(file):
    fp=open(file,'r')
    for line in fp:
        info=line.split()
        if len(info)<2:
            continue
        global reg_name
        reg_name+=[[info[0],int(info[1],16)]]        
    fp.close()
    
def get_sym(addr):
    global symbols
    for i in range(len(symbols)-1):
        if (symbols[i+1][1]>addr):
            return (symbols[i][0]+'+'+hex(addr-symbols[i][1]))

def get_reg(addr):
    for i in range(len(reg_name)):
        if (reg_name[i][1]==(addr&0xFFFFF000)):
            return (reg_name[i][0]+'+'+hex(addr-reg_name[i][1]))
    return (hex(addr))

if (len(sys.argv)<4):
    print("Usage: python tarmac.py <bootlader sym file> <reg names> <tarmac log> ")
    print("Check: \n\t1. Boot could jump to flash image.")
    print("\t2. Boot could jump IDLE loop if no flash image.")
    print("\t3. Boot could jump to RAM address if configured, time should be less than 1us")
    print("\t4. Flash register access is valid.")
    print("\t5. UART initialized OK, and simple write/read work, check with help command.")
    print("\t6. EFUSE access is valid.")
read_sym(sys.argv[1])
read_regs(sys.argv[2])
symbols.sort(key=mysort)
fp=open(sys.argv[3],'r')

stack_msp=0
for line in fp:
    info=line.split()
    type=0
    for i in info:
        if i=='ES':
            type=1
        if i=='LD' or i=='ST':
            type=2
        if i=='MSP':
            type=3
    if (type==1):
        if (start==0):
            start=int(info[0])
        addr=re.sub(r"\(|\)", "", info[3])
        addr=re.split("\(|:|\)",addr)
        if addr[0]!='EXC':
            name=get_sym(int(addr[0],16))
            if (name==None):
                name="Unknown"
            asm=line.split(':')
            offset=int(info[0])-start
            inst=asm[2].split()
            if (name.split('+')[0]!= last_name):
                if (offset!=last_offset):
                    print(last_line,end='')
                print (str(offset)+'('+info[0]+')\t'+addr[0]+':'+name+'\t'+asm[2],end='')
                last_name=name.split('+')[0]
            last_line=str(offset)+'('+info[0]+')\t'+addr[0]+':'+name+'\t'+asm[2]
            last_offset=offset
    elif (type==2):
        Address=int(info[6],16)
        if (Address&0xF0000000==0x40000000) or (Address&0xF0000000==0x50000000) or (Address&0xFFFF0000==0xE0080000):
            print('\t'+ str(last_offset)+'('+str(last_offset+start)+'):'+info[0]+' '+get_reg(Address)
                + ' ' + info[2]+ ' ' + info[3]+ ' ' + info[4]+ ' ' + info[5])   
        if (Address&0xF0000000==0x20000000):
            if (Address>stack_msp or Address<=(stack_msp-0x400)):
                print(line,end='')
                print("\t\t\t\t----%s %s" %(info[0],get_sym(Address)))
            
    elif (type==3):
        if stack_msp==0:
            stack_msp=int(info[2],16)
            print("MSP Base=0x%X" %(stack_msp))
            




