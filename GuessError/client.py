import socket
import sys
import random
import time
import struct

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((sys.argv[1], int(sys.argv[2])))
except socket.error as e:
    print("Error: " + e)
    exit(0)

random.seed(time.time())
number = random.random()
print("I'm sending " + str(number))

sock.send(str(number).encode('UTF-8'))
answer = sock.recv(31).decode('UTF-8')
answer = answer[0: len(answer) - 1]
print(answer)

sock.close()