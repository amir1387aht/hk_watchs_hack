from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import os.path
import sys
import struct
import textwrap
import struct
import serial
import threading
import time
import ble_serial
import math

DFU_IMG_HDR_ENC     =1
DFU_IMG_BODY_ENC    =2
DFU_CONFIG_ENC      =3
DFU_END             =4

DFU_IMG_HDR_COMPRESS  =1
DFU_IMG_BODY_COMPRESS =2
DFU_COMPRESS_END      =3

DFU_CONFIG_UID          =1
DFU_CONFIG_ROOT         =2
DFU_CONFIG_SIG_HASH     =3
DFU_CONFIG_FLASH_TABLE  =8
DFU_CONFIG_SIG          =9                      

DFU_FLASH_SEC_CONFIG    =0         
DFU_FLASH_FACTORY_CAL   =1         
DFU_FLASH_CONFIG        =10        
DFU_FLASH_LOG           =11        
DFU_FLASH_MATRICS       =12        

failed=0
delay_for_result=0.1

def print_hex(data):
    if (sys.version_info > (3, 0)):
        print(data.hex())
    else :
        print(data.encode('hex'))

def to_bytes(i):
    if (sys.version_info > (3, 0)):
        return i.to_bytes(4,byteorder='little')
    else:
        return struct.pack("<I", i)
        
def from_bytes(data):
    if (sys.version_info > (3, 0)):
        return int.from_bytes(data,'little')
    else: 
        if (len(data)==4):
            return struct.unpack("<I", data)[0]
        else:
            return struct.unpack("<H", data)[0]


class SerialThread:
    def __init__(self, port, baudrate=1000000):
        self.running = 1
        self.port = port
        if (port=="ble"):
            self.ble = ble_serial.BleSerial(self.get_data2)
            self.packet_count=0
        else:
            self.ser = serial.Serial()
            self.ser.port = port
            self.ser.baudrate = baudrate
            self.ser.timeout = 0
            self.ser.open()
            self.t = threading.Thread(target=self.get_data)
            self.t.daemon = 1
            self.t.start()
    def get_data2(self, state):
        global failed
        print("get_data", state)
        if (state!=0):
            failed=1
        sem.release()

    def get_data(self):
        global failed
        exp_len = 512
        data=""
        sem.acquire()
        while self.running and self.ser.is_open:
            data += self.ser.read(exp_len)
            
            #if len(data)>0:
            #    print(data)
            
            if (data.find("Fail")>=0):
                if (FLAGS.verbose==2):
                    print("Received:"+data)
                failed=1;
                data = ""
                sem.release()
            if (data.find("OK")>=0):
                if (FLAGS.verbose==2):
                    print("Received:"+data)
                data = ""    
                sem.release()
            
            #data=""    
            
    def send_data(self, data):
        if (self.port=="ble"):
            print(len(data))
            serial_len = len(data) + 5
            serial_len_low = serial_len & 0xFF
            serial_len_high = (serial_len >> 8) & 0xFF
            len_low = len(data) & 0xFF
            len_high = (len(data) >> 8) & 0xFF
            count_low = self.packet_count & 0xFF
            count_high =  (self.packet_count >> 8) & 0xFF
            print(count_low, count_high, len_low, len_high)
            #b = bytearray([0x01, 0x00, 0x04, count_low, count_high, len_low, len_high])
            #b.append(self.packet_count)
            #b.append(d_len)
            #
            if(len(data)>100):
                count=math.ceil(len(data)/100)
                print(count)
                for num in range(0, count):
                    if (num==count-1):
                        b1=data[num*100:]
                    else:
                        b1=data[num*100:(num*100+100)]
                    #print("count[]",num, len(b1), b1)
                    if (num==0):
                        b = bytearray([0x01, 0x01, serial_len_low, serial_len_high, 0x08, count_low, count_high, len_low, len_high])
                    elif (num==count-1):
                        b = bytearray([0x01, 0x03])
                    else:
                        b = bytearray([0x01, 0x02])
                    b.extend(bytearray(b1))
                    self.ble.send(b)
            else:
                b = bytearray([0x01, 0x00, serial_len_low, serial_len_high, 0x08, count_low, count_high, len_low, len_high])
                b.extend(bytearray(data))
                self.ble.send(b)
            #print("before send", b)
            
            self.packet_count += 1
        else:
            self.ser.write("dfu_recv "+str(len(data))+"\r")
            self.ser.write(data)

    def stop(self):
        if (self.port=="ble"):
            b = bytearray([0x01, 0x00, 0x01, 0x00, 0x0a])
            self.packet_count = 0
            self.ble.send(b)
            print("stop")
        else:
            self.running = 0
            #self.t.join()
            self.ser.close()
    
