#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[256];

    while (1)
    {
        int ret = recvfrom(receiver, buf, sizeof(buf), 0,
            NULL, NULL);
        if (ret <= 0)
        {
            break;
        }

        buf[ret] = 0;

        FILE *f = fopen("udp_received_file.txt", "a");
        fprintf(f, "%s", buf);
        fclose(f);
        
        printf("File content: %s\n", buf);

        
    }
}