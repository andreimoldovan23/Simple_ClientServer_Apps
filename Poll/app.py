import socket
import time
import datetime
import select
import re


max_size = 10
port = 7777
peers = {}
malformed_queue = []
broadcast_number = 0

start_time_querry = time.time()
start_date_querry = time.time()
global_time = time.time()


def elapsed_time(start, limit):
    if time.time() - start >= limit:
        return True
    return False


def get_socket(port):
    try:
        my_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        my_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        my_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        my_sock.bind(("", port))
        return my_sock
    except socket.error as e:
        print("Error:", e)
        exit(0)


def send_timequerry_request(sock, port, start, number):
    if elapsed_time(start, 3):
        data = "TIMEQUERRY"
        sock.sendto(data.encode('UTF-8'), ("192.168.0.255", port))
        start = time.time()
        number += 1
    return start, number


def send_datequerry_request(sock, port, start, number):
    if elapsed_time(start, 10):
        data = "DATEQUERRY"
        sock.sendto(data.encode('UTF-8'), ("192.168.0.255", port))
        start = time.time()
        number += 1
    return start, number


def eliminate_absent_peers(dict, comparator, limit):
    new_dict = {}
    for var in dict:
        val = dict[var][1]
        if comparator - val < limit:
            new_dict[var] = dict[var]
    return new_dict


def print_dict(dict):
    print("\n\n-----Peers-----\n\n")
    for var in dict:
        print(var, dict[var][0])


def display_malformed(malformed):
    print("\n\n-----Malformed Requests-----\n\n")
    if len(malformed) > max_size:
        low = len(malformed) - max_size
    else:
        low = 0
    high = len(malformed)
    for i in range(low, high):
        print(malformed[i])


def check_message(msg):
    tokens = msg.split()
    if len(tokens) != 2:
        return False
    if tokens[0].lower() != "time:" and tokens[0].lower() != "date:":
        return False
    if re.search("[^0-9:/]", tokens[1]):
        return False
    return True


def sync_datetime(port):
    sock = get_socket(port)
    read_set = [sock]

    while True:
        start_time_querry, broadcast_number = send_timequerry_request(sock, port, start_time_querry, broadcast_number)
        start_date_querry, broadcast_number = send_datequerry_request(sock, port, start_date_querry, broadcast_number)

        if elapsed_time(global_time, 120):
            break

        readable, _, _ = select.select(read_set, [], [], 0)
        for var in readable:
            if var == sock:
                data, addr = sock.recvfrom(1024)
                data = data.decode('UTF-8')

                if data == "TIMEQUERRY":
                    now_time = "TIME: " + datetime.datetime.now().strftime("%H:%M:%S")
                    sock.sendto(now_time.encode('UTF-8'), addr)

                elif data == "DATEQUERRY":
                    now_date = "DATE: " + datetime.datetime.now().strftime("%d/%m/%Y")
                    sock.sendto(now_date.encode('UTF-8'), addr)

                else:
                    addr = str(addr[0]) + ":" + str(addr[1])
                    if check_message(data):
                        if not addr in peers:
                            peers[addr] = [data, 1]
                        else:
                            val = peers[addr][1]
                            peers[addr] = [data, val + 1]
                    
                    else:
                        malformed_queue.append(addr + " " + data)

                    peers = eliminate_absent_peers(peers, broadcast_number, 3)
                    print_dict(peers)
                    display_malformed(malformed_queue)
                 
    sock.close()


sync_datetime(port)