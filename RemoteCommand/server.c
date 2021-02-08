#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PORT 9999

void handler(int sigNum) {
    wait(0);
}

int split(char* cmd, char** tokens) {
    int i = 0;
    tokens[i] = strtok(cmd, " ");
    i++;

    while(tokens[i - 1] != NULL) {
        tokens[i] = strtok(NULL, " ");
        i++;
    }
    return i;
}

int main(int argc, char** argv) {
    signal(SIGCHLD, handler);

    int serverSock, structSize = sizeof(struct sockaddr_in);
    struct sockaddr_in serverAddr, clientAddr;

    memset(&serverAddr, 0, structSize);
    memset(&clientAddr, 0, structSize);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(0);
    } 

    int opt = 1;
    if(setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error setting socket option");
        exit(0);
    }

    if(bind(serverSock, &serverAddr, structSize) == -1) {
        perror("Error binding socket to port");
        exit(0);
    }

    if(listen(serverSock, 5) == -1) {
        perror("Error with listen");
        exit(0);
    }

    while(1) {
        int newSock = accept(serverSock, &clientAddr, &structSize);
        if(newSock == -1) {
            perror("Error accepting connection");
            continue;
        }
        
        printf("Client %s:%d connected\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        int pid;
        if((pid = fork()) == -1) {
            perror("Error creating child process");
            close(newSock);
        }
        else if(pid == 0) {
            signal(SIGCHLD, SIG_IGN);
                
            char command[50];
            char** tokens = (char**)malloc(3 * sizeof(char*));
            memset(command, 0, 51);
            memset(tokens, 0, 3 * sizeof(char*));
            recv(newSock, command, 51, 0);
            int size = split(command, tokens);

            int newPid;
            if((newPid = fork()) == -1) {
                perror("Error creating child process");
                exit(0);
            }

            else if (newPid == 0) {
                dup2(newSock, 1);
                execvp(tokens[0], tokens);
            }

            wait(0);
            free(tokens);
            close(newSock);
            exit(0);
        }
    }
    return 0;
}