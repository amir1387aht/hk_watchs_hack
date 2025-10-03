import os
import sys
import re
import struct
import shutil

def print_file(file_name):
    size=os.path.getsize(file_name)
    fp_bin=open(file_name, 'rb')
    data=fp_bin.read(4)
    count=0
    while (len(data)>0):
        if len(data) != 4:
            data += ('\x00'*(4-len(data)))
        temp=struct.unpack("<L", data)
        if ((count%4)==0):
            print("\t"),        
        print("0x%08X,"%(temp)),
        count=count+1
        if ((count%4)==0):
            print("\n"),
        data=fp_bin.read(4)      
    fp_bin.close()
    return size

def gen_ttf_array(src,rom=False):
    print("#include <stdint.h>\n"),
    print("#include <string.h>\n"),
    print("#include \"rtconfig.h\"\n"),    
    print ("const unsigned int g_ttf_font[]= { ")
    size=print_file(src)
    print ("};")
    print("const int g_ttf_font_size=%d;"%(size));
   
if __name__ == '__main__':    
    gen_ttf_array(sys.argv[1])
