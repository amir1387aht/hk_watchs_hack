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
import zlib

from Cryptodome.PublicKey import RSA
from Cryptodome.Random import get_random_bytes
from Cryptodome.Cipher import AES, PKCS1_OAEP
from Cryptodome.Hash import SHA256
from Cryptodome.Signature import pkcs1_15
from Cryptodome.Util import Counter

DFU_FLAG_ENC      =1
DFU_FLAG_AUTO     =2
DFU_FLAG_SINGLE   =4
DFU_FLAG_COMPRESS =16

DFU_ID_CODE        = 0
DFU_ID_CODE_MIX    = 1
DFU_ID_OTA_MANAGER = 2
DFU_ID_DL          = 3

DFU_IMG_ID_HCPU    = 0
DFU_IMG_ID_LCPU    = 1
DFU_IMG_ID_PATCH   = 2
DFU_IMG_ID_RES     = 3
DFU_IMG_ID_FONT    = 4




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
        


def dfu_packed_bin(img, eimg, session_key, is_enc=True):

    # 2. Align image to 16 bytes
    data = open(img, "rb").read()
    data = bytearray(data)
    remain = (len(data) % 16)
    if remain > 0:
        remain = 16 - remain
        str = u'0'
        for i in range(0, remain):
            data.append(0)

    img_len = to_bytes(len(data))

    # 4. Encrypt image
    i = 0
    cnt_prefix = open(FLAGS.sigkey + "_hash.bin", "rb").read(8)
    cnt_prefix += b'\x00' * 4
    cnter = Counter.new(32, prefix=cnt_prefix, initial_value=0)
    cipher_aes = AES.new(session_key, AES.MODE_CTR, counter=cnter)
    data3 = bytearray(b'')
    cipherimage = bytearray(b'')
    while i < len(data):
        # 7.1 Generate Hash+offset, data block
        if i + FLAGS.bksize < len(data):
            data2 = data[i:i + FLAGS.bksize]
            hash = SHA256.new(data[i:i + FLAGS.bksize])
            hash2 = hash.digest() + to_bytes(i)
        else:
            data2 = data[i:len(data)]
            hash = SHA256.new(data[i:len(data)])
            hash2 = hash.digest() + to_bytes(i)
        # 7.2 encrypt data block
        if is_enc == 1:
            ciphertext = cipher_aes.encrypt(data2)
        else:
            ciphertext = data2

        # 7.3 Save each encrypted block
        data3 += hash2
        data3 += ciphertext
        cipherimage += ciphertext
        i += len(data2)

    # 5. Calculate hash and get signature
    if ((FLAGS.flags & DFU_FLAG_ENC) == DFU_FLAG_ENC):
        hash = SHA256.new(cipherimage)  # Hash encrypted image for integrity check
    else:
        hash = SHA256.new(data)  # Hash plain image for integrity check
    pri_key = RSA.import_key(open(FLAGS.sigkey + "_pri.pem").read())
    sign_rsa = pkcs1_15.new(pri_key)
    signature = sign_rsa.sign(hash)  # Sign the hash

    # 7. Save to encrypted image file
    file_out = open(eimg, "wb")
    file_out.write(data3)
    return [signature, img_len]


def dfu_compress_bin(img, eimg):
    # 1. Align image to 16 bytes
    data = open(img, "rb").read()
    data = bytearray(data)
    remain = (len(data) % 16)
    if (remain > 0):
        remain = 16 - remain
        str = u'0'
        for i in range(0, remain):
            data.append(0)

    img_len = to_bytes(len(data))

    # Generate header
    pksize = struct.pack("<I", FLAGS.pksize)
    header = img_len + pksize
    i = 0

    # Compress
    compresstext = b''
    print(len(data))
    while (i < len(data)):
        if (i + FLAGS.pksize < len(data)):
            data2 = data[i:i + FLAGS.pksize]
        else:
            data2 = data[i:len(data)]
        i += len(data2)
        data2 = zlib.compress(data2, 9)
        pklen = struct.pack('<I', len(data2))
        # pklen = to_bytes(pklen)
        print(len(data2))
        compresstext += pklen + data2

    # header+=to_bytes(len(compresstext))
    img_text = header + compresstext
    file_out = open(eimg, "wb")
    file_out.write(img_text)


