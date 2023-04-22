#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_LENGTH 60

int main(int argc, char *argv[]){
    // Tạo socket
    int sender = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender < 0){
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server receiver
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    FILE *fp = fopen(argv[3], "rb");
    if (fp == NULL){
        perror("fopen() failed");
        exit(EXIT_FAILURE);
    }

    char buf[MAX_LENGTH + 31];
    memset(buf, 0, MAX_LENGTH + 31);
    //Them ten file vao buf
    strcat(buf, argv[4]);
    //Noi ki tu * vao duoi chuoi
    strcat(buf, "*");
    int pos = strlen(buf);
    while (fgets(buf + pos, MAX_LENGTH + 1, fp) != NULL){
        printf("Sending %ld bytes: %s\n", strlen(buf), buf);

        if (sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) != strlen(buf)){
            perror("sendto() failed");
            exit(EXIT_FAILURE);
        }

        memset(buf + pos, 0, MAX_LENGTH + 31 - pos);
        usleep(1000000);
    }

    printf("Send successfully");
    fclose(fp);
    close(sender);

    return 0;
}