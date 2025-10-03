from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import os.path
import sys
import struct
import textwrap
import struct
import json5

from Crypto.PublicKey import RSA
from Crypto.Random import get_random_bytes
from Crypto.Cipher import AES, PKCS1_OAEP
from Crypto.Hash import SHA256
from Crypto.Signature import pkcs1_15
from Crypto.Util import Counter

TOTAL_PARTION = 16
SIG_OFFSET = (4+TOTAL_PARTION*16)
IMG_OFFSET = 4096+512*2
     
def print_hex(data):
    if (sys.version_info > (3, 0)):
        print(data.hex(), end='')
    else :
        print(data.encode('hex'), end='')

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
        
def encrypt_image_help(img, eimg) :
    file_key=open(FLAGS.key+".bin", "rb")
    recipient_key = file_key.read()
    
    #1. Generate AES Key - Encrypt the data with the AES session key
    session_key = get_random_bytes(32)
    
    #2. Align image to 16 bytes 
    data=open(img,"rb").read()
    data=bytearray(data)
    remain=(len(data)%16)
    if (remain>0) :
        remain=16-remain
        str=u'0'
        for i in range(0,remain):
            data.append(0)
                
    img_len=to_bytes(len(data))
    # 3. Generate Header
    # img_len(4 bytes)+bksize(2 bytes) + flags(2 bytes) + session_key(32 bytes) + signature(256 bytes) = 296 bytes
    bksize=struct.pack("<H", FLAGS.bksize)    
    flags=struct.pack("<H", FLAGS.flags)    
    header=img_len+bksize+flags+session_key

    # 4. Encrypt image
    i=0
    cnt_prefix=open(FLAGS.sigkey+"_hash.bin", "rb").read(8)
    cnt_prefix+='\x00'*4
    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0)
    cipher_aes = AES.new(session_key, AES.MODE_CTR,counter=cnter)
    data3=''
    cipherimage=''
    while (i<len(data)):
        # 7.1 Generate Hash+offset, data block
        if (i+FLAGS.bksize<len(data)):
            data2=data[i:i+FLAGS.bksize]
            hash=SHA256.new(data[i:i+FLAGS.bksize])
            hash2=hash.digest()+to_bytes(i)
        else:
            data2=data[i:len(data)]
            hash=SHA256.new(data[i:len(data)])
            hash2=hash.digest()+to_bytes(i)
        # 7.2 encrypt data block    
        ciphertext= cipher_aes.encrypt(data2)     
        
        # 7.3 Save each encrypted block
        data3+=hash2
        data3+=ciphertext
        cipherimage+=ciphertext
        i+=len(data2)

    #5. Calculate hash and get signature
    if ((FLAGS.flags&1)==1):
        hash=SHA256.new(cipherimage)                                       # Hash encrypted image for integrity check    
    else:
        hash=SHA256.new(data)                                              # Hash plain image for integrity check
    pri_key = RSA.import_key(open(FLAGS.sigkey+"_pri.pem").read())
    sign_rsa = pkcs1_15.new(pri_key)
    signature = sign_rsa.sign(hash)                                        # Sign the hash
    header+=signature
        
    #6. Encrypt header
    hash=SHA256.new(header)
    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0)
    cipher_core_aes = AES.new(recipient_key, AES.MODE_CTR, counter=cnter)
    enc_header = cipher_core_aes.encrypt(header)
    
    #7. Save to encrypted image file
    data2=hash.digest()+enc_header
    file_out=open(eimg, "wb")
    file_out.write(data2)
    file_out.write(data3)
    
def encrypt_image() :
    if os.path.isdir(FLAGS.img):
        if os.path.exists(FLAGS.eimg):
            print("Skip create, already exist:" + FLAGS.eimg)
        else:
            os.makedirs(FLAGS.eimg)
        for f in os.listdir(FLAGS.img):
            print(FLAGS.img+'/'+f)
            print(FLAGS.eimg+'/'+f+'_sec.bin')  
            encrypt_image_help(FLAGS.img+'/'+f, FLAGS.eimg+'/'+f+'_sec.bin')
    else:
        encrypt_image_help(FLAGS.img,FLAGS.eimg)

