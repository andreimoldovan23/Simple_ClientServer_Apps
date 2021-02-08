#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define PORT 9999
#define PLAYERS 5

pthread_t threads[4];
pthread_rwlock_t lock;
int guessed = 0;
int number;

void* handleClient(void* sockNo) {
    int sock = sockNo;
    int tries = 0;
    int amWinner = 0;
    uint32_t myNumber;
    char hint[30];
    memset(hint, 0, 31);

    while(1) {
        pthread_rwlock_rdlock(&lock);
        if(guessed == 1) {
            pthread_rwlock_unlock(&lock);
            break;
        }
        pthread_rwlock_unlock(&lock);

        if(recv(sock, &myNumber, sizeof(uint32_t), 0) == -1) {
            pthread_t tid;
            tid = pthread_self();
            
            char message[60];
            memset(message, 0, 61);
            
            sprintf(message, "Error receiving data in thread: %ld\n", tid);
            perror(message);
            break;
        }

        myNumber = ntohl(myNumber);
        tries++;
        if(myNumber < number) {
            strcpy(hint, "G");
            send(sock, hint, strlen(hint) + 1, 0);
        }
        else if(myNumber > number) {
            strcpy(hint, "L");
            send(sock, hint, strlen(hint) + 1, 0);
        }
        else{
            pthread_rwlock_wrlock(&lock);
            if(guessed == 0) {
                guessed = 1;
                amWinner = 1;
            }
            pthread_rwlock_unlock(&lock);
        }
    }

    if(amWinner == 1) {
        sprintf(hint, "You win. %d tries\n", tries);
    }
    else if(tries > 0){
        sprintf(hint, "You lose. %d tries\n", tries);
    }
    else {
        sprintf(hint, "The game is no longer open\n");
    }
    send(sock, hint, strlen(hint) + 1, 0);

    close(sock);
}

int main(int argc, char** argv) {
    pthread_rwlock_init(&lock, NULL);
    memset(threads, 0, 4);

    srand(time(NULL));
    number = rand() % 1000 + 1;
    printf("I've decided upon %d\n", number);

    int serverSock;
    int playerNo = 0;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    int length = sizeof(struct sockaddr_in);

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSock == -1) {
        perror("Error creating socket");
        exit(0);
    }

    int opt = 1;
    if(setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error setting socket option");
        exit(0);
    }

    memset(&serverAddr, 0, length);
    memset(&clientAddr, 0, length);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if(bind(serverSock, &serverAddr, length) == -1) {
        perror("Error binding socket to port");
        exit(0);
    }

    if(listen(serverSock, PLAYERS) == -1) {
        perror("Error listen");
        exit(-1);
    }

    while(1) {
        pthread_rwlock_rdlock(&lock);
        if(guessed == 1) {
            pthread_rwlock_unlock(&lock);
            break;
        }
        pthread_rwlock_unlock(&lock);

        int newPlayer = accept(serverSock, &clientAddr, &length);
        if(newPlayer == -1) {
            perror("Error accepting connection");
            break;
        }
        printf("Player %s : %d connected\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        pthread_create(&threads[playerNo], NULL, handleClient, (void*)newPlayer);
        playerNo++;
    }

    for(int i = 0; i < playerNo; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_rwlock_destroy(&lock);
    close(serverSock);

    return 0;
}