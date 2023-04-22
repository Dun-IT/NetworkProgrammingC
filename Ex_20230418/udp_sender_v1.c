#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("usage: %s <IP address> <port> <filename>\n", argv[0]);
        exit(1);
    }
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    char buf[256];
    char file_name[256];
    char result[1024];

    FILE *f = fopen(argv[3], "r");

    fgets(buf, sizeof(buf), f);

    fclose(f);

    fprintf(stdout, "File content: %s\n", buf);

    sprintf(result, "%s | %s", argv[3], buf);

    int ret = sendto(sender, result, strlen(result), 0,
        (struct sockaddr *)&addr, sizeof(addr));
    
    printf("Send successfully!\n");
}