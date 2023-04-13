import socket
import struct
import wifiutils
import speechrec

localIP     = wifiutils.ip
localPort   = wifiutils.port
clientPort = wifiutils.clientPort

bufferSize  = 400
unpackString = '<' + ''.join(['h' for a in range(bufferSize)])

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))
print("UDP server up and listening on " + localIP + " at point " + str(localPort))

# Instantiate Audio Buffer
audioBuffer = speechrec.AudioBuffer(bufferSize)

# Listen for incoming datagrams
while(True):
    data, addr = UDPServerSocket.recvfrom(bufferSize * 2 + 8)

    if (len(data) == bufferSize * 2):
        soundData = struct.unpack(unpackString, data)
        audioBuffer.add_chunk(soundData)

        if (False):
            UDPServerSocket.sendto(str.encode('hello'), addr)


# TODO: Our project is special because we only have to recognize certain words, and it will always do that.
# Once a word is spoken, it will catch it and go ham on it. Figure out how the signal changes from quietness to speech
# See how long speech lasts and how we can recognize a change in the signal quickly
# Once sound is detected, we should wait for it to finish and then immediately classify it. I don't care if classification
# takes a bit of a long time and causes the server to freeze for too long. 