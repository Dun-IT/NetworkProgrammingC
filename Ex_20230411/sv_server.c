#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
int main(int argc, char const *argv[])
{
    if (argc != 3) {
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

    char buf[BUFFER_SIZE];
    while (1)
    {

    // Mo file log
    FILE *log_file = fopen(argv[2], "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return 1;
    }

    // In IP va thoi gian
    time_t time_now = time(NULL);
    char *new_time = ctime(&time_now);
    new_time[strlen(new_time) - 1] = '\0';
    fprintf(log_file, "Accepted socket %d from IP %s at %s: %d\n", 
        client,
        inet_ntoa(client_addr.sin_addr),
        new_time,
        ntohs(client_addr.sin_port)); 

    // Nhan du lieu tu client
    int received_bytes = recv(client, buf, BUFFER_SIZE, 0);
    if (received_bytes < 0) {
        perror("recv() failed");
        continue;
    }

    // In thong tin sinh vien va dia chi IP ra man hinh va file log
    buf[received_bytes] = '\0';
    fprintf(stdout, "Received data from client with IP %s:\n%s", inet_ntoa(client_addr.sin_addr), buf);
    fprintf(log_file, "%s %s %s\n", inet_ntoa(client_addr.sin_addr), new_time, buf);

    if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) {
        break;
    }
    // Dong 
    fclose(log_file);

    }
    close(client);
    close(listener);
    
    return 0;
}