def encrypt_image_help_static(img, eimg) :
    file_key=open(FLAGS.key+".bin", "rb")
    recipient_key = file_key.read()
    #print_hex(recipient_key)
    #print("\r")
    
    uid=open(FLAGS.uid+".bin", "rb").read()
    #print_hex(uid)
    #print("\r")
    #1. Generate AES Key - Encrypt the data with the AES session key
    session_key = get_random_bytes(32)
    #print_hex(session_key)
    #print("\r")
    #2. Align image to 16 bytes 
    data=open(img,"rb").read()
    data=bytearray(data)
    remain=(len(data)%16)
    if (remain>0) :
        remain=16-remain
        str=u'0'
        for i in range(0,remain):
            data.append(0)
                
    img_len=to_bytes(len(data))
    # 3. Generate Header
    
    cnt_prefix=open(FLAGS.sigkey+"_hash.bin", "rb").read(8)
    cnt_prefix+='\x00'*4
    #print_hex(cnt_prefix)
    #print("\r\n")
    # img_len(4 bytes)+bksize(2 bytes) + flags(2 bytes) + session_key(32 bytes) + signature(256 bytes) = 296 bytes
    # cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0)
    #iv_val=open(FLAGS.sigkey+"_hash.bin", "rb").read(8)
    
    #2.1 encrypt session key
    cipher_core_aes = AES.new(recipient_key, AES.MODE_CBC, uid)
    #cipher_core_des = AES.new(recipient_key, AES.MODE_CBC, uid)
    enc_session = cipher_core_aes.encrypt(session_key)
 
   
    bksize=struct.pack("<H", FLAGS.bksize)    
    flags=struct.pack("<H", FLAGS.flags)    
    header=img_len+bksize+flags+enc_session

    #dec_session = cipher_core_des.decrypt(enc_session)

    # 3.1 Insert header to ftab
    #print(FLAGS.sigkey)
    sig_key=open(FLAGS.sigkey + "_pub.der", "rb").read()
    sig_len=len(sig_key)
    
    hd_len=len(header)
    
    file_hd=open(FLAGS.table, "rb")
    ftab_binary = file_hd.read()
    #print("sig len", len(sig_key), "total len", len(ftab_binary))
    data_new = ftab_binary[0:SIG_OFFSET]+sig_key+ftab_binary[(SIG_OFFSET+sig_len):IMG_OFFSET]+header+ftab_binary[(IMG_OFFSET+hd_len):len(ftab_binary)]    
    
    file_wr=open("enc_"+FLAGS.table,"wb")
    file_wr.write(data_new)

    # 4. Encrypt image
    i=0

    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0)
    cipher_aes = AES.new(session_key, AES.MODE_CTR,counter=cnter)
    data3=''
    cipherimage=''
    while (i<len(data)):
        # 7.1 Generate Hash+offset, data block
        if (i+FLAGS.bksize<len(data)):
            data2=data[i:i+FLAGS.bksize]
        else:
            data2=data[i:len(data)]
        # 7.2 encrypt data block    
        ciphertext= cipher_aes.encrypt(data2)     
        
        # 7.3 Save each encrypted block
        data3+=ciphertext
        i+=len(data2)

   
    #7. Save to encrypted image file
    file_out=open(eimg, "wb")
    file_out.write(data3)
    
def encrypt_image_static() :
    if os.path.isdir(FLAGS.img):
        if os.path.exists(FLAGS.eimg):
            print("Skip create, already exist:" + FLAGS.eimg)
        else:
            os.makedirs(FLAGS.eimg)
        for f in os.listdir(FLAGS.img):
            print(FLAGS.img+'/'+f)
            print(FLAGS.eimg+'/'+f+'_sec.bin')  
            encrypt_image_help_static(FLAGS.img+'/'+f, FLAGS.eimg+'/'+f+'_sec.bin')
    else:
        encrypt_image_help_static(FLAGS.img,FLAGS.eimg)

