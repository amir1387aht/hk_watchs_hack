#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import sys

f = open(sys.argv[1], "rb")
try:
    org_data = f.read()
finally:
    f.close()

for i in range(len(org_data)):
    if (sys.version_info > (3, 0)):
        print("0x%02X," %(org_data[i]), end='')
    else :
        print("0x%02X," %(ord(org_data[i])), end='')
    if 7 == (i % 8):
        print("")
	
