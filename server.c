// ******************************************************************
// Socket template code source:
// https://www.programminglogic.com/sockets-programming-example-in-c-server-converts-strings-to-uppercase/
// *******************************************************************
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>  // pthread.kill()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util.h"

pid_t pid;
pthread_t tid[10];
pthread_attr_t attr;
struct capList domain[2][10];  // domain[0]: AOS, domain[1]: CSE
struct fileInfo fi[10];        // Maximum 10 file on server
int count = 0;
int aos_count = 0;
int cse_count = 0;
int fidx = 0;  // File index

void showFileInfo() {
    printf("********** Showing file info **************\n");
    for (int i = 0; i < fidx; i++) {
        printf("File name: %s\n", fi[i].fname);
        printf("File owner: %s\n", fi[i].owner);

        if (i != fidx - 1) {
            printf("\n");
        }
    }
    printf("********************************************\n");
}
void showCapability() {
    printf("********** Showing capability **************\n");
    printf("AOS: ");
    for (int i = 0; i < fidx; i++) {
        printf("%s | %d | %d -> ", domain[0][i].fname, domain[0][i].read,
               domain[0][i].write);
        if (i == fidx - 1) printf("NULL");
    }
    printf("\nCSE: ");
    for (int i = 0; i < fidx; i++) {
        printf("%s | %d | %d -> ", domain[1][i].fname, domain[1][i].read,
               domain[1][i].write);
        if (i == fidx - 1) printf("NULL");
    }
    printf("\n********************************************\n");
}
void handleCreate(int sockfd, char *name, int group, char *fname, char *arg) {
    // printf("Name: %s\n", name);
    // printf("Group ID: %d\n", group);
    // printf("Filename: %s\n", fname);
    // printf("Permission: %s\n", arg);

    // Check file is exist or not
    char recvbuf[1024];
    char sendbuf[1024];
    memset(sendbuf, 0, 1024);
    memset(recvbuf, 0, 1024);
    for (int i = 0; i < fidx; i++) {
        if (strcmp(fname, fi[i].fname) == 0) {
            printf("%s\n", "The file is exist...");
            strcpy(sendbuf, "The file is exist...");
            printf("Denied...\n");
            send(sockfd, sendbuf, strlen(sendbuf), 0);
            return;
        }
    }
    printf("Accepted...\n");
    // Store file information
    strcpy(fi[fidx].fname, fname);
    // printf("%d\n", strcmp(fi[fidx].fname, fname));
    strcpy(fi[fidx].owner, name);
    fi[fidx].rcount = 0;
    fi[fidx].wcount = 0;

    // Handling capability list
    // Same group with owner
    strcpy(domain[group][fidx].fname, fname);
    if (arg[2] == 'r') {
        domain[group][fidx].read = 1;
    } else {
        domain[group][fidx].read = 0;
    }
    if (arg[3] == 'w') {
        domain[group][fidx].write = 1;
    } else {
        domain[group][fidx].write = 0;
    }

    // Other group
    strcpy(domain[!group][fidx].fname, fname);
    if (arg[4] == 'r') {
        domain[!group][fidx].read = 1;
    } else {
        domain[!group][fidx].read = 0;
    }
    if (arg[5] == 'w') {
        domain[!group][fidx].write = 1;
    } else {
        domain[!group][fidx].write = 0;
    }
    fidx = fidx + 1;
    showFileInfo();
    printf("\n");
    showCapability();

    FILE *fp;
    fp = fopen(fname, "w");
    char content[256];
    sprintf(content, "Created by %s\n", name);
    // fwrite(content, 1, strlen(content), fp);
    fprintf(fp, "%s", content);
    fclose(fp);
    printf("Create Finish...\n");
    strcpy(sendbuf, "Create Finish...");
    send(sockfd, sendbuf, strlen(sendbuf), 0);
    printf("Procedure of %s Finish...\n", name);
}
void handleRead(int sockfd, char *name, int group, char *fname) {
    // printf("Name: %s\n", name);
    // printf("Filename: %s\n", fname);
    char recvbuf[1024];
    char sendbuf[1024];
    memset(sendbuf, 0, 1024);
    memset(recvbuf, 0, 1024);
    strtok(fname, "\n");
    int i;
    for (i = 0; i < fidx; i++) {
        if (strcmp(fname, fi[i].fname) == 0) {
            printf("File is found...\n");
            break;
        }
    }
    if (i == fidx) {
        printf("File is not found...\n");
        printf("Denied...\n");

        strcpy(sendbuf, "File is not found...");
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        return;
    }
    if (strcmp(fi[i].owner, name) != 0) {
        printf("%s is not the owner...\n", name);
        if (domain[group][i].read != 1) {
            printf("%s has no permission to read\n", name);
            printf("Denied...\n");
            strcpy(sendbuf, "You have no permission to read...");
            send(sockfd, sendbuf, strlen(sendbuf), 0);
            return;
        } else {
            printf("%s has permission to read...\n", name);
        }
    } else {
        printf("%s is the owner...\n", name);
    }

    if (fi[i].wcount != 0) {
        printf("Someone is reading...\n");
        strcpy(sendbuf, "Someone is writing...");
        printf("Denied...\n");
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        return;
    }
    printf("Accepted...\n");
    strcpy(sendbuf, "You can read...");
    send(sockfd, sendbuf, strlen(sendbuf), 0);
    fi[i].rcount++;
    sleep(10);
    fi[i].rcount--;

    FILE *fp;
    char data[1024];
    memset(data, 0, 1024);

    fp = fopen(fname, "r");
    fread(data, 1, 1024, fp);
    fclose(fp);
    send(sockfd, data, strlen(data), 0);

    sleep(1);
    printf("Read Finish...\n");
    strcpy(sendbuf, "Read Finish...");
    send(sockfd, sendbuf, strlen(sendbuf), 0);
    printf("Procedure of %s Finish...\n", name);
}
void handleWrite(int sockfd, char *name, int group, char *fname, char *arg) {
    // printf("Name: %s\n", name);
    // printf("Filename: %s\n", fname);
    // printf("Write Type: %s\n", arg);
    char recvbuf[1024];
    char sendbuf[1024];
    memset(sendbuf, 0, 1024);
    memset(recvbuf, 0, 1024);
    int i;
    for (i = 0; i < fidx; i++) {
        if (strcmp(fname, fi[i].fname) == 0) {
            printf("File is found...\n");

            break;
        }
    }
    if (i == fidx) {
        printf("File is not found...\n");
        printf("Denied...\n");

        strcpy(sendbuf, "File is not found...");
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        return;
    }
    if (strcmp(fi[i].owner, name) != 0) {
        printf("%s is not the owner...\n", name);
        if (domain[group][i].write != 1) {
            printf("%s has no permission to write\n", name);
            printf("Denied...\n");
            strcpy(sendbuf, "You have no permission to write...");
            send(sockfd, sendbuf, strlen(sendbuf), 0);
            return;
        } else {
            printf("%s has permission to write...\n", name);
        }
    } else {
        printf("%s is the owner...\n", name);
    }

    if (fi[i].rcount != 0) {
        printf("Someone is reading...\n");
        printf("Denied...\n");
        strcpy(sendbuf, "Someone is reading...");
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        return;
    }
    if (fi[i].wcount != 0) {
        printf("Someone is writing...\n");
        printf("Denied...\n");
        strcpy(sendbuf, "Someone is writing...");
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        return;
    }
    fi[i].wcount++;
    printf("Accepted...\n");
    strcpy(sendbuf, "You can write...");
    send(sockfd, sendbuf, strlen(sendbuf), 0);

    char data[1024];
    memset(data, 0, 1024);
    recv(sockfd, data, 1024, 0);

    FILE *fp;
    if (arg[0] == 'a') {
        printf("Writing to %s (appending): %s", fname, data);
        fp = fopen(fname, "a");
    } else {
        printf("Writing to %s (overwritting): %s", fname, data);
        fp = fopen(fname, "w");
    }

    fprintf(fp, "%s", data);
    fclose(fp);
    printf("Write Finish...\n");
    strcpy(sendbuf, "Write Finish...");

    send(sockfd, sendbuf, strlen(sendbuf), 0);
    fi[i].wcount--;
    printf("Procedure of %s Finish...\n", name);
}
void handleChangemode(int sockfd, char *name, int group, char *fname,
                      char *arg) {
    char recvbuf[1024];
    char sendbuf[1024];
    memset(sendbuf, 0, 1024);
    memset(recvbuf, 0, 1024);
    int i;
    for (i = 0; i < fidx; i++) {
        if (strcmp(fi[i].fname, fname) == 0) {
            break;
        }
    }

    if (i == fidx) {
        printf("Flie is not found\n");
        strcpy(sendbuf, "File is not found...");
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        printf("Denied...\n");

        return;
    }

    if (strcmp(fi[i].owner, name) != 0) {
        printf("%s is not the owner\n", name);
        strcpy(sendbuf, "You are not the owner...");
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        printf("Denied...\n");

        return;
    } else {
        printf("%s is the owner\n", name);
    }
    printf("Accepted...\n");
    // Handling capability list
    // Same group with owner
    if (arg[2] == 'r') {
        domain[group][i].read = 1;
    } else {
        domain[group][i].read = 0;
    }
    if (arg[3] == 'w') {
        domain[group][i].write = 1;
    } else {
        domain[group][i].write = 0;
    }

    // Other group
    if (arg[4] == 'r') {
        domain[!group][i].read = 1;
    } else {
        domain[!group][i].read = 0;
    }
    if (arg[5] == 'w') {
        domain[!group][i].write = 1;
    } else {
        domain[!group][i].write = 0;
    }
    showCapability();
    printf("Changemode Finish...\n");
    strcpy(sendbuf, "Changemode Finish...");
    send(sockfd, sendbuf, strlen(sendbuf), 0);
    printf("Procedure of %s Finish...\n", name);
}