def decrypt_image_help(img,eimg) :
    #1. Get root key, Read signature hash, 1st 12 bytes as prefix for counter
    file_key=open(FLAGS.key+".bin", "rb")
    recipient_key = file_key.read()
    cnt_prefix=open(FLAGS.sigkey+"_hash.bin", "rb").read(8)
    cnt_prefix+='\x00'*4
    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0);
    
    #2. Decode image header
    file_in=open(eimg, "rb")
    hash=file_in.read(32)
    enc_header=file_in.read(296)
    cipher_core_aes = AES.new(recipient_key, AES.MODE_CTR, counter=cnter)
    header = cipher_core_aes.decrypt(enc_header)
    
    #3.Decrypt the data with the AES session key
    img_len=from_bytes(header[:4])
    bksize=from_bytes(header[4:6])
    flags=from_bytes(header[6:8])
    session_key=header[8:40]
    signature=header[40:296]
    print_hex(signature)
    
    i=0    
    if (sys.version_info > (3, 0)):
        data=bytes('', encoding='UTF-8')
    else:
        data=bytes('')
        
    print_hex(session_key)
    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0); 
    cipher_aes = AES.new(session_key, AES.MODE_CTR, counter=cnter)
    ciphertext=''
    while (i<img_len):
        hash=file_in.read(32)
        offset=file_in.read(4)
        if (i+bksize<img_len):
            ciphertext+=file_in.read(bksize)
            i+=bksize
        else:
            ciphertext+=file_in.read(img_len-i)
            i=img_len

    data=cipher_aes.decrypt(ciphertext)
    data=bytearray(data)    
    
    #4. Calculate Hash, and validate signature
    h = SHA256.new()
    if ((int(flags)&1)==1):
        print("Sign encrypted")
        h.update(ciphertext)
    else:
        print("Sign plain")
        h.update(data)
    print_hex(h.digest())
    data2=open(FLAGS.sigkey+"_pub.der", "rb").read()
    pub_key = RSA.import_key(data2)
    verify_rsa = pkcs1_15.new(pub_key)
    verify_rsa.verify(h,signature)
    
    #5. Save to image
    file_out=open(img, "wb")
    file_out.write(data)

def decrypt_image() :
    if os.path.isdir(FLAGS.eimg):
        if os.path.exists(FLAGS.img):
            print("Skip create, already exist:" + FLAGS.img)
        else:
            os.makedirs(FLAGS.img)
        for f in os.listdir(FLAGS.eimg):
            print(FLAGS.img+'/dec_'+f)
            print(FLAGS.eimg+'/'+f)  
            decrypt_image_help(FLAGS.img+'/dec_'+f, FLAGS.eimg+'/'+f)
    else:
        decrypt_image_help(FLAGS.img,FLAGS.eimg)
        
def gen_uid() :
    session_key = get_random_bytes(16)
    file_out = open(FLAGS.key+".bin", "wb")
    file_out.write(session_key)
    
def gen_key2() :
    key = RSA.generate(2048)
    private_key = key.export_key()
    file_out = open(FLAGS.sigkey+"_pri.pem", "wb")
    file_out.write(private_key)

    public_key = key.publickey().export_key()
    file_out = open(FLAGS.sigkey+"_pub.pem", "wb")
    file_out.write(public_key)    
      
def dump_sig():
    key = RSA.import_key(open(FLAGS.sigkey+"_pub.pem").read())    
    #Encrypt the core key using root key
    data=key.export_key(format='DER')
    file_out=open(FLAGS.sigkey+"_pub.der","wb")
    file_out.write(data);
    h=SHA256.new()
    h.update(data)
    data=h.digest()    
    file_out=open(FLAGS.sigkey+"_hash.bin","wb")
    file_out.write(data);
    
def gen_root() :
    session_key = get_random_bytes(32) 
    file_out = open(FLAGS.key+".bin", "wb")
    file_out.write(session_key)
	
