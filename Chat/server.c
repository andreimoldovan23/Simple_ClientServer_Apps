#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define tcpPORT 9999
#define RoomCapacity 20

void closeAll(fd_set* set, int size) {
    for(int i = 1; i <= size; i++) {
        if(FD_ISSET(i, set)) {
            FD_CLR(i, set);
            close(i);
        }
    }
    FD_CLR(0, set);
}

void sendAll(fd_set set, int size, int except, char* msg) {
    for(int i = 1; i <= size; i++) {
        if(FD_ISSET(i, &set) && (i != except)) {
            send(i, msg, strlen(msg) + 1, 0);
        }
    }
}

int getKicked(char* msg) {
    int number = -1, tokens = 1;
    char* token;
    token = (char*)malloc(50 * sizeof(char));
    token = strtok(msg, " ");

    while(token != NULL) {
        token = strtok(NULL, " ");
        tokens++;
        if(token != NULL)
            number = atoi(token);
    }

    free(token);
    if(number == 0 || tokens > 3)
        return -1;
    return number;
}

int main(int argc, char** argv) {
    char welcomeMessage[60], introMessage[60];
    int over = 0;
    fd_set master, readSet;
    int fdMax, opt = 1;
    int serverSock;
    struct sockaddr_in serverAddr, clientAddr;
    int structSize = sizeof(serverAddr);

    if((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating receiver socket");
        exit(0);
    }

    if(setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error setting socket reuse option");
        exit(0);
    }

    memset(welcomeMessage, 0, 61);
    memset(introMessage, 0, 61);
    memset(&serverAddr, 0, structSize);
    memset(&clientAddr, 0, structSize);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(tcpPORT);

    if(bind(serverSock, &serverAddr, structSize) == -1) {
        perror("Error binding receiver socket to port");
        exit(0);
    }

    if(listen(serverSock, RoomCapacity) == -1) {
        perror("Error with listen");
        exit(0);
    }

    FD_SET(serverSock, &master);
    FD_SET(0, &master);
    fdMax = serverSock;

    while(over == 0) {
        readSet = master;
        if(select(fdMax + 1, &readSet, NULL, NULL, NULL) == -1) {
            perror("Error with select");
            exit(0);
        }

        for(int i = 0; i < fdMax + 1; i++) {
            if(FD_ISSET(i, &readSet)) {
                if(i == serverSock) {
                    int newSock = accept(serverSock, &clientAddr, &structSize);
                    if(newSock == -1) {
                        perror("Error accepting connection");
                        continue;
                    }


                    sprintf(introMessage, "Client %d, from %s : %d has entered the chat", newSock, 
                        inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                    sendAll(master, fdMax, serverSock, introMessage);

                    FD_SET(newSock, &master);
                    if(newSock > fdMax)
                        fdMax = newSock;

                    sprintf(welcomeMessage, "Welcome to the chat! You are client: %d", newSock);
                    send(newSock, welcomeMessage, strlen(welcomeMessage) + 1, 0);
                }

                else if(i == 0) {
                    char input[10];
                    memset(input, 0, 11);
                    read(0, input, 11);

                    if(strcmp(input, "EXIT\n") == 0) {
                        char sendMsg[50];
                        memset(sendMsg, 0, 51);
                        sprintf(sendMsg, "The chat is over");
                        sendAll(master, fdMax, serverSock, sendMsg);
                        closeAll(&master, fdMax);
                        over = 1;
                        break;
                    }

                    else if(strstr(input, "KICK")) {
                        char kickMsg[50], sendMsg[50];
                        int kicked = getKicked(input);

                        memset(kickMsg, 0, 51);
                        sprintf(kickMsg, "You have been kicked");
                        send(kicked, kickMsg, strlen(kickMsg) + 1, 0);

                        FD_CLR(kicked, &master);
                        close(kicked);

                        memset(sendMsg, 0, 51);
                        sprintf(sendMsg, "<%d> has been kicked", kicked);
                        sendAll(master, fdMax, serverSock, sendMsg);
                    }

                    else {
                        printf("Invalid command!\n");
                    }
                }

                else {
                    char recvMsg[200], sendMsg[250];
                    memset(recvMsg, 0, 201);
                    memset(sendMsg, 0, 251);
                    recv(i, recvMsg, 201, 0);

                    if(strcmp(recvMsg, "QUIT") == 0) {
                        FD_CLR(i, &master);
                        close(i);
                        sprintf(sendMsg, "<%d> has left the chat", i);
                    }

                    else{
                        sprintf(sendMsg, "<%d> %s", i, recvMsg);
                    }
                    sendAll(master, fdMax, serverSock, sendMsg);
                }
            }
        }
    }
}