void run(void *arg) {
    int sockfd = *(int *)arg;
    int nBytes = 1;

    // Give client name with group
    char name[20];
    int group;

    char recvbuf[1024];
    char sendbuf[1024];

    if (count % 2 == 1) {
        sprintf(name, "AOS-%d", aos_count);
        group = 0;
    } else {
        sprintf(name, "CSE-%d", cse_count);
        group = 1;
    }

    strcpy(sendbuf, name);
    send(sockfd, sendbuf, strlen(sendbuf), 0);
    /*loop while connection is live*/
    while (1) {
        memset(sendbuf, 0, 1024);
        memset(recvbuf, 0, 1024);

        char oper[20];  // operation
        char f[20];     // filename
        char arg[20];   // permission (create / chmod) or a/o (write)
        nBytes = recv(sockfd, recvbuf, 1024, 0);
        if (strcmp(recvbuf, "exit\n") == 0) {
            return;
        }
        printf("\n%s: %s", name, recvbuf);
        // printf("%s: ", name);
        // puts(recvbuf);
        // Operation
        char *token = strtok(recvbuf, " ");
        strcpy(oper, token);

        // Filename
        token = strtok(NULL, " ");
        strcpy(f, token);

        if (strcmp(oper, "read") != 0) {
            token = strtok(NULL, " ");
            strcpy(arg, token);
        }

        // printf("Operation: %s\n", oper);
        // printf("Filename: %s\n", f);
        // if (strcmp(oper, "read") != 0)
        //     printf("Permission: %s\n", permit);
        if (strcmp(oper, "create") == 0) {
            handleCreate(sockfd, name, group, f, arg);
        } else if (strcmp(oper, "read") == 0) {
            handleRead(sockfd, name, group, f);
        } else if (strcmp(oper, "write") == 0) {
            handleWrite(sockfd, name, group, f, arg);
        } else if (strcmp(oper, "changemode") == 0) {
            handleChangemode(sockfd, name, group, f, arg);
        }
        // strcpy(sendbuf, "Parsing Finish...");
        // send(newSocket, sendbuf, strlen(sendbuf), 0);
    }
}

