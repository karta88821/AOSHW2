#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "common.h"

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
        char sep_cmd[3][50];
        fgets(sendbuf, 1024, stdin);
        if (strcmp(sendbuf, "exit\n") == 0) {
            send(clientSocket, sendbuf, strlen(sendbuf), 0);
            break;
        }
        strcpy(temp, sendbuf);

        char *token = strtok(temp, " ");
        int count = 0;
        while (token != NULL) {
            strcpy(sep_cmd[count], token);
            token = strtok(NULL, " ");
            count++;
        }

        /* Check if the operation is valid or not */
        if (strcmp(sep_cmd[0], "create") == 0 || strcmp(sep_cmd[0], "read") == 0 ||
            strcmp(sep_cmd[0], "write") == 0 ||
            strcmp(sep_cmd[0], "changemode") == 0 ||
            strcmp(sep_cmd[0], "exit") == 0) {
            if (strcmp(sep_cmd[0], "read") == 0 && count != 2) {
                printUsage();
                continue;
            } else if (strcmp(sep_cmd[0], "create") == 0 || strcmp(sep_cmd[0], "changemode") == 0) {
                if (count != 3) {
                    printUsage();
                    continue;
                }
                if (strlen(sep_cmd[2]) != 7) {
                    printUsage();
                    continue;
                }
                if (sep_cmd[2][0] != 'r') {
                    printUsage();
                    continue;
                }
                if (sep_cmd[2][1] != 'w') {
                    printUsage();
                    continue;
                }
                /* Check permission is valid or not */
                int flag = 0;
                for (int i = 2; i < 6; i++) {
                    if (i % 2 == 0) {
                        if (sep_cmd[2][i] != 'r' && sep_cmd[2][i] != '-') {
                            flag = 1;
                            break;
                        }
                    } else {
                        if (sep_cmd[2][i] != 'w' && sep_cmd[2][i] != '-') {
                            flag = 1;
                            break;
                        }
                    }
                }
                if (flag) {
                    printUsage();
                    continue;
                }
            } else if (strcmp(sep_cmd[0], "write") == 0) {
                if (count != 3) {
                    printUsage();
                    continue;
                }
                if (strlen(sep_cmd[2]) != 2) {
                    printUsage();
                    continue;
                }
                if (sep_cmd[2][0] != 'o' && sep_cmd[2][0] != 'a') {
                    printUsage();
                    continue;
                }
            }

            /* Handle write operation */
            if (strcmp(sep_cmd[0], "write") == 0) {
                send(clientSocket, sendbuf, strlen(sendbuf), 0);
                recv(clientSocket, recvbuf, 1024, 0);
                if (strcmp(recvbuf, "Someone is writing...") == 0 ||
                    strcmp(recvbuf, "Someone is reading...") == 0 ||
                    strcmp(recvbuf, "File is not found...") == 0 ||
                    strcmp(recvbuf, "You have no permission to write...") == 0) {
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
            } 
            /* Handle read operation */
            else if (strcmp(sep_cmd[0], "read") == 0) {
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
                    strtok(sep_cmd[1], "\n");
                    fp = fopen(sep_cmd[1], "w");
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