def download_ftab():
    file_in=open(FLAGS.table+"_enc.bin", "rb")
    data2=file_in.read()
    serial=SerialThread(FLAGS.port)
    data=struct.pack("<BB", DFU_CONFIG_ENC, DFU_CONFIG_FLASH_TABLE)
    data+=data2
    serial.send_data(data)
    time.sleep(delay_for_result)
    serial.stop()
       
def download_image():
    size=os.stat(FLAGS.eimg).st_size
    file_in=open(FLAGS.eimg, "rb")
    data2=file_in.read(32+316)
    #data2=file_in.read(32+80)
    print(size)
    serial=SerialThread(FLAGS.port)
    data=struct.pack("<BB", DFU_IMG_HDR_ENC, FLAGS.flashid)
    data+=data2
    print(data)
    serial.send_data(data)
    sem.acquire()
    if (failed==1):
        print("Image download failed, header not recognized\n")
        return;
    
    offset=0
    img_offset=0
    data=struct.pack("<BB", DFU_IMG_BODY_ENC, FLAGS.flashid)
    count=0;
    while (1):
        data2=file_in.read(32+4+FLAGS.bksize)
        if (len(data2)==0):
            break;
        count=count+1;
        if (FLAGS.verbose==2):
            print("offset=%d, len=%d, img_offset=%d, img_len=%d\n"%(offset, len(data2), img_offset, len(data2)-36))
        elif (FLAGS.verbose==1): 
            print(".", end="")
            if (count==64):
                print("\n", end="")
                count=0
        else:
            if (count==16):
                print("%2.0f%s" %((offset*100.0/size),"%"))
                count=0
        offset+=len(data2)
        img_offset+=len(data2)-36        
        data2=data+data2
        serial.send_data(data2)
        sem.acquire()
        if (failed==1):
            break;
    if (failed==0): 
        if (FLAGS.verbose==2):
            print("Verify image...\n")
        data=struct.pack("<BB", DFU_END, FLAGS.flashid)
        serial.send_data(data)
        sem.acquire()
    if (failed==1):
        print("\nImage download failed\n")
    else:
        print("\nImage download success\n")
    serial.stop()


def download_test(): 
    serial=SerialThread(FLAGS.port)   
    #data=struct.pack("<BB", DFU_END, FLAGS.flashid)
    data=bytearray(b'\x01\x02\xfe\xe7\x01\t\t\x00\x00\x17\t\x00\x00\xb7\x08\x00\x00\x1b\t\x00\x00\x1d\t\x00\x00\x1f\t\x00\x00!\t')
    serial.send_data(data)
    time.sleep(5)        
    serial.stop()
    
def download_root():    
    key=open(FLAGS.key + ".bin", "rb").read()
    data=struct.pack("<BB", DFU_CONFIG_ENC, DFU_CONFIG_ROOT)
    data+=key
    serial=SerialThread(FLAGS.port)
    serial.send_data(data)
    time.sleep(delay_for_result)
    serial.stop()
    
        
def download_sigkey():    
    key=open(FLAGS.key + "_pub.der", "rb").read()
    data=struct.pack("<BB", DFU_CONFIG_ENC, DFU_CONFIG_SIG)
    data+=key
    serial=SerialThread(FLAGS.port)
    serial.send_data(data)
    time.sleep(delay_for_result)
    serial.stop()
    
