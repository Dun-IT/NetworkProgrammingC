#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(){
    // Khai bao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    // Ket noi den server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1){
        printf("Khong ket noi duoc den server!\n");
        return 1;
    }

    FILE *f = fopen("streaming_file.txt", "r");
    if (f == NULL)
    {
        perror("fopen() failed");
        exit(EXIT_FAILURE);
    }

    // Read file
    char bufSend[2048];
    //feof(): Hàm này trả về một giá trị khác không khi End-Of-File Indicator mà gắn kết với Stream được thiết lập, nếu không thì hàm này trả về 0.
    while (!feof(f))
    {
        fgets(bufSend, sizeof(bufSend), f);

        if (send(client, bufSend, strlen(bufSend), 0) == -1)
        {
            perror("send() failed");
            exit(EXIT_FAILURE);
        }
    }
    printf("Data sent successfully!\n");
    fclose(f);
    close(client);
    return 0;
}