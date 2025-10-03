import asyncio
import time
import threading
from bleak import discover
from bleak import BleakClient

CHARACTERISTIC_UUID_CFG = "00000000-0000-0100-6473-5f696c666973"
CHARACTERISTIC_UUID_DATA = "00000000-0000-0200-6473-5f696c666973"




    
async def run(upper):
    devices = await discover()
    for d in devices:
        print(d)
        print(len(d.name))
        print(len("SIFLI-BLE-DEV\0"))
        if (d.name=="SIFLI-BLE-DEV\0"):
            global client
            client=BleakClient(d.address)
            connected=await client.connect()
            print(client)
            if (connected==True):
                svcs=await client.get_services()
                chars=svcs.characteristics
                for svc in svcs:
                    print("Service:", svc)               
                for c in chars:
                    ch=svcs.get_characteristic(c)
                    print("Characteristic:", ch)  
                    print("\tproperty:", ch.properties)
                    print("ch len", len(ch.uuid))
                    if (ch.uuid==CHARACTERISTIC_UUID_CFG):
                        if (('write' in ch.properties) or ('write-without-response' in ch.properties)) : 
                            a = bytearray(bytes(range(5)))
                            await client.write_gatt_char(ch.uuid,a)
                    elif (ch.uuid==CHARACTERISTIC_UUID_DATA):
                        if ('notify' in ch.properties):
                            await client.start_notify(ch.uuid,upper.notification_handler)
                        if ('write-without-response' in ch.properties):
                            #b0:uuid, b1:type, b2-bn:data
                            #negotiate:b2-b3:packet_size
                            p = bytearray([0x01, 0x00, 0x03, 0x00, 0x00, 0x01, 0x02])
                            data = await client.write_gatt_char(ch.uuid, p)

                            p = bytearray([0x01, 0x00, 0x03, 0x00, 0x02, 0x80, 0x00])
                            data = await client.write_gatt_char(ch.uuid, p)
                            #print (data)
                            print("send_completed")
                            upper.sem.acquire()
                            print("send pakcet req")
                            req=bytearray([0x01,0x00, 0x05, 0x00, 0x06, 0xFF, 0x01, 0x00, 0x00])
                            data = await client.write_gatt_char(ch.uuid, req)
                            upper.uuid=ch.uuid
                            upper.sem.acquire()
                            #print("uuid",upper.uuid)
                            #asyncio.run(client.write_gatt_char(sender, req))
                #time.sleep(1.0)
            else:
                print("connect failed")

class BleSerial():
    def __init__(self, notfi_callback):
        self.running = 1
        self.sem = threading.Semaphore(0)
        self.callback = notfi_callback
        loop = asyncio.get_event_loop()
        loop.run_until_complete(run(self))
        global client
        self.client = client

    async def send1(self, data):
        print("send1")
        data1 = await self.client.write_gatt_char(self.uuid, data)
    
    def send(self, data):
        #print("uuid", self.uuid, data)
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self.send1(data))

    def notification_handler(self, sender, data):
        global client
        """Simple notification handler which prints the data received."""
        print("{0}: {1}".format(sender, data))
        if (sender==CHARACTERISTIC_UUID_DATA):
            print("DATA",data[0],data[1],data[2],data[3])
            cateid=data[0]
            is_frag=data[1]
            if(cateid==0x01):
                msg=data[2]
                if(msg==0x01):
                    img_id=data[3]
                    img_state=data[4]
                    img_curr_ver=data[5:25]
                    print(img_id, img_state, img_curr_ver)
                    self.sem.release()
                if(msg==0x03):
                    status=data[3]
                    len=data[4] | data[5]<<4
                    print("req", status, len)
                    if(status==0x0):
                        #send packet req
                        self.sem.release()
                        #eq=bytearray([0x01, 0x02, 0xFF, 0x01, 0x00, 0x00])
                        #data = client.write_gatt_char(sender, req)
                        #asyncio.run(client.write_gatt_char(sender, req))
                        print("send")
                if(msg==0x07):
                    status=data[3]
                    if(status==0):
                        self.sem.release()
                        print("could send data")
                if(msg==0x09):
                    status=data[3]
                    self.callback(status)

def mycallback(data):
    print(data)

#loop = asyncio.get_event_loop()
#loop.run_until_complete(run())
#ble_client=BleSerial(mycallback)
#print("ble comp")
#bin1=bytearray([0x01,0x00, 0x04,0x01,0x00,0x04,0x00,0x05,0x06,0x07,0x08])
#ble_client.send(bin1)
#time.sleep(30.0)
#0