def download_sighash():    
    key=open(FLAGS.key + "_hash.bin", "rb").read()
    data=struct.pack("<BB", DFU_CONFIG_ENC, DFU_CONFIG_SIG_HASH)
    data+=key
    serial=SerialThread(FLAGS.port)
    serial.send_data(data)
    time.sleep(delay_for_result)
    serial.stop()    
    
def download_uid():    
    key=open(FLAGS.key + ".bin", "rb").read()
    data=struct.pack("<BB", DFU_CONFIG_ENC, DFU_CONFIG_UID)
    data+=key
    serial=SerialThread(FLAGS.port)
    serial.send_data(data)
    time.sleep(delay_for_result)
    serial.stop()      

def download_flashwrite():
    size=os.stat(FLAGS.eimg).st_size
    file_in=open(FLAGS.eimg, "rb")
    serial=SerialThread(FLAGS.port)
    
    offset=FLAGS.offset;
    offset>>=8;
    count=0;
    while (1):
        data=struct.pack("<BBH", DFU_FLASH_WRITE, FLAGS.flashid,offset)
        data2=file_in.read(256)
        if (len(data2)==0):
            break;
        count=count+1;
        if (FLAGS.verbose==2):
            print("offset=%d, len=%d\n"%(offset, len(data2)))
        elif (FLAGS.verbose==1): 
            print(".", end="")
            if (count==64):
                print("\n", end="")
                count=0
        else:
            if (count==16):
                print("%2.0f%s" %((offset*256*100.0/size),"%"))
                count=0
        offset=offset+1
        data2=data+data2
        serial.send_data(data2)
        sem.acquire()
        if (failed==1):
            break;
    if (failed==1):
        print("\nFlash write failed\n")
    else:
        print("\nFlash write success\n")
    serial.stop()
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="General Usage ", 
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=textwrap.dedent('''\
         Note: Please do not use suffix for each key, for example, core01.pem, use core01 as keyname   
         Send Flashtable: python download.py ftab --table=<flash table> --port=<port name>
         Send image:   python download.py img --eimg=<encrypted image> --bksize=<block size> --port=<port name>
         Download signature:   python download.py sigkey --key=<sig key> --port=<port name>
         Download root:     python download.py root --key=<sig key> --port=<port name>
         Flash write:     python download.py write --flashid=<flashid> --eimg=<flash content> --offset=<write offset> --port=<port name>
         '''))

    parser.add_argument('action', choices=['ftab', 'img', 'sigkey', 'sighash', 'root', 'test', 'write', 'uid'], default='key')
    parser.add_argument(
        '--flashid',
        type=int,
        default=2,
        help='Flash ID number')
    parser.add_argument(
        '--eimg',
        type=str,
        default='sifli01_e.bin',
        help='Encrypted and signed image.')
    parser.add_argument(
        '--bksize',
        type=int,
        default=512,
        help='Block size in bytes to encrypt.')
    parser.add_argument(
        '--key',
        type=str,
        default='sifli01key',
        help='Encrypted core key.')
    parser.add_argument(
        '--table',
        type=str,
        default='ftab',
        help='Encrypted Flash table')
    parser.add_argument(
        '--port',
        type=str,
        default='COM3',
        help='Download UART port')
    parser.add_argument(
        '--verbose',
        type=int,
        default=0,
        help='Progress show')
    parser.add_argument(
        '--offset',
        type=int,
        default=0,
        help='Flash write offset')
        
    FLAGS, unparsed = parser.parse_known_args()
    print("parameter ready")
    sem = threading.Semaphore()
    if (FLAGS.action == 'ftab'):
        download_ftab()
    if (FLAGS.action == 'img'):
        download_image()
    if (FLAGS.action == 'sigkey'):
        download_sigkey()
    if (FLAGS.action == 'sighash'):
        download_sighash()
    if (FLAGS.action == 'root'):
        download_root()        
    if (FLAGS.action == 'test'):
        download_test()     
    if (FLAGS.action == 'write'):
        download_flashwrite()    
    if (FLAGS.action == 'uid'):
        download_uid()     
