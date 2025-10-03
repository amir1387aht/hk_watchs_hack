#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import sys
import struct
import random


from array import array
from color_sf import *

class bmp:
    """ bmp data structure """
 
    def __init__(self, w=1080, h=1920, color = 0xffffff):
        self.w = w
        self.h = h
        self.gen_bmp_header()
        self.paint_bgcolor(color)
 
    def calc_data_size (self):
        if((self.w*3)%4 == 0):
            self.dataSize = self.w * 3 * self.h
        else:
            self.dataSize = (((self.w * 3) // 4 + 1) * 4) * self.h
 
        self.fileSize = self.dataSize + 54
 
    def conv2byte(self, l, num, len):
        tmp = num
        for i in range(len):
            l.append(tmp & 0x000000ff)
            tmp >>= 8
 
    def gen_bmp_header (self):
        self.calc_data_size();
        self.bmp_header = [0x42, 0x4d]
        self.conv2byte(self.bmp_header, self.fileSize, 4) #file size
        self.conv2byte(self.bmp_header, 0, 2)
        self.conv2byte(self.bmp_header, 0, 2)
        self.conv2byte(self.bmp_header, 54, 4) #rgb data offset
        self.conv2byte(self.bmp_header, 40, 4) #info block size
        self.conv2byte(self.bmp_header, self.w, 4)
        self.conv2byte(self.bmp_header, self.h, 4)
        self.conv2byte(self.bmp_header, 1, 2)
        self.conv2byte(self.bmp_header, 24, 2) #888
        self.conv2byte(self.bmp_header, 0, 4)  #no compression
        self.conv2byte(self.bmp_header, self.dataSize, 4) #rgb data size
        self.conv2byte(self.bmp_header, 0, 4)
        self.conv2byte(self.bmp_header, 0, 4)
        self.conv2byte(self.bmp_header, 0, 4)
        self.conv2byte(self.bmp_header, 0, 4)
 
    def print_bmp_header (self):
        length = len(self.bmp_header)
        for i in range(length):
            print("{:0>2x}".format(self.bmp_header[i]), end=' ')
            if i%16 == 15:
                print('')
        print('')
 
    def paint_bgcolor(self, color=0xffffff):
        self.rgbData = []
        for r in range(self.h):
            self.rgbDataRow = []
            for c in range(self.w):
                self.rgbDataRow.append(color)
            self.rgbData.append(self.rgbDataRow)
 
    def paint_line(self, x1, y1, x2, y2, color):
        k = (y2 - y1) / (x2 - x1)
        for x in range(x1, x2+1):
            y = int(k * (x - x1) + y1)
            self.rgbData[y][x] = color
 
    def paint_rect(self, x1, y1, w, h, color):
        for x in range(x1, x1+w):
            for y in range(y1, y1+h):
                self.rgbData[y][x] = color
 
    def paint_point(self, x, y, color=0x000000):
        self.rgbData[y][x] = color

    #bin_format       - bin format
    #bin_array - bin array
    #offset   - bin offset to read
    def convert_bin2bmp(self, bin_format, bin_array, offset):
        index = offset
        for y in range(self.h-1,-1,-1):
            for x in range(0,self.w,1):
                argb8888 = to_argb88888(bin_format, bin_array, index)
                index=index+(bin_format_depth(bin_format)>>3)
                
                a = (argb8888 >> 24)&0xFF
                r = (argb8888 >> 16)&0xFF
                g = (argb8888 >> 8)&0xFF
                b = (argb8888 >> 0)&0xFF
                
                r = (r * a) >> 8
                g = (g * a) >> 8
                b = (b * a) >> 8
                rgb888 = r << 16 | g<<8 | b

                self.paint_point(x, y, rgb888)

    def save_image(self, name="save.bmp"):
        f = open(name, 'wb')
 
        #write bmp header
        #f.write(array('B', self.bmp_header).tobytes())
        f.write(array('B', self.bmp_header))
 
        #write rgb data
        zeroBytes = self.dataSize // self.h - self.w * 3
 
        for r in range(self.h):
            l = []
            for i in range(len(self.rgbData[r])):
                p = self.rgbData[r][i]
                l.append(p & 0x0000ff)
                p >>= 8
                l.append(p & 0x0000ff)
                p >>= 8
                l.append(p & 0x0000ff)
 
            #f.write(array('B', l).tobytes())
            f.write(array('B', l))

            for i in range(zeroBytes):
                f.write(bytes(0x00))
 
        #close file
        f.close()
 