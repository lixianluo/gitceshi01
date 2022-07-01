from ctypes import *
import time
import tkinter as tk
from tkinter import filedialog
import os
import sys
import copy
import shutil
from zlib import crc32
import struct

# global definitions
app_bin_name = "app.bin"
app_bin_crc32_name = "app_ota.bin"
merge_bin_name = "app_boot_merge.bin"
boot_bin_name = "P2001_bootload.bin"

# target device for ota file running
# TND_SNR: sensor hub
# TND_DRV: drive board
# TND_INT: interface board
# TND_CTL: control board
# TND_PWR: 新增电源
# P2001_DRV:2001项目软件
target_device: str = "P2001_DRV"


# first copy raw app bin file from project output folder
shutil.copyfile('..\..\\P2001_app.bin', '.\P2001_app.bin')

# open bin file
application_window = tk.Tk()

# 设置文件对话框会显示的文件类型
my_filetypes = [('bin files', '.bin'),('rbl files', '.rbl')]

# 请求选择文件
application_window.withdraw()
file_path = filedialog.askopenfilename(parent=application_window,
                                    initialdir=os.getcwd(),
                                    title="Please select app file:",
                                    filetypes=my_filetypes)
# open file
bin_file = open(file_path, 'rb');

print("app bin file name is: ",os.path.basename(file_path))

# get file size
file_size = os.path.getsize(file_path)

# offset size of valid application code in bin file of HDSC chip
#offset_size = 32768
offset_size = 0
#skip null data and read real bin file
bin_file.seek(offset_size,0)
data = bin_file.read(file_size-offset_size)

app_bin_file = open(app_bin_name, 'wb')
app_bin_file.write(data)

# check file alignment
#print("file size is: %d, remain: %d" %(file_size, file_size%16))
i = file_size % 16

# fill file with 0xFF
if i != 0:
	file_size = file_size + 16 - i
	data = struct.pack('B',0xFF)
	for i in range(i, 16):
		app_bin_file.write(data)

#print("file size is: %d" %(file_size))

# close file
bin_file.close()
app_bin_file.close()

# start to calculate CRC32
crc32_file = open(app_bin_name, 'rb')
app_file_size = os.path.getsize(app_bin_name)
print("app bin file size is: %d" %(app_file_size))

# get crc32
data = crc32_file.read()
bin_crc32 = crc32(data)& 0xffffffff
print("crc:%s"%(hex(bin_crc32)))

# add crc and magic word at begin
bin_file_crc32 = open(app_bin_crc32_name, 'wb')
magic_word_begin = 0xf0a55a0f
magic_word_end = 0xa5f00f5a

a = struct.pack('<I', magic_word_begin)	# 4 bytes write, little endian
b = struct.pack('<I', app_file_size)	# 4 bytes write, little endian
c = struct.pack('<I', bin_crc32)		# 4 bytes write, little endian
d = struct.pack('<I', magic_word_end)	# 4 bytes write, little endian

bin_file_crc32.write(a)
bin_file_crc32.write(b)
bin_file_crc32.write(c)
bin_name = struct.pack('<16s', target_device.encode('UTF-8'))
bin_file_crc32.write(bin_name)
bin_file_crc32.write(d)
bin_file_crc32.write(data)
# close file
crc32_file.close()
bin_file_crc32.close()

# bootloader size, 32 Kbytes
boot_size = 0x8000

merge_bin = open(merge_bin_name, 'wb')
boot_bin = open(boot_bin_name, 'rb')
boot_data = boot_bin.read()
merge_bin.write(boot_data)
merge_bin.seek(boot_size, 0)
app_bin = open(app_bin_name, 'rb')
app_data = app_bin.read()
merge_bin.write(app_data)

# close file
merge_bin.close()
boot_bin.close()
app_bin.close()