def dfu_bin_generation(bin, flag, session_key):
    img = bin+'.bin'
    is_enc=flag & DFU_FLAG_ENC
    if (flag & DFU_FLAG_COMPRESS):
        out_img = 'com_'+img
        dfu_compress_bin(img, out_img)
        img = 'com_'+img
    if (flag & DFU_FLAG_ENC):
        out_img = 'enc'+img
    else :
        out_img = 'out' + img
    return dfu_packed_bin(img, out_img, session_key, is_enc)




def dfu_generation() :

    file_key = open(FLAGS.key + ".bin", "rb")
    recipient_key = file_key.read()

    control_packet = struct.pack("<B", FLAGS.dfu_id)
    control_packet += struct.pack("<I", FLAGS.hw_ver)
    control_packet += struct.pack("<I", FLAGS.sdk_ver)
    control_packet += struct.pack("<I", FLAGS.fw_ver)

    img_count = struct.pack("<B", int(len(FLAGS.img_para)/3))
    bksize = struct.pack("<H", FLAGS.bksize)

    img_header = bksize+img_count
    print(img_header)
    list_pair=[FLAGS.img_para[i:i+3] for i in range(0, len(FLAGS.img_para), 3)]

    print(list_pair)
    # 1. Generate AES Key - Encrypt the data with the AES session key
    session_key = get_random_bytes(32)
    control_packet += session_key
    for a in list_pair:
        img_bin = a[0]
        img_flag = int(a[1])
        img_id = struct.pack("<B", int(a[2]))
        print(img_bin)
        print(type(img_flag))
        [img_sig, img_len] = dfu_bin_generation(img_bin, img_flag, session_key)
        img_header += img_sig + img_len + struct.pack("<H", img_flag) + img_id
    img_header_len = struct.pack("<H", len(img_header))
    control_packet += img_header_len
    control_packet += img_header
    print(control_packet)

    #6. Encrypt header
    cnt_prefix = open(FLAGS.sigkey + "_hash.bin", "rb").read(8)
    cnt_prefix += b'\x00' * 4
    hash=SHA256.new(control_packet)
    cnter=Counter.new(32,prefix=cnt_prefix,initial_value=0)
    cipher_core_aes = AES.new(recipient_key, AES.MODE_CTR, counter=cnter)
    enc_header = cipher_core_aes.encrypt(control_packet)
    pri_key = RSA.import_key(open(FLAGS.sigkey+"_pri.pem").read())
    sign_rsa = pkcs1_15.new(pri_key)
    signature = sign_rsa.sign(hash)

    file_out = open('ctrl_packet.bin', "wb")
    file_out.write(hash.digest())
    file_out.write(enc_header)
    file_out.write(signature)



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="General Usage ", 
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=textwrap.dedent('''\
         Note: Please do not use suffix for each key, for example, core01.pem, use core01 as keyname   
         Generate dfu bin: python dfu_bin_generate.py gen_dfu --img_para <bin_name, etc: img> <flag, ect: 17> <img_id, ect: 0> --key=<keyname, ect: s01> --sigkey=<sig bin, ect: sig> --dfu_id=<dfu_id, ect:1> --hw_ver=<hw version, ect: 51> --sdk_ver=<sdk_lowest_ver, ect: 7010> --fw_ver=<fw_ver, ect: 1001001>
         '''))

    parser.add_argument('action', choices=['gen_dfu'], default='gen_dfu')
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
        '--ver',
        type=str,
        default='sifli_0.1.0',
        help='Image version, length should less than 20 Byte')
    parser.add_argument(
        '--pksize',
        type=int,
        default=10240,
        help='Compress block size')
    parser.add_argument(
        '--img_para',
        type=str,
        default=10240,
        nargs='*',
        help='img parameter list')
    parser.add_argument(
        '--dfu_id',
        type=int,
        default=0,
        help='DFU ID')
    parser.add_argument(
        '--hw_ver',
        type=int,
        default=0,
        help='HW version requirement')
    parser.add_argument(
        '--sdk_ver',
        type=int,
        default=0,
        help='SDK version requirement')
    parser.add_argument(
        '--fw_ver',
        type=int,
        default=0,
        help='FW version')
    FLAGS, unparsed = parser.parse_known_args()
    FLAGS, unparsed = parser.parse_known_args()
    FLAGS, unparsed = parser.parse_known_args()
    FLAGS, unparsed = parser.parse_known_args()
    print(FLAGS.action)
    if (FLAGS.action == 'gen_dfu'):
        dfu_generation()
