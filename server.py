import socket
import struct
import wifiutils
import speechrec
import torch
import models
import time

localIP     = wifiutils.ip
localPort   = wifiutils.port
clientPort  = wifiutils.clientPort

bufferSize  = 400
unpackString = '<' + ''.join(['h' for a in range(bufferSize)])

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))
print("UDP server up and listening on " + localIP + " at point " + str(localPort))

# Speech Recognition
audioBuffer = speechrec.AudioBuffer(bufferSize, 8000)
model = models.SpeechRec()
model.load_state_dict(torch.load('./speechmodel.pt'))
model.eval()

# Testing/Temporary 
num_words = 0
active_word = "MACE"

# Listen for incoming datagrams
while(True):
    data, addr = UDPServerSocket.recvfrom(bufferSize * 2 + 8)

    if (len(data) == bufferSize * 2):
        soundData = struct.unpack(unpackString, data)
        audioBuffer.add_chunk(soundData)

        if (audioBuffer.is_speech):
            model_output = model(audioBuffer.get_scaled_tensor())
            result = torch.argmax(model_output)
            result = result.item()
            
            UDPServerSocket.sendto(str.encode(str(result)), addr)
            audioBuffer.reset_speech()

torch.save(audioBuffer.get_raw_tensor(), './detected_speech.pt')