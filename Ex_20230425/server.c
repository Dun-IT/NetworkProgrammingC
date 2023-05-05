#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

#define MAX_CLIENT 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int client_fds[MAX_CLIENT], max_fd, activity, valread, sd;
    char buffer[BUFFER_SIZE], client_name[BUFFER_SIZE];
    fd_set read_fds;
    struct sockaddr_in address;
    int opt = 1, addrlen = sizeof(address);
    //Kiem tra dau vao
    if (argc != 2) 
    {
        printf("Sai tham so");
        exit(EXIT_FAILURE);
    }

    //Khoi tao socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    //Thiet lap dia chi
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    //Gan dia chi cho socket
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    //Lang nghe ket noi
    if (listen(server, MAX_CLIENT) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    //Mang client
    for (int i = 0; i < MAX_CLIENT; i++) 
    {
        client_fds[i] = 0;
    }

    while (1)
    {
        //Đặt vào tập đọc
        FD_ZERO(&read_fds);
        FD_SET(server, &read_fds);
        max_fd = server;

        for (int i = 0; i < MAX_CLIENT; i++) 
        {
            sd = client_fds[i];

            if (sd > 0) 
            {
                FD_SET(sd, &read_fds);
            }

            if (sd > max_fd)
            {
                max_fd = sd;
            }

            //Cho doi cac socket san sang cho doc
            activity = select(max_fd+1, &read_fds, NULL, NULL, NULL);

            if(activity < 0) 
            {
                perror("Select() error");
                exit(EXIT_FAILURE);
            }

            //Kiem tra xem co ket noi moi khong
            if (FD_ISSET(server, &read_fds))
            {
                int new_socket;

                if ((new_socket = accept(server, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
                {
                    perror("accept() error");
                    exit(EXIT_FAILURE);
                }

                printf("New connection, socket fd is %d, ip is %s, port is %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                // gửi yêu cầu nhập tên của client
                char *message = "Please enter your name: ";
                send(new_socket, message, strlen(message), 0);

                // thêm client socket vào mảng
                for (int i = 0; i < MAX_CLIENT; i++) 
                {
                    if (client_fds[i] == 0) {
                    client_fds[i] = new_socket;
                    break;
                }
            }
        }

        //Kiem tra xem cac client socket co san sang cho doc khong
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            sd = client_fds[i];

            if (FD_ISSET(sd, &read_fds))
            {
                //Doc du lieu tu client socket
                if((valread = read(sd, buffer, BUFFER_SIZE)) == 0)
                {
                    //Ngat ket noi
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnected, socket fd is %d, ip is %s, port is %d\n", sd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_fds[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    if (client_name[i] == '\0') {
                        //Neu chua nhap ten, luu ten client
                        if (sscanf(buffer, "%*s %s", client_name) == 1)
                        {
                            printf("Client %d set name to %s\n", sd, client_name);
                            sprintf(buffer, "%s has joined the chat\n", client_name);
                            for (int j = 0; j < MAX_CLIENT; j++)
                            {
                                if (client_fds[j] > 0 && client_fds[j] != sd)
                                {
                                    send(client_fds[j], buffer, strlen(buffer), 0);
                                }
                            }
                        } else {
                            //Yeu cau nhap ten lai
                            char *message = "Invalid name, please enter your name again: ";
                            send(sd, message, strlen(message), 0);
                        }
                    } else {
                        //Gui tin nhan cho client khac
                        char message[BUFFER_SIZE + 64];
                        memset(message, 0, BUFFER_SIZE + 64);
                        sprintf(message, "%s: %s", client_name, buffer);
                        for (int j = 0; j < MAX_CLIENT; j++) 
                        {
                            
                            if (client_fds[j] > 0 && client_fds[j] != sd)
                            {
                                send(client_fds[j], message, strlen(message), 0);
                            }
                        }
                    }
                }
            }
        }
    }
}
}
