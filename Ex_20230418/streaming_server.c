#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define MAX_CLIENT 10

int main(int argc, char *argv[])
{
    // Thiết lập thông tin địa chỉ cho socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(9000);

    // Tao socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Gan dia chi cho socket
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // Lang nghe ket noi
    if (listen(server, MAX_CLIENT) == -1)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    // Chap nhan ket noi cuar client
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    int client = accept(server, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client == -1)
    {
        perror("accept() failed");
        exit(EXIT_FAILURE);
    }

    // Nhan du lieu tu client
    int count = 0;
    int i;
    char bufRecv[2048];
    memset(bufRecv, 0, 2048);
    while (1)
    {
        int ret = recv(client, bufRecv, sizeof(bufRecv), 0);
        bufRecv[ret] = '\0';
        char subStr[] = "0123456789";
        if (ret <= 0){
            break;
        }
        
        if (ret < sizeof(bufRecv)){
            bufRecv[ret] = 0;
        }

        int len_str = strlen(bufRecv);
        int len_subtr = strlen(subStr);

        if(len_str < len_subtr){
            break;
        }

        for(i = 0; i <= len_str - len_subtr; i++){
            if(strncmp(bufRecv + i, subStr, len_subtr) ==  0){
                count++;
            }
        }

        // Thong tin client gui
        printf("%s Counting: %d times\n", subStr, count);
        
    }
    close(client);
    close(server);
    return 0;
}