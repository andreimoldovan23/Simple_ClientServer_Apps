import socket
import random
import string
import select
import time
import sys


def generate_question():
    number = random.uniform(0.00, 1.00)
    if number > 0.5:
        return ''.join(random.choices(string.ascii_uppercase + string.digits, k=5))
    return ""


def elapsed_seconds(start_time, limit):
    end_time = time.time()
    if end_time - start_time >= limit:
        return True
    return False


def run_student(port):
    my_sock_from_leader = []
    my_sock_to_leader = []

    try:
        my_sock_from_leader = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        my_sock_to_leader = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        my_sock_to_leader.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        my_sock_from_leader.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        my_sock_from_leader.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

        my_sock_from_leader.bind(('', port))
    except socket.error as e:
        print("Error:", e)
        exit(0)

    read_set = [my_sock_from_leader]
    know_leader_address = False
    start_time = time.time()
    start_global_time = time.time()
    over = False

    while not over:
        readable, _, _ = select.select(read_set, [], [], 0)
        for var in readable:
            if var == my_sock_from_leader:
                try:
                    data, addr = my_sock_from_leader.recvfrom(1024)
                except socket.error:
                    print("The leader has left")
                    over = True
                    break

                if not know_leader_address:
                    ip = addr[0]
                    try:
                        my_sock_to_leader.connect((ip, int(sys.argv[3])))
                    except socket.error as e:
                        print("Error:", e)
                        exit(0)
                    know_leader_address = True

                print("Leader said:", data.decode('UTF-8'), "\n")
                if data.decode('UTF-8') == "Session over":
                    over = True
                    break

        if elapsed_seconds(start_time, 3) and know_leader_address and not over:
            question = generate_question()
            if question != "":
                print("My question is:", question)
                try:
                    my_sock_to_leader.send(question.encode('UTF-8'))
                except socket.error:
                    print("The leader has left")
                    over = True
            start_time = time.time()

        if elapsed_seconds(start_global_time, 5) and not know_leader_address:
            print("The leader has left")
            over = True

    my_sock_to_leader.close()
    my_sock_from_leader.close()

