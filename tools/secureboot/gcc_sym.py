from __future__ import print_function
import sys
import re

def read_sym(file):
    symbols=[]
    fp=open(file,'r')
    for line in fp:
        info=line.split()
        if info[0][0]=='#':
            continue
        if len(info)<2:
            continue
        symbols+=[[info[2],info[0]]]
    fp.close()
    return symbols

def print_gcc_sym(symbols):
    for i in symbols:
        print(i[0] + ' = ' + i[1] +';')

symbols=read_sym(sys.argv[1])
print_gcc_sym(symbols)