def enc_ftab() :
    #1. Open key
    file_in=open(FLAGS.key+".bin", "rb")
    key=file_in.read()
    
    #2. Open flash table json, and output file
    with open(FLAGS.table+'.json') as f:
        data = json5.load(f)
    file_out= open(FLAGS.table+"_enc.bin", "wb")      
    
    #3. Translate json file into binary.
    data2=''
    for x in data['tables']:
        data2+=struct.pack("<I",x['base'])
        data2+=struct.pack("<I",x['size'])
        data2+=struct.pack("<I",x['xip_base'])
        data2+=struct.pack("<I",x['flags'])
    #4. Calculate Hash
    hash=SHA256.new(data2)
    
    #5. Encrypt the core key using root key
    cnt_prefix=open(FLAGS.sigkey+"_hash.bin", "rb").read(8)    
    cnt_prefix+='\x00'*4
    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0)
    cipher_core_aes = AES.new(key, AES.MODE_CTR, counter=cnter)
    data = cipher_core_aes.encrypt(data2)
 
    #6. Save to output file
    [ file_out.write(x) for x in (hash.digest(),data)]    
       
def dec_ftab() :
    #1. Open key
    file_in=open(FLAGS.key+".bin", "rb")
    key=file_in.read()
    
    #2. Open encrypted flash table and read back.
    enc_in = open(FLAGS.table+"_enc.bin", "rb")
    hash=enc_in.read(32)
    ciphertext=enc_in.read()
   
    #3. Decrypt the flash table
    cnt_prefix=open(FLAGS.sigkey+"_hash.bin", "rb").read(8)    
    cnt_prefix+='\x00'*4
    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0)
    cipher_core_aes = AES.new(key, AES.MODE_CTR, counter=cnter)
    data = cipher_core_aes.decrypt(ciphertext)

    #4. Print out decrypted flash table and save.
    table={}
    table['name']='Flash table'
    table['num']=int(len(data)/16)
    table['tables'] = []
    i=0
    for k in range(table['num']):
        base,size,xip_base,flags=struct.unpack("<IIII",data[i:i+16])
        i+=16
        table['tables'].append({'base':base,'size':size,'xip_base':xip_base,'flags':flags})
    print(table)    
    file_out= open(FLAGS.table+"2.json", "wb")     
    json5.dump(table,file_out)

def ftab() :   
    with open(FLAGS.table+'.json') as f:
        data = json5.load(f)
    file_out= open(FLAGS.table+".bin", "wb")      
    
    data2=''
    for x in data['tables']:
        data2+=struct.pack("<I",x['base'])
        data2+=struct.pack("<I",x['size'])
        data2+=struct.pack("<I",x['xip_base'])
        data2+=struct.pack("<I",x['flags'])
    file_out.write(data2)
    print_hex(data2)
    
def rftab() :   
    file_in = open(FLAGS.table+".bin", "rb")
    data= file_in.read()

    table={}
    table['name']='Flash table'
    table['num']=int(len(data)/16)
    table['tables'] = []
    i=0
    for k in range(table['num']):
        base,size,xip_base,flags=struct.unpack("<IIII",data[i:i+16])
        i+=16
        table['tables'].append({'base':base,'size':size,'xip_base':xip_base,'flags':flags})
    file_out= open(FLAGS.table+"_dec.json", "wb")     
    json5.dump(table,file_out)

def print_data(lable,data):
    print("unsigned char " + lable + "[] = {")
    for i in range (0,len(data)):
        if (i%16==0):
            print("    ", end='')
        print("0x", end='')
        print_hex(data[i])
        if (i!=len(data)-1):
            print(",", end='')
        if ((i%16)==15 or i==len(data)-1):
            print("\n", end='')
    print("};\n")
    
def test_vec() :
    file_key=open(FLAGS.key+".bin", "rb")
    recipient_key = file_key.read()
    print_data("root_key", recipient_key)
    
    cnt_prefix=open(FLAGS.sigkey+"_hash.bin", "rb").read(8)
    print_data("nounce_prfix", cnt_prefix)
    cnter=Counter.new(64,prefix=cnt_prefix,initial_value=0);
    
    data=bytearray(256)
    for i in range(0,256):
        data[i]=i
    print_data("plain_text", str(data))

    cipher_core_aes = AES.new(recipient_key, AES.MODE_CTR, counter=cnter)
    data = cipher_core_aes.encrypt(data)    
    print_data("encrypted_text",str(data))


