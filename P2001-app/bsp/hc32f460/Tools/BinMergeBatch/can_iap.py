from ctypes import *
import time
import tkinter as tk
from tkinter import filedialog
import os
import sys
import copy

####### IAP CMD Definition################################################################################
IAP_CODE_NONE = 0x00
IAP_CODE_SOH = 0x01
IAP_CODE_STX = 0x02
IAP_CODE_EOT = 0x04
IAP_CODE_ACK = 0x06
IAP_CODE_NAK = 0x15
IAP_CODE_CAN = 0x18
IAP_CODE_C = 0x43

IAP_CMD_REQUEST = 0
IAP_CMD_REQUEST_ACK = 1
IAP_CMD_REQUEST_ACK_CONFIRM = 2
IAP_CMD_STATUS = 3

# #####################################Function Definition Start###########################################

def can_wait_ack(node):
	"wait for ack after can message transmit"
	res = dll.Receive(nDeviceType, nDeviceInd, 0, byref(vco2), 1, 10000)  # 以vco2的形式接收报文， timeout in ms

	if res > 0:
		#print(i)
		#print("ID:", hex(vco2.ID))
		#print(list(vco2.Data))  # 打印接收到的报文
		if (vco2.ID == 0x11000000) and (vco2.ExternFlag == 1) and (vco2.Data[0] == IAP_CMD_STATUS) and ((vco2.Data[1] == node)) and (vco2.Data[2] == IAP_CODE_ACK):
			return IAP_CODE_ACK
		if (vco2.ID == 0x11000000) and (vco2.ExternFlag == 1) and (vco2.Data[0] == IAP_CMD_STATUS) and ((vco2.Data[1] == node)) and (vco2.Data[2] == IAP_CODE_NAK):
			return 1
		elif (vco2.ID & 0xFFFF0000 == (0x10000000 + (node << 16))) and (vco2.ExternFlag == 1) and ((vco2.Data[0] + vco2.Data[1]) == 0xFF):
			return IAP_CODE_ACK
	else:
		print("receive fault")
		return -1

	return -1


#    IAP_CODE_NONE = 0x00,
#    IAP_CODE_SOH = 0x01,
#    IAP_CODE_STX = 0x02,
#    IAP_CODE_EOT = 0x04,
#    IAP_CODE_ACK = 0x06,
#    IAP_CODE_NAK = 0x15,
#    IAP_CODE_CAN = 0x18,
#    IAP_CODE_C = 0x43,
def can_iap_set_offline():
	#set CAN nodes offline first
	vco = VciCanObj()
	vco.ID = 0x000  # 帧的ID
	vco.SendType = 1  # 发送帧类型，0是正常发送，1为单次发送，这里要选1！要不发不去！
	vco.RemoteFlag = 0
	vco.ExternFlag = 0
	vco.DataLen = 2
	vco.Data = (0x02, 0)
	vco.Reserved = (0, 0, 0)
	
	frame_num = dll.Transmit(nDeviceType, nDeviceInd, 0, byref(vco), 1)  # 发送vco
	if frame_num == 1:
		print("set can bus offline")
		return 0
	else:
		print("packet transmit failed, retry")
		time.sleep(1)
		return -1
		
def can_iap_handshake(node):
	#handshake with target node
	vco = VciCanObj()
	vco.ID = 0x11000000  # 帧的ID
	vco.SendType = 1  # 发送帧类型，0是正常发送，1为单次发送，这里要选1！要不发不去！
	vco.RemoteFlag = 0
	vco.ExternFlag = 1
	vco.DataLen = 3
	vco.Data = (0x00, node,  0x00)
	vco.Reserved = (0, 0, 0)
	
	#set iap request to target node
	frame_num = dll.Transmit(nDeviceType, nDeviceInd, 0, byref(vco), 1)  # 发送vco
	if frame_num == 1:
		# wait iap request ack from target node
		res = dll.Receive(nDeviceType, nDeviceInd, 0, byref(vco2), 1, 10000)  # 以vco2的形式接收报文， timeout in ms
		if res > 0:
			print("ID:%s, Ext:%d" % (hex(vco2.ID), vco2.ExternFlag))
			print(list(vco2.Data))  # 打印接收到的报文
			
			if (vco2.ID == 0x11000000) and (vco2.ExternFlag == 1) and (vco2.Data[0] == 0x01)  and (vco2.Data[1] == node):
				print("iap request ack received")
				vco.Data[0] = 0x02
				vco.Data[1] = node
				vco.Data[2] = 0
				vco.DataLen = 3
				#transmit iap request ack confirm message
				frame_num = dll.Transmit(nDeviceType, nDeviceInd, 0, byref(vco), 1)  # 发送vco
				if frame_num == 1:
					print("iap request ack confirmed")
					return 0
				else:
					print("packet transmit failed, retry")
					time.sleep(1)
					return -1		
			else:
				print("iap request ack receive failed")
				return -1
		else:
			print("handshake message receive failed")
			return -1
	else:
		print("packet transmit failed, retry")
		time.sleep(1)
		return -1		
		
		
