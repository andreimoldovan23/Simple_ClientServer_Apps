import socket
import sys
import struct


my_sock = []
try:
    my_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    my_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    my_sock.connect((sys.argv[1], int(sys.argv[2])))
except socket.error as e:
    print("Error:", e)

command = ""
for i in range(3, len(sys.argv)):
    command += sys.argv[i] + " "
command = command[0: len(command) - 1]

my_sock.send(command.encode('utf-8'))
answer = my_sock.recv(8000).decode('utf-8')
my_sock.close()

print("Result:\n" + answer)