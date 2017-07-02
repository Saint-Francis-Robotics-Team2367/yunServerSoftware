import socket
import select
import serial
import sys
import time
import atexit
from Queue import Queue
from threading import Thread
 
#network related variables
PORT = 2380 # Game host port
BROADCAST_PORT = 472 # Broadcast port
BROADCAST_IP = '' 
LOCAL_IP = ''

#timouts for respective interval timers
KEEP_ALIVE_TIMEOUT = 2
NAME_BROADCAST_TIMEOUT = 1

#thread communication queues
broadcastQueue = Queue()
serialRealTimeQueue = Queue()
networkRealTimeQueue = Queue()
networkQueue = Queue()

#change this to the relevant team name when the script is loaded to the yun
ROBOT_NAME = "ChangeThisToTheTeamName"


#loging wrapper function    
def logWrite(strng):
	log.write("[" + str(time.time()) + "]" +strng +"\n")
	print strng
	
#Error log file, overwtite file from last reset
logFile = "./elog_" + str(time.time()) + ".txt"
try:
	log = open(logFile, "w")
except Exception as f:
	print "Uhhh... check that your hard drive isn't on fire!"   
	sys.exit(1)
else:
	logWrite("log File" + logFile)

    
#threaded function that listens for broadcasts from host.
def broadcastListener():
        
	bCastSock =socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	bCastSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	bCastSock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

	try:
		bCastSock.bind((BROADCAST_IP, BROADCAST_PORT))
	except socket.error as msg:
		bCastSock.close()
		logWrite('Broadcast bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1] +"\n")
		sys.exit(1)
			
	lastKeepAlive = time.time()
	while True:   
		readReady = select.select([bCastSock], [], [], 0.02)
		if readReady[0]: 
			data, sender =  bCastSock.recvfrom(1500) 
            #check that the datagram is a alive packet
			if data == sender[0]:
				lastKeepAlive = time.time()
				broadcastQueue.put(sender)
		time.sleep(.05)
        
#serial communication thread
def serialComthread():
	#setup arduino serial comm
	try:
		ser = serial.Serial('/dev/ttyATH0', 115200)
	except Exception as msg:
		if 'Windows' in str(msg):
			noSerialFlag = True
			logWrite("Running on windows, assuming debug mode. Printing serial data to STDOUT")
		else:
			logWrite("port in use by another program")
			logWrite(msg[0])
			sys.exit(1)
	else:
                noSerialFlag = False
		print ser.portstr
		ser.open()
	
	#thread main loop
	while True:
		#check for data that needs to be bridged to arduino
		dataFlag = True
		if not serialRealTimeQueue.empty():
			data = serialRealTimeQueue.get
		else:
			dataFlag = False
			
		#if data was recieved send it to arduino
		if dataFlag: 
			if noSerialFlag:
				print data
			else:
				ser.write(data)	
        
		#check if the arduino sent us some data to relay to the host
		if not noSerialFlag:
			readReady = select.select([ser], [], [], 0.01)
			if readReady[0]:
				networkQueue.put(ser.read())	
			
def networkComThread():
	#create socket to recieve from host
	netSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
	netSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	try:
		netSock.bind((LOCAL_IP, PORT))
	except socket.error as msg:
		logWrite('Broadcast bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
		sys.exit(1)
	
	#main thead loop
	while True:
		#wait for data from broadcast listener thread
		logWrite("Waiting for host.")
		host = broadcastQueue.get(True)
		
		#run loop while we continue to get broadcast packets from the host
		fKeepAlive = True
		keepAliveTimer = time.time()
		nameBroadcastTimer = time.time()
		logWrite("Found Host.")
		while fKeepAlive:
			if not broadcastQueue.empty():
				while not broadcastQueue.empty():
					qPop = broadcastQueue.get()
				keepAliveTimer = time.time() 
			#if we haven't recieved a keep alive packet in a while kill the connection
			if time.time() - keepAliveTimer > KEEP_ALIVE_TIMEOUT:
				fKeepAlive = False
				logWrite("Connection Died due to timeout")
				break
				
			#check for data from host and pass it to the serial thread
			ready = select.select([netSock], [], [], 0.005)
			if ready[0]:
				bridgeData, senderAddr = netSock.recvfrom(1024)
				if bridgeData:
					if senderAddr[0] == host[0]:
						serialRealTimeQueue.put(bridgeData)    
		
			#check serial queue and pass to the host if data is available
			if not networkQueue.empty():
				serialData = networkQueue.get()
				netSock.sendto('DBG:' + ROBOT_NAME + ':' + serialData,(host[0], PORT))		
			
			#broadcast name on interval
			if time.time() - nameBroadcastTimer > NAME_BROADCAST_TIMEOUT:
				nameBroadcastTimer = time.time()
				netSock.sendto('ROB:' + ROBOT_NAME,(host[0], PORT))
				logWrite("Sent robot name.")
				
#cleanup function
def cleanup():
	logWrite('cleanup')
	
#Main program start
serialThread = Thread(target=serialComthread, args=())
broadcastThread = Thread(target=broadcastListener, args=())
networkThread = Thread(target=networkComThread, args=())

serialThread.start()
broadcastThread.start()
networkThread.start()

atexit.register(cleanup)

while True:
	if not serialThread.isAlive():
		serialThread.run()
		logWrite("Restarting serial thread")
	if not broadcastThread.isAlive():
		broadcastThread.run()
		logWrite("Restarting broadcast listener thread")
	if not networkThread.isAlive():
		networkThread.run()
		logWrite("Restarting network thread")
		
	time.sleep(1)
	
