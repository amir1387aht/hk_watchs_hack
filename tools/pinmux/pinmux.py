from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import os.path
import sys
import struct
import openpyxl
import textwrap

def get_func_sel_num(title_row):
    if (len(title_row) > 8) and ("Function" in title_row[8]):
        func_sel_num = 16
    else:
        func_sel_num = 8    

    return func_sel_num   

def pinmux_parse_h(excel_file):
    wb=openpyxl.load_workbook(filename=excel_file)
    # funtions 
    ws=wb['Functions']
    data=ws.values
    data=list(data)
    print('#ifndef _BF0_PIN_CONST_H')
    print('#define _BF0_PIN_CONST_H')    
    print("/** @addtogroup PINMUX")
    print(" * @{")
    print(" */\n")

    func_idx=1
    for row in data:        
        if row[2]=='Function Name ':
            print('/** pin function */')
            print('typedef enum{\n\tPIN_FUNC_UNDEF,')
        elif row[2] is not None:
            print("\t/** {} ={}*/".format(row[2],func_idx))
            func_idx=func_idx+1
            print('\t'+row[2]+',')
    print('\tPIN_FUNC_MAX,')
    print('} pin_function;')
    
    # HCPU pads
    func_idx=0
    ws=wb['HPSYS']
    data=ws.values
    data=list(data)
    started=0
    func_sel_num = 0
    for row in data:
        if (0 == func_sel_num) and (row is not None):
            if ("Function" in row[4]):
                func_sel_num = get_func_sel_num(row[4:])
            elif ("Function" in row[9]):
                func_sel_num = get_func_sel_num(row[9:])
            else:
                pass    

        if row[2]=='PAD_SIP00':
            print('\n/** HCPU pin pad */')
            print('typedef enum {\n\tPIN_PAD_UNDEF_H,')
            started=1

        if (started==1) and (row is not None) and (row[2] is not None) and ("PAD" in row[2]):
            print("\t/** {} {}*/".format(row[2], func_idx))
            func_idx=func_idx+1
            print('\t'+row[2]+',')
    print('\tPIN_PAD_MAX_H,')
    print('} pin_pad_hcpu;')
    
    #LCPU pads
    func_idx=0
    ws=wb['LPSYS']
    data=ws.values
    data=list(data)
    for row in data:        
        if row[2]=='Pad Name':
            print('\n/** LCPU pin pad */')
            print('typedef enum {\n\tPIN_PAD_UNDEF_L,')
        elif row[2] is not None:
            print("\t/** {} {}*/".format(row[2],func_idx))
            func_idx=func_idx+1
            print('\t'+row[2]+',')
    print('\tPIN_PAD_MAX_L,')
    print('}pin_pad_lcpu;')

    print("#define PIN_FUNC_SEL_NUM  ({})".format(func_sel_num))
    print("\n/** HCPU pad function definition table */")
    print("extern const unsigned short pin_pad_func_hcpu[][{}];".format(func_sel_num))
    print("/** LCPU pad function definition table */")
    print("extern const unsigned short pin_pad_func_lcpu[][{}];".format(func_sel_num))
    print("#ifdef PIN_DEBUG")    
    print("    extern const char pin_function_str[][20];")
    print("    extern const char pin_pad_str_hcpu[][20];")
    print("    extern const char pin_pad_str_lcpu[][20];")
    print("#endif")
    
    print("/**   ")
    print(" * @}")
    print(" */\n")
    
    print('#endif')


    

def pinmux_parse_c(excel_file):
    wb=openpyxl.load_workbook(filename=excel_file)
    print('#include "bf0_pin_const.h"')
    print('#ifdef PIN_DEBUG')

    # functions debug string
    ws=wb['Functions']    
    data=ws.values
    data=list(data)
    for row in data:
        if row[2]=='Function Name ':
            print('const char pin_function_str[][20]={\n\t"UNDEF",')
        elif row[2] is not None:
            print('\t"'+row[2]+'",')
    print('};')

    # HCPU debug string
    ws=wb['HPSYS']
    data=ws.values
    data=list(data)
    started=0
    func_sel_num = 0
    func_start_col = -1
    for row in data:  
        if (0 ==  func_sel_num) and (row is not None):
            if ("Function" in row[4]):
                func_sel_num = get_func_sel_num(row[4:])
                func_start_col = 4
            elif ("Function" in row[9]):
                func_sel_num = get_func_sel_num(row[9:])
                func_start_col = 9
            else:
                pass    

        if (row[2]=='PAD_SIP00'):
            print('const char pin_pad_str_hcpu[][20]={\n\t"UNDEF",')
            started=1

        if (started==1) and (row[2] is not None) and ("PAD" in row[2]):
            print('\t"'+row[2]+'",')
    print('};')    
    
    #LCPU debug string
    ws=wb['LPSYS']
    data=ws.values
    data=list(data)
    for row in data:        
        if row[2]=='Pad Name':
            print('const char pin_pad_str_lcpu[][20]={\n\t"UNDEF",')
        elif row[2] is not None:
            print('\t"'+row[2]+'",')
    print('};')    

    #end for debug strings
    print('#endif')

    #HCPU function mapping matrix
    ws=wb['HPSYS']
    data=ws.values
    data=list(data)
    started=0
    for row in data:        
        if (row[2]=='PAD_SIP00'):
            print('const unsigned short pin_pad_func_hcpu[][{}]={{\n\t{{{}}},'.format(func_sel_num,','.join(['0']*func_sel_num)))
            started=1            
        if (started==1) and (row[2] is not None) and ("PAD" in row[2]):
            row=row[func_start_col:(func_start_col+func_sel_num)]
            print('\t{', end='')
            for i in range(func_sel_num):
                if (i>0):
                    print(',\t', end='')
                if (i >= len(row)) or (row[i]==None) or ('#' == row[i][0]) or ('x' == row[i][0]) or ('' == row[i].strip()):
                    print('0', end='')
                else:
                    print(row[i].replace('!', ''), end='')
            print('},')                    
    print('};')    
            
    #LCPU function mapping matrix
    ws=wb['LPSYS']
    data=ws.values
    data=list(data)
    for row in data:        
        if row[2]=='Pad Name':
            print('const unsigned short pin_pad_func_lcpu[][{}]={{\n\t{{{}}},'.format(func_sel_num,','.join(['0']*func_sel_num)))
        else:
            row=row[func_start_col:(func_start_col+func_sel_num)]
            print('\t{', end='')
            for i in range(func_sel_num):
                if (i>0):
                    print(',\t', end='')
                if (i >= len(row)) or (row[i]==None) or ('#' == row[i][0])  or ('x' == row[i][0]) or ('' == row[i].strip()):
                    print('0', end='')
                else:
                    print(row[i].replace('!', ''), end='')
            print('},')                    
    print('};')    
            
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="General Usage ", 
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=textwrap.dedent('''\
            python pinmux.py --excel=<filename> --output=<output folder>
         '''))
    parser.add_argument(
        '--excel',
        type=str,
        default='pinmux.xslx',
        help='Pinmux definition file')
    parser.add_argument(
        '--type',
        type=str,
        choices=["c", "h"], 
        default='h',
        help='Output file type')
        
    FLAGS, unparsed = parser.parse_known_args()    
    if (FLAGS.type=='c'):
        pinmux_parse_c(FLAGS.excel)
    else:
        pinmux_parse_h(FLAGS.excel)

        
