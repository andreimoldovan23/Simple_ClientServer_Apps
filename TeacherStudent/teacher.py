from normalStudent import generate_question
import socket
import select
import sys
import random
import string


def generate_array():
    arr = []
    for _ in range(1, 6):
        arr.append(random.randint(1, 101))
    return arr


def generate_answer():
    string_answer = ''.join(random.choices(string.ascii_uppercase + string.digits, k=5))
    array_answer = generate_array()
    return string_answer + " -- " + str(array_answer)


def run_teacher():
    my_sock_from_leaders = []
    try:
        my_sock_from_leaders = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        my_sock_from_leaders.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        my_sock_from_leaders.bind(('', int(sys.argv[1])))
        my_sock_from_leaders.listen(10)
    except socket.error as e:
        print("Error:", e)

    read_set = [my_sock_from_leaders]

    while True:
        readable, _, _ = select.select(read_set, [], [], 60)

        if len(readable) == 0:
            break

        for var in readable:
            if var == my_sock_from_leaders:
                new_sock, _ = my_sock_from_leaders.accept()
                read_set.append(new_sock)

            else:
                try:
                    data = var.recv(1024)
                    new_data = "Received question: " + data.decode('UTF-8') + "\n" + "Answer: " + generate_answer()
                    var.send(new_data.encode('UTF-8'))
                except socket.error:
                    continue

    for var in read_set:
        var.close()

    print("Session over")


run_teacher()