def can_iap_wait_start(node):
	res = dll.Receive(nDeviceType, nDeviceInd, 0, byref(vco2), 1, 3000)  # 以vco2的形式接收报文， timeout in ms

	if res > 0:
		#print("ID:%s, Ext:%d" % (hex(vco2.ID), vco2.ExternFlag))
		#print(list(vco2.Data))  # 打印接收到的报文
		
		if (vco2.ID == 0x11000000) and (vco2.ExternFlag == 1) and (vco2.Data[0] == IAP_CMD_STATUS) and (vco2.Data[1] == node) and (vco2.Data[2] == IAP_CODE_C):
			return IAP_CODE_C
	else:
		print("start message receive failed")
		return -1

	return -1	


# #####################################Function Definition End###########################################
#dll = windll.LoadLibrary('./ECanVci.dll')  # 调用dll文件
dll = windll.LoadLibrary('./ECanVci64.dll')  # 调用dll文件
nDeviceType = 3  # 设备类型USBCAN-2E-U
nDeviceInd = 0  # 索引号0，代表设备个数
nReserved = 0  # 无意义参数
# nCANInd = 1  # can通道号


# 定义一个python的'结构体'，使用ctypes继承Structure，内容是初始化需要的参数，依据产品手册
class VciInitConfig(Structure):
    _fields_ = [("AccCode", c_ulong),  # 验收码，后面是数据类型
                ("AccMask", c_ulong),  # 屏蔽码
                ("Reserved", c_ulong),  # 保留
                ("Filter", c_ubyte),  # 滤波使能。0=不使能，1=使能使能时，/
                # 请参照SJA1000验收滤波器设置验收码和屏蔽码。
                ("Timing0", c_ubyte),  # 波特率定时器0（BTR0）
                ("Timing1", c_ubyte),  # 波特率定时器1（BTR1)
                ("Mode", c_ubyte)]  # 模式。=0为正常模式，=1为只听模式， =2为自发自收模式


# 定义发送报文的结构体
class VciCanObj(Structure):
    _fields_ = [("ID", c_uint),  # 报文帧ID'''
                ("TimeStamp", c_uint),  # 接收到信息帧时的时间标识
                ("TimeFlag", c_ubyte),  # 是否使用时间标识， 为1时TimeStamp有效
                ("SendType", c_ubyte),  # 发送帧类型。=0时为正常发送,=1时为单次发送（不自动重发)，/
                # =2时为自发自收（用于测试CAN卡是否损坏） ， =3时为单次自发自收（只发送一次， 用于自测试），/
                # 只在此帧为发送帧时有意义。
                ("RemoteFlag", c_ubyte),  # 是否是远程帧。=0时为数据帧，=1时为远程帧。
                ("ExternFlag", c_ubyte),  # 是否是扩展帧。=0时为标准帧（11位帧ID），=1时为扩展帧（29位帧ID）。
                ("DataLen", c_ubyte),  # 数据长度DLC(<=8)， 即Data的长度
                ("Data", c_ubyte * 8),  # CAN报文的数据。 空间受DataLen的约束。
                ("Reserved", c_ubyte * 3)]  # 系统保留


# 定义一个用于初始化的实例对象vic
vic = VciInitConfig()
vic.AccCode = 0x00000000
vic.AccMask = 0xffffffff
vic.reserved = 0
vic.Filter = 0
vic.Timing0 = 0x00  # 500Kbps
vic.Timing1 = 0x1C  # 500Kbps
vic.Mode = 0

# 定义报文实例对象，用于发送
vco = VciCanObj()
vco.ID = 0x00000055  # 帧的ID
vco.SendType = 1  # 发送帧类型，0是正常发送，1为单次发送，这里要选1！要不发不去！
vco.RemoteFlag = 0
vco.ExternFlag = 0
vco.DataLen = 8
vco.Data = (1, 2, 3, 4, 5, 6, 7, 0)
vco.Reserved = (0, 0, 0)
# 也可以用下面的定义方式，其中报文可以用0x的表达选择是16进制还是10进制

# 定义报文实例对象，用于接收
vco2 = VciCanObj()
vco2.ID = 0x00000001  # 帧的ID 后面会变成真实发送的ID
vco2.SendType = 0  # 这里0就可以收到
vco2.RemoteFlag = 0
vco2.ExternFlag = 1
vco2.DataLen = 3
vco2.Data = (0, 0, 0, 0, 0, 0, 0, 0)


'''设备的打开如果是双通道的设备的话，可以再用initcan函数初始化'''
# OpenDevice(设备类型号，设备索引号，参数无意义)
ret = dll.OpenDevice(nDeviceType, nDeviceInd, nReserved)
print("opendevice:", ret)
# InitCAN(设备类型号，设备索引号，第几路CAN，初始化参数initConfig)，
ret = dll.InitCAN(nDeviceType, nDeviceInd, 0, byref(vic))
print("initcan0:", ret)
# StartCAN(设备类型号，设备索引号，第几路CAN)
ret = dll.StartCAN(nDeviceType, nDeviceInd, 0)
print("startcan0:", ret)