def add_sig_in_ftab():
    sig_key=open(FLAGS.sigkey + "_pub.der", "rb").read()
    #print(FLAGS.sigkey)
    file_hd=open(FLAGS.table, "rb")
    ftab_binary = file_hd.read()
    #print_data("ftab", ftab_binary)
    
    sig_len=len(sig_key)
    #print("sig len", len(sig_key), "total len", len(ftab_binary))
    data_new = ftab_binary[0:SIG_OFFSET]+sig_key+ftab_binary[(SIG_OFFSET+sig_len):len(ftab_binary)]
    if "sifli01" in FLAGS.sigkey:
        file_wr=open("sec_enc_"+FLAGS.table,"wb")
    else:
        file_wr=open("plain_enc_"+FLAGS.table,"wb")
    file_wr.write(data_new)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="General Usage ", 
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=textwrap.dedent('''\
         Note: Please do not use suffix for each key, for example, core01.pem, use core01 as keyname   
         Generate root key: python imgtool.py root --key=<key>
         Generate core key: python imgtool.py uid --key=<keyname> 
         Generate signature key: python imgtool.py gensig --key=<keyname> 
         Encrypt image:     python imgtool.py enc --key=<keyname> --img=<plain image> --eimg=<encrypted image> --bksize=<block size>
         Encrypt flashtable: python imgtool.py enctab --key=<keyname> --table=<flash table>
         NonEncrypt flashtable: python imgtool.py ftab --table=<flash table>
         
         Validate commands:
         Decrypt image:     python imgtool.py dec --key=<keyname> --img=<plain image> --eimg=<encrypted image> --bksize=<block size>  
         Decrypt flashtable: python imgtool.py dectab --key=<keyname> --table=<flash table>
         Print Non-ecnrypted: python imgtool.py rftab --table=<flash table>
         '''))

    parser.add_argument('action', choices=['enc', 'dec', 'uid', 'root','dumproot', 'enctab', 'dectab','ftab','rftab','gensig','dumpsig', 'test', 'sig_ftab', 'enc_static'], default='enc')
    parser.add_argument(
        '--img',
        type=str,
        default='sifli01.bin',
        help='Plain image')
    parser.add_argument(
        '--eimg',
        type=str,
        default='sifli01_e',
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
        help='File for key.')
    parser.add_argument(
        '--sigkey',
        type=str,
        default='sifli01key',
        help='File for signature key in PEM format.')        
    parser.add_argument(
        '--table',
        type=str,
        default='ftab',
        help='Encrypted Flash table')
    parser.add_argument(
        '--flags',
        type=int,
        default=1,
        help='Image flags, 1:save encrypted')
    parser.add_argument(
        '--uid',
        type=str,
        default='sifli01uid',
        help='UID')
        
    FLAGS, unparsed = parser.parse_known_args()
    if (FLAGS.action == 'uid'):
        gen_uid()
    if (FLAGS.action == 'enc'):
        encrypt_image()
    if (FLAGS.action == 'dec'):
        decrypt_image()
    if (FLAGS.action == 'root'):
        gen_root()
    if (FLAGS.action == 'enctab'):
        enc_ftab()    
    if (FLAGS.action == 'dectab'):
        dec_ftab()    
    if (FLAGS.action == 'ftab'):
        ftab()         
    if (FLAGS.action == 'rftab'):
        rftab()
    if (FLAGS.action == 'gensig'):
        gen_key2()
    if (FLAGS.action == 'dumpsig'):
        dump_sig()    
    if (FLAGS.action == 'test'):
        test_vec()    
    if (FLAGS.action == 'sig_ftab'):
        add_sig_in_ftab()
    if (FLAGS.action == 'enc_static'):
        encrypt_image_static()
