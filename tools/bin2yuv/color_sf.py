#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import sys
import struct
import random


from array import array


def rand_c(v, range, mask):
    rand = random.randint(-range,range)
    if v + rand < 0:
        return 0
    elif v+rand > 255:
        return 255 & mask
    else:
        return (v + rand) & mask
#|          | bit31~bit25 | bit24~bit17 | bit16~bit8   | bit7~bit0   |
#| ------   | ------      | ------      | ------       | ------      |
#| RGB565   |    /        |    /        | R4~R0G5~G3   | G2~G0B4~B0  |
#| ARGB8565 |    /        | A7 ~ A0     | R4~R0G5~G3   | G2~G0B4~B0  |
#| RGB888   |    /        | R7 ~ R0     | G7 ~ G0      | B7 ~ B0     |
#| ARGB8888 | A7 ~ A0     | R7 ~ R0     | G7 ~ G0      | B7 ~ B0     |


def bin_format_depth(bin_format):
    if bin_format == "a8":
        return 8
    elif bin_format == "rgb565" or bin_format == "bgr565":
        return 16
    elif bin_format == "rgb888":
        return 24
    elif bin_format == "argb8888" or bin_format == "rgba8888":
        return 32
    else:
        return 0


def to_argb88888(bin_format, bin_array, index):
    a = 255

    if bin_format == "a8":
        r = ord(bin_array[index])
        g = r
        b = r
    elif bin_format == "rgb565":
        r = ord(bin_array[index+1]) | 0x7
        g_h = (ord(bin_array[index+1]) & 0x7)<<5
        g_l = (ord(bin_array[index]) & 0xE0) >> 3 
        g = g_h | g_l
        b = (ord(bin_array[index]) & 0x1F)<<3
    elif bin_format == "rgb888":
        b = ord(bin_array[index])
        g = ord(bin_array[index+1])
        r = ord(bin_array[index+2])

        #r = rand_c(r,3,0xF8)
        #g = rand_c(g,2,0xFC)
        #b = rand_c(b,3,0xF8)

    elif bin_format == "argb8888":
        b = ord(bin_array[index])
        g = ord(bin_array[index+1])
        r = ord(bin_array[index+2])
        a = ord(bin_array[index+3])

    elif bin_format == "rgba8888":
        a = ord(bin_array[index])
        b = ord(bin_array[index+1])
        g = ord(bin_array[index+2])
        r = ord(bin_array[index+3])
    else:
        r = 0
        g = 0
        b = 0

    #a = 255 #force alpha to 255
    return (a<< 24 |r << 16 | g<<8 | b)


#Y∈(16,235)  U/V ∈(16,240)
#R，G，B~[0,255]  
def yuv2rgb(yuv):
    y = (yuv >> 16)&0xFF
    u = (yuv >> 8)&0xFF
    v = (yuv >> 0)&0xFF

    r= y + ((360 * (v - 128))>>8)
    g= y - (( ( 88 * (u - 128)  + 184 * (v - 128)) )>>8)
    b= y +((455 * (u - 128))>>8)

    r = 0 if r < 0 else (255 if r > 255 else r)
    g = 0 if g < 0 else (255 if g > 255 else g)
    b = 0 if b < 0 else (255 if b > 255 else b)
    
    return (r << 16 | g<<8 | b)

def rgb2yuv(rgb888):
    r = (rgb888 >> 16)&0xFF
    g = (rgb888 >> 8)&0xFF
    b = (rgb888 >> 0)&0xFF
    
    y = (77*r + 150*g + 29*b)>>8
    u = ((-44*r  - 87*g  + 131*b)>>8) + 128
    v = ((131*r - 110*g - 21*b)>>8) + 128

    y = 16 if y < 16 else (235 if y > 235 else y)
    u = 16 if u < 16 else (240 if u > 240 else u)
    v = 16 if v < 16 else (240 if v > 240 else v)
    
    return (y << 16 | u<<8 | v)


def RGBarray2YUYVarray(rgb_data, rgb_cf, width, height, yuyv_data):
    index = 0
    for j in range(0,height,1):
        prev_y = 0
        for i in range(0,width,1):
            argb8888 = to_argb88888(rgb_cf, rgb_data, index)
            index=index+(bin_format_depth(rgb_cf)>>3)
            
            yuv = yuv2rgb(argb8888&0xFFFFFF)
            y = (yuv >> 16)&0xFF
            u = (yuv >> 8)&0xFF
            v = (yuv >> 0)&0xFF

            
            #print("%x, -> %x" % (argb8888,yuv))
            if i%2 == 1:
                yuyv_data.append(prev_y & 0x0000ff)
                yuyv_data.append(u & 0x0000ff)
                yuyv_data.append(y & 0x0000ff)
                yuyv_data.append(v & 0x0000ff)

            prev_y = y





def RGBarray2UYVYarray(rgb_data, rgb_cf, width, height, uyvy_data):
    index = 0
    for j in range(0,height,1):
        prev_y = 0
        for i in range(0,width,1):
            argb8888 = to_argb88888(rgb_cf, rgb_data, index)
            index=index+(bin_format_depth(rgb_cf)>>3)
            
            yuv = yuv2rgb(argb8888&0xFFFFFF)
            y = (yuv >> 16)&0xFF
            u = (yuv >> 8)&0xFF
            v = (yuv >> 0)&0xFF

            
            #print("%x, -> %x" % (argb8888,yuv))
            if i%2 == 1:

                uyvy_data.append(u & 0x0000ff)
                uyvy_data.append(prev_y & 0x0000ff)
                uyvy_data.append(v & 0x0000ff)
                uyvy_data.append(y & 0x0000ff)

            prev_y = y
            
            
            

def RGBarray2IYUVarray(rgb_data, rgb_cf, width, height, y_data, u_data, v_data):
    index = 0
    for j in range(0,height,1):
        for i in range(0,width,1):
            argb8888 = to_argb88888(rgb_cf, rgb_data, index)
            index=index+(bin_format_depth(rgb_cf)>>3)
            
            yuv = yuv2rgb(argb8888&0xFFFFFF)
            y = (yuv >> 16)&0xFF
            u = (yuv >> 8)&0xFF
            v = (yuv >> 0)&0xFF

            #print("%x, -> %x" % (argb8888,yuv))
            
            
            y_data.append(y & 0x0000ff)
            if (i%2 == 0) and (j%2 == 0):
                u_data.append(u & 0x0000ff)
                v_data.append(v & 0x0000ff)
