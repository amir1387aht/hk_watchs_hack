from __future__ import absolute_import
from __future__ import division
from __future__ import  print_function

import  argparse
import  os.path
import  sys
import  struct
import  textwrap
import  serial
import  threading
import  time

class SerialThread:
    def __init__(self, port, baudrate=1000000, record=False):
        self.running = 1
        self.ser = serial.Serial()
        self.ser.port = port
        self.ser.baudrate = baudrate
        self.ser.timeout = 0
        self.ser.open()
        if (record == True):
            self.file = open(FLAGS.logfile, "wb+")
        else:
            self.file = open(FLAGS.logfile, "rb")
        self.record = record
        self.t = threading.Thread(target=self.get_data)
        self.t.daemon = 1
        self.t.start()

    def get_data(self):
        global failed
        '''
        if (self.record == True):
            exp_len = 12
        else:
            exp_len = 4
        '''
        cnt = 0
        data = self.ser.read(4)
        print(len(data))
        while self.running and self.ser.is_open:
            if (len(data) < 4):
                data += self.ser.read(4-len(data))
            elif (len(data) == 4):
                temp = int.from_bytes(data[0:3], byteorder='little')
                if (temp == 0):
                    self.file.close()
                    break
                elif (self.record == False):
                    print('get request %d %d' % (cnt, temp))
                    cnt = cnt+1
                    data2 = self.file.read(temp * 12)
                    if(len(data2) < temp * 12):
                        self.file.close()
                        self.file = open(FLAGS.logfile, "rb")
                        data2 = self.file.read(temp * 12)
                        print('reopen file')
                    self.send_data(data2)
                    data = self.ser.read(4)
                elif (self.record == True):
                    data += self.ser.read(12 - len(data))
            elif (len(data) < 12):
                data += self.ser.read(12-len(data))
            elif (len(data) == 12):
                self.file.write(data)
                print('get record %d' % cnt)
                cnt = cnt + 1
                print(data)
                data = self.ser.read(4)
    def send_data(self, data):
        print("Sending: ")
        print(data)
        self.ser.write(data)

    def stop(self):
        self.running = 0
        self.ser.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="General Usage ",
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=textwrap.dedent('''\
            Note:
            Record input: python indev.py record --logfile=<log file name> --port=<port name>
            Playback input: python indev.py play --logfile=<log file name> --port=<port name>
            '''))
    parser.add_argument('action', choices=['record','play'], default='play')
    parser.add_argument(
        '--logfile',
        default = 'input.bin',
        help = 'Input log file' )

    parser.add_argument(
        '--port',
        type = str,
        default = 'COM4',
        help='Input uart port')

    FLAGS, unparsed = parser.parse_known_args()

    sem = threading.Semaphore()
    if (FLAGS.action == 'record'):
        serial = SerialThread(FLAGS.port, record=True)
    if (FLAGS.action == 'play'):
        serial = SerialThread(FLAGS.port, record=False)

    event = threading.Event()

try:
    print('Press Ctrl+C to exit')
    event.wait()
except KeyboardInterrupt:
    print('Got Ctrl+C')