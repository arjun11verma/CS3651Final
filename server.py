import socket
import struct
import wifiutils

localIP     = wifiutils.ip
localPort   = wifiutils.port

bufferSize  = 400
unpackString = '<' + ''.join(['h' for a in range(bufferSize)])

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))
print("UDP server up and listening on " + localIP + " at point " + str(localPort))

# Listen for incoming datagrams
while(True):
    data, addr = UDPServerSocket.recvfrom(bufferSize * 2 + 8)

    if (len(data) == bufferSize * 2):
        soundData = struct.unpack(unpackString, data)