# open bin file
application_window = tk.Tk()

# 设置文件对话框会显示的文件类型
my_filetypes = [('bin files', '.bin'),('rbl files', '.rbl')]

# 请求选择文件
application_window.withdraw()
file_path = filedialog.askopenfilename(parent=application_window,
                                    initialdir=os.getcwd(),
                                    title="Please select a file:",
                                    filetypes=my_filetypes)
# open file
bin_file = open(file_path, 'rb');

# get file size
file_size = os.path.getsize(file_path)

# choose target iap node
iap_node = 0
while(1):
	print("CAN Device Nodes:")
	print("0 - Broadcast")
	print("1 - TND_CTL")
	print("2 - TND_INT")
	print("3 - TND_DRV")
	print("4 - TND_SNR")
	print("5 - TND_PWR")

	iap_node = int(input("> Choose which node will be update:"))
	if (iap_node > 5) or (iap_node < 0):
		print("Invalid node, please retry !")
	else: 
		break

# start handshake
print("set all can node offline ...")
retry = 8
while(retry):
	can_iap_set_offline()
	retry -= 1
	time.sleep(0.02)
time.sleep(1)	

# clear can buffer first
res = dll.ClearBuffer(nDeviceType, nDeviceInd, 0)
if res == 1:
	print("buffer clear success")
else:
	print("buffer clear failed")
	
print("start CAN IAP handshake ...")	
while(1):
	res = can_iap_handshake(iap_node)
	if res == 0:
		break
	time.sleep(0.02)

# wait start request
print("wait start message from node ...")	
while(1):
	res = can_iap_wait_start(iap_node)
	if res == IAP_CODE_C:
		break

# start transmission
j = 0
packet_index = 0
retry_count = 8
frame_index = 0
frame_index_mask = 0
for i in range(file_size):
	data = bin_file.read(1) #每次输出一个字节
	vco.Data[j] = data[0]
	j += 1
	if j == 8:
		frame_index_mask = 255 - frame_index
		vco.ID = 0x10000000 + (iap_node << 16) + (frame_index << 8) + frame_index_mask # 帧的ID
		vco.ExternFlag = 1
		vco.DataLen = 8
		retry_count = 8
		#send CAN message
		while retry_count:
			len = dll.Transmit(nDeviceType, nDeviceInd, 0, byref(vco), 1)  # 发送vco
			if len == 1:
				print("[%d%%] packet %d send (%d frame)" %((packet_index+1)*8*100/file_size,packet_index,len))
				res = can_wait_ack(iap_node)
				if res == IAP_CODE_ACK:
					# ack ok, then switch to next packet
					frame_index = (frame_index + 1) % 256;
					break
			print("packet transmit failed, retry")
			time.sleep(0.5)
			retry_count -= 1
		if retry_count == 0:
			print("packet %d transmit failed, abort!" %(packet_index))
			break;
		j=0
		packet_index += 1
			
# all file bytes read out			
if (i+1) == file_size:
	# left data less than 8 ,then fill it with 0xFF
	if j != 0:
		for j in range(j,8):
			vco.Data[j] = 0xFF	
		frame_index_mask = 255 - frame_index
		vco.ID = 0x10000000 + (iap_node << 16) + (frame_index << 8) + frame_index_mask # 帧的ID
		#vco.ID = 0x10010000 + (frame_index << 8) + frame_index_mask # 帧的ID
		vco.ExternFlag = 1
		vco.DataLen = 8
		retry_count = 8
		#send CAN message
		while retry_count:
			len = dll.Transmit(nDeviceType, nDeviceInd, 0, byref(vco), 1)  # 发送vco
			if len == 1:
				print("[%d%%] packet %d send (%d frame)" %((packet_index+1)*8*100/file_size,packet_index,len))
				res = can_wait_ack(iap_node)
				if res == IAP_CODE_ACK:
					# ack ok, then switch to next packet
					frame_index = (frame_index + 1) % 256;
					break

			print("packet transmit failed, retry")
			time.sleep(0.5)
			retry_count -= 1
		if retry_count == 0:
			print("packet %d transmit failed, abort!" %(packet_index))
		packet_index += 1

# prepare to finish transmission
while (1):
	vco.ID = 0x11000000
	vco.ExternFlag = 1
	vco.Data[0] = 0x03
	vco.Data[1] = iap_node
	vco.Data[2] = 0x04
	vco.DataLen = 3
	len = dll.Transmit(nDeviceType, nDeviceInd, 0, byref(vco), 1)  # 发送vco
	if len == 1:
		print("Prepare to finish transmission")
		res = can_wait_ack(iap_node)
		if res == IAP_CODE_ACK:
			break
		

print("Transmission successful !!!")
# close file
bin_file.close()

ret = dll.CloseDevice(nDeviceType, nDeviceInd)
print("closedevice:", ret)


