#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    //Kiem tra tham so
    if (argc != 4) {
        printf("Sai tham so.\n");
        return 1;
    }

    //Tao socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai bao cau truc dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // Gan dia chi voi socket
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;        
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    printf("Khoi tao thanh cong!!!\n");

    // Chap nhan ket noi
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int client = accept(listener, 
        (struct sockaddr *)&client_addr, 
        &client_addr_len);

    printf("Accepted socket %d from IP: %s:%d\n", 
        client,
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port)); 

    //TCP_HELLO_SERVER.TXT
    char *file_open = argv[2];
    char bufReadAndSend[256];
    FILE *f = fopen(file_open, "r");
    fgets(bufReadAndSend, 256, f);
    fclose(f);
    send(client, bufReadAndSend, strlen(bufReadAndSend), 0);

    //TCP_CLIENT_RECEIVED.TXT
    char *file_received = argv[3];
    f = fopen(file_received, "w");
    char bufReceived[2048];
    int ret;

    while (1)
    {                           
        ret = recv(client, bufReceived, sizeof(bufReceived), 0);
        if(ret <= 0) {
            break;
        } 

        if(ret < sizeof(bufReceived)) {
            bufReceived[ret] = 0;
        }

        printf("%d bytes received: %s", ret, bufReceived);
        fprintf(f, "%s", bufReceived);
        fclose(f);
    }
    
    //Close
    close(client);
    close(listener);
}