#programme de test basique
#
import time
import socket
from struct import *
import zmq


context = zmq.Context()

#  Socket to talk to server
print("Connecting to server…")
socket = context.socket(zmq.SUB)
socket.connect("tcp://localhost:5563")

socket.setsockopt_string(zmq.SUBSCRIBE,"IQ")

File_Number=0
#  Do 10 requests, waiting each time for a response
print("Connected to server.")
for request in range(90000):

	str=socket.recv()

	if str == b'IQ':
		dataPoints=unpack("i",socket.recv())[0]
		print(dataPoints)
		if dataPoints==0:
			print("End of Transmission")
			f=open(fileName,'wb')
			f.write(b'0')
			f.close()
			File_Number+=1
		else:
			floatPoints=dataPoints*2
			strData=socket.recv()
			dataValues=unpack("%if"%floatPoints,strData)
			fileName="dataRaw%05i"%File_Number
			f=open(fileName,'wb')
			f.write(strData)
			f.close()
			File_Number+=1
