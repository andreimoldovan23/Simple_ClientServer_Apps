from normalStudent import *
from groupLeader import *


def read_port():
    while True:
        try:
            group_port = int(input("Input the port: "))
            if group_port < 5000 or group_port > 10000:
                raise RuntimeError("The port can have values only between 5000 and 10000")
            return group_port
        except ValueError as ve:
            print("The port must be an integer")
        except RuntimeError as re:
            print(str(re))


def read_leader():
    possible_values_true = ['true','t', 'y', 'yes']
    possible_values_false = ['n', 'no', 'f', 'false']
    while True:
        am_leader = input("Am I the leader? --")
        if am_leader.lower() in possible_values_true:
            return True
        elif am_leader.lower() in possible_values_false:
            return False
        else:
            print("Invalid input. Hint: true, false, yes, no")


group_port = read_port()
group_leader = read_leader()


if group_leader:
    run_leader(group_port)
else:
    run_student(group_port)
