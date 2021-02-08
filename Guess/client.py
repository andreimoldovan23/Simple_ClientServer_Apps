import socket
import struct
import sys
import random
import time

random.seed(time.time())

try:
    mySock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    mySock.connect((sys.argv[1], int(sys.argv[2])))
except socket.error as e:
    print("Error:" + e)
    exit(0)

low = 1
high = 1000

while True:
    time.sleep(2)
    number = random.randint(low, high + 1)
    print("Sending " + str(number))
    mySock.send(struct.pack("!I", number))

    answer = mySock.recv(31)
    answer = answer.decode('UTF-8')[0 : len(answer) - 1]

    if answer == "G":
        low = number + 1
    elif answer == "L":
        high = number - 1
    else:
        print(answer)
        break

mySock.close()
