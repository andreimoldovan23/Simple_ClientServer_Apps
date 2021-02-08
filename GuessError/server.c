#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PORT 9999
#define CLIENTS 5

struct client {
    int sockFd;
    float error;
};

fd_set master, readSet;
int fdmax;
struct timeval timeout;
float number;
struct client clients[CLIENTS];
int currentClient = 0;

int getMax(struct client* clientList, int size, float comparator) {
    float minError = (float)RAND_MAX;
    int id = -1;
    for(int i = 0; i < size; i++) {
        if(clientList[i].error < minError) {
            minError = clientList[i].error;
            id = i;
        }
    }
    return id;
}

void sendAnswer(struct client* clientList, int size, int winner) {
    char message[50];
    memset(message, 0, 51);
    for(int i = 0; i < size; i++) {
        if(i == winner) {
            sprintf(message, "You win with an error of: %5f\n", clientList[i].error);
        }
        else {
            sprintf(message, "You lose with an error of: %5f\n", clientList[i].error);
        }
        send(clientList[i].sockFd, message, strlen(message) + 1, 0);
    }
}

void closeAll(fd_set* fdSet, int size) {
    for(int i = 0; i < size; i++) {
        if(FD_ISSET(i, fdSet)) {
            FD_CLR(i, fdSet);
            close(i);
        }
    }
}

int main(int argc, char** argv) {
    srand(time(NULL));
    number = (float)rand()/(float)RAND_MAX * 1.5;
    printf("My number is: %5f\n", number);

    int mySock;
    struct sockaddr_in myAddr, clientAddr;
    int structSize = sizeof(struct sockaddr_in);

    if((mySock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(0);
    }

    memset(&timeout, 0, sizeof(struct timeval));
    timeout.tv_sec = 10;

    memset(&myAddr, 0, structSize);
    memset(&clientAddr, 0, structSize);
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons(PORT);

    int opt = 1;
    if(setsockopt(mySock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error setting socket option");
        exit(0);
    }

    if(bind(mySock, &myAddr, structSize) == -1) {
        perror("Error binding socket to port");
        exit(0);
    }

    if(listen(mySock, CLIENTS) == -1) {
        perror("Error with listen");
        exit(0);
    }

    FD_SET(mySock, &master);
    fdmax = mySock;

    while(1) {
        readSet = master;
        int nfds;
        if((nfds = select(fdmax + 1, &readSet, NULL, NULL, &timeout)) == -1) {
            perror("Error with select");
            exit(0);
        }

        if(nfds == 0) {
            int winner = getMax(clients, currentClient, number);
            sendAnswer(clients, currentClient, winner);
            break;
        }

        for(int i = 0; i < fdmax + 1; i++) {
            if(FD_ISSET(i, &readSet)) {
                if(i == mySock) {
                    int newClient = accept(mySock, &clientAddr, &structSize);
                    if(newClient == -1) {
                        perror("Error accepting connection");
                        exit(0);
                    }
                    FD_SET(newClient, &master);
                    if(newClient > fdmax)
                        fdmax = newClient;
                    printf("Incoming connection from %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                }
                else {
                    char buf[20];
                    recv(i, &buf, 21, 0);
                    float recvNumber = atof(buf);
                    recvNumber -= number;
                    if(recvNumber < 0.00)
                        recvNumber *= -1.00;
                    clients[currentClient].sockFd = i;
                    clients[currentClient].error = recvNumber;
                    currentClient++;
                }
            }
        }
    }

    closeAll(&master, fdmax + 1);
    return 0;
}