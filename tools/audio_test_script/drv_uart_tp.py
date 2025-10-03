import argparse
import os.path
import sys
import struct
import textwrap
import struct
import serial
if len(sys.argv)>1:
    sys.path.append(sys.argv[1])
from utils.utestClient import *
from utils.glovar import *
from utils.misc import *

#setup UART
block_size=[512]
length=409600
baudrate=460800

ser = serial.Serial(RTD_UART_PORT1, baudrate, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, timeout=5)
if not ser.isOpen():
    print("serial open failed!")
    exit(TC_RESULT_FAIL)

#init utest device
dut = utestClient(DUT_UTEST_PORT,'dut.log',DUT_UTEST_PORT_BAUDRATE)
wait_idx=0
for size in block_size:
    #run testcase on utest
    dut.run('tc_drv_uart_loop',1,'uart1 '+ str(baudrate) +' 8 1 NONE ' + str(length) + ' ' + str(size))
    i=0
    while (i<length):
        data=ser.read(size=size);
        data=bytearray(data)
        ser.write(data)
        i = i+len(data)
    if (i<length):
        result = TC_RESULT_FAIL
        break
    result=TC_RESULT_PASS
    time.sleep(5)
    
#close devices
ser.close()
dut.close()

ser = serial.Serial(RTD_UART_PORT2, baudrate, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, timeout=5)
if not ser.isOpen():
    print("serial open failed!")
    exit(TC_RESULT_FAIL)

#init utest device
dut = utestClient(DUT_UTEST_PORT,'dut.log',DUT_UTEST_PORT_BAUDRATE)
wait_idx=0
for size in block_size:
    #run testcase on utest
    dut.run('tc_drv_uart_loop',1,'uart2 '+ str(baudrate) +' 8 1 NONE ' + str(length) + ' ' + str(size))
    i=0
    while (i<length):
        data=ser.read(size=size);
        data=bytearray(data)
        ser.write(data)
        i = i+len(data)
    if (i<length):
        result = TC_RESULT_FAIL
        break
    result=TC_RESULT_PASS
    time.sleep(5)

#return test result
report_result(result)
exit(result)

