#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from color_sf import *
 
# example:
# Convert rgb565 454x454 framebuffer hcpu_ram.bin to yuyv
# python bin2yuv.py hcpu_ram.bin rgb565 454 454 yuyv

def AddFileHeader(cf, width, height, l):
#      typedef struct
#      {
#          uint32_t cf : 5;          /* Color format: See `EPIC_Input_Color_Mode`*/
#          uint32_t always_zero : 3;
#      
#          uint32_t reserved : 2;
#      
#          uint32_t w : 11; /*Width of      the image map*/
#          uint32_t h : 11; /*Height of     the image map*/
#      } ffs_img_header_t;
    file_header= (height<<21)|(width<<10)|cf

    l.append((file_header>>0)&0xff)
    l.append((file_header>>8)&0xff)
    l.append((file_header>>16)&0xff)
    l.append((file_header>>24)&0xff)
        
if __name__ == '__main__':

    bin_file_name = sys.argv[1]
    cf = sys.argv[2]
    width=int(sys.argv[3])  #in pixel
    height=int(sys.argv[4])  #in pixel
    yuv_cf=sys.argv[5]


    #read bin file
    f = open(bin_file_name, "rb")
    try:
        org_data = f.read()
    finally:
        f.close()

    #file headr
    AddFileHeader

    if "yuyv" == yuv_cf:
        #Create output file
        f_yuv = open("./yuyv_%dx%d.yuv"%(width,height),"wb")
        
        #Convert data
        yuv_data = []
        AddFileHeader(0,width, height, yuv_data)
        RGBarray2YUYVarray(org_data,cf, width, height, yuv_data)
        
        #Ouput data
        f_yuv.write(array('B', yuv_data))
        f_yuv.flush()
        f_yuv.close()
        
    elif "uyvy" == yuv_cf:
        #Create output file
        f_yuv = open("./uyvy_%dx%d.yuv"%(width,height),"wb")
        
        #Convert data
        yuv_data = []
        AddFileHeader(0, width, height, yuv_data)
        RGBarray2UYVYarray(org_data,cf, width, height, yuv_data)
        
        #Ouput data
        f_yuv.write(array('B', yuv_data))
        f_yuv.flush()
        f_yuv.close()

    elif "iyuv" == yuv_cf:
        #Create output file
        f_y = open("./y_%dx%d.yuv"%(width,height),"wb")
        f_u = open("./u_%dx%d.yuv"%(width,height),"wb")
        f_v = open("./v_%dx%d.yuv"%(width,height),"wb")
        
        #Convert data
        y_data = []
        u_data = []
        v_data = []
        AddFileHeader(4,width, height, y_data)
        AddFileHeader(4,width>>1, height>>1, u_data)
        AddFileHeader(4,width>>1, height>>1, v_data)
        RGBarray2IYUVarray(org_data,cf, width, height, y_data, u_data, v_data)

        #Ouput data
        f_y.write(array('B', y_data))
        f_u.write(array('B', u_data))
        f_v.write(array('B', v_data))
        
        f_y.close()
        f_u.close()
        f_v.close()

            