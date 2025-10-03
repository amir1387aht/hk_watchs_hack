#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from bmp import *

# example:
# Show rgb565 454x454 framebuffer at 0x20027610
# python jlinkbin2bmp.py SF32LB58X rgb565 454 454 20027610


if __name__ == '__main__':

    jlink_tgt_dev = sys.argv[1]
    cf = sys.argv[2]
    width=int(sys.argv[3])  #in pixel
    height=int(sys.argv[4])  #in pixel
    address=int(sys.argv[5],16)   #framebuffer addresss

    image = bmp(width, height)

    offset = 0


    #Dump bin through Jlink
    color_depth = bin_format_depth(cf)
    if color_depth == 0:
       raise Exception("Invalid format",cf)
    else:
       print ("color depth = %d"%color_depth)

    fb_bytes=width*height*color_depth/8
    bin_file_name=('%08x.bin'%(address))

    os.system('echo connect > tmp.jlink')
    os.system(('echo savebin %s %x %x >> tmp.jlink'%(bin_file_name,address,fb_bytes)))
    os.system('echo exit >> tmp.jlink')
    os.system(('call jlink.exe -device %s -if SWD -speed 12000 -autoconnect 1 -CommandFile tmp.jlink'%(jlink_tgt_dev)))
    
    #read bin file
    f = open(bin_file_name, "rb")
    try:
        org_data = f.read()
    finally:
        f.close()



    image.convert_bin2bmp(cf, org_data, offset)

    bmp_fname = ("%x_%dx%d_%s.bmp"%(address,width,height,cf))
    image.save_image(bmp_fname)

    os.system('del tmp.jlink')
    os.system(bmp_fname)