int thread_check(pthread_t tid) {
    int pthread_kill_err;
    pthread_kill_err = pthread_kill(tid, 0);

    if (pthread_kill_err == ESRCH)  // thread   is  not   exist
        return 0;
    else if (pthread_kill_err == EINVAL)  // the signal is  illeagal
        return -1;

    return 1;  // thread is alive
}
int main() {
    int welcomeSocket, newSocket, clientLen, nBytes;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    int i;

    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portNum);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    bind(welcomeSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (listen(welcomeSocket, 10) == 0)
        printf("Listening\n");
    else
        printf("Error\n");

    addr_size = sizeof(serverStorage);
    if (pthread_attr_init(&attr) != 0) printf("initial_attr_error");
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
        printf("detached_state_error");
    /*loop to keep accepting new connections*/
    while (1) {
        newSocket = accept(welcomeSocket, (struct sockaddr *)&serverStorage,
                           &addr_size);
        if (newSocket < 0) {
            perror("Accept error!");
            exit(1);
        }
        count = count + 1;
        if (count % 2 == 1) {
            aos_count = aos_count + 1;
        } else {
            cse_count = cse_count + 1;
        }

        for (int i = 0; i < 10; i++) {
            if (tid[i] == 0 || thread_check(tid[i]) == 0) {
                if (pthread_create(&tid[i], &attr, (void *(*)(void *))run,
                                   &newSocket) < 0)
                    printf("pthread_create_error");
                break;
            }
        }
    }

    return 0;
}