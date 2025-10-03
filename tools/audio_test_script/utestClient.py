import argparse
import os.path
import sys
import struct
import textwrap
import struct
import serial
import threading
import time
import re
import telnetlib

from misc import *
from glovar import *

jrtt_port='Jlink_RTT'

#ReferenceTestDevice
class utestClient:
    count = 0
    def __init__(self, port, log, baudrate=115200,prefix=''):
        utestClient.count +=1
        self.idx = utestClient.count
        self.port=port
        self.running=-1
        self.prefix=prefix
        if (port==jrtt_port):
            self.tn=telnetlib.Telnet("127.0.0.1", 19021)            
        else:            
            self.ser = serial.Serial()
            self.ser.port = port
            self.ser.baudrate = baudrate
            self.ser.timeout = 0
            self.ser.open()        
        self.t = threading.Thread(target=self.get_data)
        self.t.daemon=True
                
        self.casename = ""
        self.log_file =open(log, "w")
        self.utest_log("utestClient init port %s,baudrate=%d,log %s\n"%(port,baudrate,log))
        self.test_result = TC_RESULT_FAIL
    def __del__(self):
        utestClient.count -=1
        
    def utest_log(self,str):
        tc_log("[utest%d]%s"%(self.idx,str))

    def close(self):
        #self.t.stop()
        self.running = 0
        if (self.port==jrtt_port):
            self.send_data('\n')
            self.tn.close()
            self.utest_log("utestClient on port %s close!"%self.port)
        else:
            self.ser.close()
            self.utest_log("utestClient on port %s close!"%self.ser.port)
        self.log_file.flush()
        self.log_file.close()


    def get_data(self):
        global failed
        exp_len = 512

        str=""
        
        result_pattern = ('\[?/utest\]\s*\[\s*(\S*)\s*\]\s*\[\s*result\s*\]\s*testcase\s*\(%s\)'%self.casename)
        self.utest_log("Thread started\n")

        self.running=1;
        #sem.acquire()
        while self.running :
            if (self.port==jrtt_port):
                data = self.tn.read_some()
            else:
                if (self.ser.is_open):
                    data = self.ser.read(exp_len)
                else:
                    data = ""
            
            if len(data)>0:
                #print(data)
                self.log_file.write(data)
                str += data
                data=""

                searchObj = re.search(result_pattern,str)
                if searchObj:
                    result_str = searchObj.group(1)
                    self.utest_log("testcase[%s] result:%s"%(self.casename,result_str))
                    if("FAILED" in result_str):
                        self.test_result = TC_RESULT_FAIL
                    elif("PASSED" in result_str):
                        self.test_result = TC_RESULT_PASS
                    self.running = 0
                    break

        self.utest_log("Thread exit\n")

    def send_data(self, data):
        self.utest_log(data)
        self.log_file.write(add_timestamp("send:%s\n"%data))
        if (self.port==jrtt_port):
            if (self.prefix != ''):
                self.tn.write(self.prefix+ ' on\n')
                time.sleep(1)
            self.tn.write(self.prefix + ' ' + data)
        else:
            self.ser.write(data)

    def run(self,case_name,loop,param=""):
        
        self.casename = case_name
        self.test_result = TC_RESULT_INIT
        if (self.running<0):
            self.t.start()
        else:
            self.t = threading.Thread(target=self.get_data)
            self.t.daemon=True
            self.t.start()
        send_cmd = ("utest_run %s %d %s\n"%(case_name,loop,param))
        self.send_data(send_cmd)
        
        
        #serial.send_data(str(len(data))+"\r")
        #serial.send_data(data)
        
        #self.stop()
        #self.t.join()
        #self.ser.close()
        return 0
    
    def get_result(self,timeout):

        sleep_time = 0
        
        #timeout = timeout * TIMEOUT_RATIO
        while self.running and sleep_time < timeout:
            time.sleep(0.1)
            sleep_time+=0.1

        if sleep_time >= timeout:
            self.utest_log("get result TIME OUT!")
            self.running = 0
        
        return self.test_result

