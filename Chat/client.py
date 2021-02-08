import socket
import sys
import select

try:
    my_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    my_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    my_sock.connect((sys.argv[1], int(sys.argv[2])))
except socket.error as e:
    print("Error:", e)
    exit(0)

read_set = [my_sock, sys.stdin]
over = False

while not over:
    readable, _, _ = select.select(read_set, [], [])
    for var in readable:
        if var == my_sock:
            data = my_sock.recv(1024)
            data = data.decode('UTF-8')[0 : len(data) - 1]
            print(data)

            if (data.__contains__("kicked") and data.__contains__("You")) or (data.__contains__("chat") and data.__contains__("over")):
                over = True
                break
        
        else:
            message = sys.stdin.readline()
            message = message[0 : len(message) - 1]
            my_sock.send(message.encode('UTF-8'))

            if message == "QUIT":
                over = True
                break

my_sock.close()