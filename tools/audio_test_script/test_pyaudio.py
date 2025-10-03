from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import os.path
import pyaudio
import wave
import sys
import serial
import threading
import textwrap

class SerialThread:
    def __init__(self, port, ch, samplerate):
        self.running = 1
        self.ser = serial.Serial()
        self.ser.port = port
        self.ser.baudrate = 1000000
        self.ser.timeout = None
        self.ser.open()

        p = pyaudio.PyAudio()
        self.stream = p.open(format=p.get_format_from_width(2),
                channels=ch,
                rate=samplerate,
                output=True)
        self.wf=wave.open(FLAGS.file, 'wb')
        print("Record channels=%d samplerate=%d" %(ch,samplerate))
        print(FLAGS.file)
        print(self.wf)
        self.wf.setnchannels(ch)
        self.wf.setsampwidth(2)
        self.wf.setframerate(samplerate)
        self.t = threading.Thread(target=self.get_data)
        self.t.daemon = 1
        self.total=0
        self.t.start()

    def get_data(self):
        exp_len=256
        data=""
        print("Start recording")
        while self.running and self.ser.is_open:
            data += self.ser.read(size=exp_len)
            length=len(data)
            if (length>0):
                self.total+=length
                #print("get %d audio, total %d" %(length, self.total))
                print("get %d audio" %(length))
                self.stream.write(data)
                self.wf.writeframes(data)
                data=""    
        print("End recording")
            
    def send_data(self, data):
        self.ser.write(data)

    def stop(self):
        self.running = 0
        #self.t.join()
        self.ser.close()
        self.wf.close()
        self.stream.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="General Usage ", 
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=textwrap.dedent('''\
         Note: Test Audio tools. This application read audio PCM data from UART. PCM is 16K sample rate and 16bits   
         Usage: python test_pyaudio.py --port=<COM port> --file=<audio saved file>
         '''))

    parser.add_argument(
        '--file',
        type=str,
        default='audio.wav',
        help='Saved audio file.')
    parser.add_argument(
        '--port',
        type=str,
        default='COM3',
        help='Download UART port')
    parser.add_argument(
        '--ch',
        type=int,
        default='2',
        help='audio channels')
    parser.add_argument(
        '--rate',
        type=int,
        default='16000',
        help='audio samplerate')
            
    FLAGS, unparsed = parser.parse_known_args()
    print("Press q to end recording")    
    serial=SerialThread(FLAGS.port,FLAGS.ch,FLAGS.rate)
    while (1):
        ch=sys.stdin.read(1)
        if ch=='q' :
            serial.stop()
            break
    
