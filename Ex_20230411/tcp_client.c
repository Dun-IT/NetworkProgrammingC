#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[])
{
    //Kiem tra tham so
    if (argc != 3) {
        printf("Sai tham so.\n");
        return 1;
    }

    //Khai bao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //Khai bao dia chi của server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    //Ket noi den server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));

    if (res == -1) {
        printf("Khong ket noi duoc den server");
        return 1;
    }
    
    //Gui tin nhan den server
    char buf[256];
    while (1)
    {
        printf("Nhap xau: ");
        fgets(buf, sizeof(buf), stdin);
        int ret = sendto(client, buf, strlen(buf), 0,
            (struct sockaddr *)&addr, sizeof(addr));
        printf("%d bytes sent\n", ret);
        
        if (strncmp(buf, "exit", 4) == 0)
            break;
    }
    close(client);
    return 0;
}