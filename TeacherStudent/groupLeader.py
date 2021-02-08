import socket
import select
import time
import sys


def elapsed_seconds(start_time, limit):
    end_time = time.time()
    if end_time - start_time >= limit:
        return True
    return False


def run_leader(port):
    my_sock_to_teacher = []
    my_sock_from_students = []
    my_sock_to_students = []
    try:
        my_sock_to_students = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        my_sock_from_students = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        my_sock_to_teacher = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        my_sock_to_students.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        my_sock_to_students.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        my_sock_from_students.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        my_sock_to_teacher.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        my_sock_from_students.bind(('', int(sys.argv[3])))
        my_sock_from_students.listen(10)
        my_sock_to_teacher.connect((sys.argv[1], int(sys.argv[2])))
    except socket.error as e:
        print("Error:", e)

    read_set = [my_sock_from_students, my_sock_to_teacher]
    over = False
    start_time_local = time.time()
    start_time_global = time.time()

    while not over:
        readable, _, _ = select.select(read_set, [], [], 0)
        for var in readable:
            if var == my_sock_from_students:
                new_sock, _ = my_sock_from_students.accept()
                read_set.append(new_sock)
            
            elif var == my_sock_to_teacher:
                try:
                    data = my_sock_to_teacher.recv(1024).decode('UTF-8')
                    if len(data) > 0:
                        new_data = "Teacher said: \n" + data
                        my_sock_to_students.sendto(new_data.encode('UTF-8'), ('192.168.0.255', port))
                except socket.error:
                    print("Teacher has left")
                    msg = "Session over"
                    my_sock_to_students.sendto(msg.encode('UTF-8'), ('192.168.0.255', port))
                    over = True
                    break

            else:
                try:
                    data = var.recv(1024)
                except socket.error:
                    continue

                try:
                    my_sock_to_teacher.send(data)
                except socket.error:
                    print("Teacher has left")
                    msg = "Session over"
                    my_sock_to_students.sendto(msg.encode('UTF-8'), ('192.168.0.255', port))
                    over = True
                    break

        if elapsed_seconds(start_time_local, 5) and not over:
            msg = "I am the leader"
            my_sock_to_students.sendto(msg.encode('UTF-8'), ('192.168.0.255', port))
            start_time_local = time.time()

        if elapsed_seconds(start_time_global, 30) and not over:
            msg = "Session over"
            my_sock_to_students.sendto(msg.encode('UTF-8'), ('192.168.0.255', port))
            over = True
            print(msg)

    for var in read_set:
        var.close()
    my_sock_to_students.close()