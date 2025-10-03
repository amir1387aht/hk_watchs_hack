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

def ptc_parse(excel_file):
    wb=openpyxl.load_workbook(filename=excel_file)
    # funtions 
    ws=wb['Sheet1']
    data=ws.values
    data=list(data)
    
    ptc1_funcs=[]
    ptc2_funcs=[]
    ptc1=0
    ptc2=0
    for row in data:        
        if row[3]=='ptc1_trig':
            ptc1=1
        elif row[3]=='ptc2_trig':
            ptc1=0
            ptc2=1
        elif row[3] is not None:
            if (ptc1==1):
                ptc1_funcs+=row[4:12]                
            if (ptc2==1):
                ptc2_funcs+=row[4:12]
    ptc1_funcs.reverse()
    ptc2_funcs.reverse()
    print('/** PTC 1 trigger functions */')
    print('enum{')
    j=0
    index=0
    for i in ptc1_funcs:
        if i is not None:
            if (type(i) is long) or i=='/':
                print('\tPTC_HCPU_RSVD_'+str(j)+', // %d' %(index))
                j=j+1
            else:
                print('\tPTC_HCPU_'+i+', // %d' %(index))
            index=index+1;
    print('};')
    print('/** PTC 2 trigger functions */')
    print('enum{')
    j=0
    index=0
    for i in ptc2_funcs:
        if i is not None:
            if (type(i) is long) or i=='/':
                print('\tPTC_LCPU_RSVD_'+str(j)+', // %d' %(index))
                j=j+1
            else:
                print('\tPTC_LCPU_'+i+', // %d' %(index))
            index=index+1;
    print('};')
    
            
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="General Usage ", 
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=textwrap.dedent('''\
            python ptc.py --excel=<filename> 
         '''))
    parser.add_argument(
        '--excel',
        type=str,
        default='ptc_trig.xslx',
        help='PTC trigger definition file')
        
    FLAGS, unparsed = parser.parse_known_args()    
    ptc_parse(FLAGS.excel)

        
