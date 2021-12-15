// ******************************************************************
// Socket template code source:
// https://www.programminglogic.com/sockets-programming-example-in-c-server-converts-strings-to-uppercase/
// *******************************************************************

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "util.h"

void printUsage() {
    printf("1) create filename permision (Ex: rwr---)\n");
    printf("2) read filename\n");
    printf("3) write filename o/a\n");
    printf("4) changemode filename permision (Ex: rw----)\n");
}
int main(void) {
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    char recvbuf[1024];
    char sendbuf[1024];

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portNum);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

    addr_size = sizeof serverAddr;
    connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size);
    recv(clientSocket, recvbuf, 1024, 0);
    printf("Client ID: %s\n", recvbuf);
    printUsage();

    while (1) {
        memset(sendbuf, 0, 1024);
        memset(recvbuf, 0, 1024);
        printf("Input command:\n");
        char temp[1024];
        char ttemp[3][50];
        fgets(sendbuf, 1024, stdin);
        if (strcmp(sendbuf, "exit\n") == 0) {
            send(clientSocket, sendbuf, strlen(sendbuf), 0);
            break;
        }
        strcpy(temp, sendbuf);

        char *token = strtok(temp, " ");
        int count = 0;
        while (token != NULL) {
            strcpy(ttemp[count], token);
            token = strtok(NULL, " ");
            count++;
        }
        if (strcmp(ttemp[0], "create") == 0 || strcmp(ttemp[0], "read") == 0 ||
            strcmp(ttemp[0], "write") == 0 ||
            strcmp(ttemp[0], "changemode") == 0 ||
            strcmp(ttemp[0], "exit") == 0) {
            if (strcmp(ttemp[0], "read") == 0 && count != 2) {
                printUsage();
                continue;
            } else if (strcmp(ttemp[0], "create") == 0 ||
                       strcmp(ttemp[0], "changemode") == 0) {
                if (count != 3) {
                    printUsage();
                    continue;
                }
                if (strlen(ttemp[2]) != 7) {
                    printUsage();
                    continue;
                }
                if (ttemp[2][0] != 'r') {
                    printUsage();
                    continue;
                }
                if (ttemp[2][1] != 'w') {
                    printUsage();
                    continue;
                }
                int flag = 0;
                for (int i = 2; i < 6; i++) {
                    if (i % 2 == 0) {
                        if (ttemp[2][i] != 'r' && ttemp[2][i] != '-') {
                            flag = 1;
                            break;
                        }
                    } else {
                        if (ttemp[2][i] != 'w' && ttemp[2][i] != '-') {
                            flag = 1;
                            break;
                        }
                    }
                }
                if (flag) {
                    printUsage();
                    continue;
                }
            } else if (strcmp(ttemp[0], "write") == 0) {
                if (count != 3) {
                    printUsage();
                    continue;
                }
                if (strlen(ttemp[2]) != 2) {
                    printUsage();
                    continue;
                }
                if (ttemp[2][0] != 'o' && ttemp[2][0] != 'a') {
                    printUsage();
                    continue;
                }
            }
            if (strcmp(ttemp[0], "write") == 0) {
                send(clientSocket, sendbuf, strlen(sendbuf), 0);
                recv(clientSocket, recvbuf, 1024, 0);
                // printf("%d", strcmp(recvbuf, "permit"));
                if (strcmp(recvbuf, "Someone is writing...") == 0 ||
                    strcmp(recvbuf, "Someone is reading...") == 0 ||
                    strcmp(recvbuf, "File is not found...") == 0 ||
                    strcmp(recvbuf, "You have no permission to write...") ==
                        0) {
                    printf("Received from server: %s\n\n", recvbuf);
                    continue;
                } else {
                    printf("Received from server: %s\n", recvbuf);

                    char data[1024];
                    memset(data, 0, 1024);
                    printf("> ");
                    fgets(data, 1024, stdin);
                    send(clientSocket, data, strlen(data), 0);
                    recv(clientSocket, recvbuf, 1024, 0);
                    printf("Received from server: %s\n\n", recvbuf);
                }

            } else if (strcmp(ttemp[0], "read") == 0) {
                send(clientSocket, sendbuf, strlen(sendbuf), 0);
                recv(clientSocket, recvbuf, 1024, 0);
                if (strcmp(recvbuf, "Someone is writing...") == 0 ||
                    strcmp(recvbuf, "File is not found...") == 0 ||
                    strcmp(recvbuf, "You have no permission to read...") == 0) {
                    printf("Received from server: %s\n\n", recvbuf);
                    continue;
                } else {
                    printf("Received from server: %s\n", recvbuf);
                    printf("Reading:\n");
                    char data[1024];
                    memset(data, 0, 1024);
                    recv(clientSocket, data, 1024, 0);

                    printf("*****************************\n");
                    printf("%s", data);
                    FILE *fp;
                    strtok(ttemp[1], "\n");
                    fp = fopen(ttemp[1], "w");
                    // fwrite(data, 1, strlen(data), fp);
                    printf("******************************\n");
                    fprintf(fp, "%s", data);
                    fclose(fp);
                    recv(clientSocket, recvbuf, 1024, 0);
                    printf("Received from server: %s\n\n", recvbuf);
                }

            } else {
                send(clientSocket, sendbuf, strlen(sendbuf), 0);

                recv(clientSocket, recvbuf, 1024, 0);
                printf("Received from server: %s\n\n", recvbuf);
            }
        } else {
            printUsage();
            continue;
        }
    }

    return 0;
}
