#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>


#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s server_ip server_port client_port\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_port = atoi(argv[3]);

    // Tạo socket UDP
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // Gán cổng chờ cho client
    struct sockaddr_in client_address;
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = htonl(INADDR_ANY);
    client_address.sin_port = htons(client_port);

    if (bind(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) == -1) {
        perror("Binding failed");
        close(client_socket);
        exit(1);
    }

    // Tạo địa chỉ của server
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &(server_address.sin_addr)) <= 0) {
        perror("Invalid address");
        close(client_socket);
        exit(1);
    }

    // Đăng ký socket đọc (stdin) và socket chờ (client_socket) vào danh sách đọc
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(client_socket, &readfds);

    printf("Connected, type anything:\n");

    while (1) {
        fd_set tmp_fds = readfds;

        // Sử dụng hàm select để kiểm tra các socket có dữ liệu để đọc
        if (select(client_socket + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
            perror("Select failed");
            close(client_socket);
            exit(1);
        }

        // Kiểm tra xem socket đọc từ bàn phím có dữ liệu để đọc không
        if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
            char message[MAX_BUFFER_SIZE];
            if (fgets(message, sizeof(message), stdin) == NULL) {
                perror("Reading from stdin failed");
                close(client_socket);
                exit(1);
            }

            // Gửi tin nhắn từ bàn phím tới server
            if (sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
                perror("Sending message failed");
                close(client_socket);
                exit(1);
            }
        }

        // Kiểm tra xem socket chờ có dữ liệu để đọc không
        if (FD_ISSET(client_socket, &tmp_fds)) {
            char message[MAX_BUFFER_SIZE];
            struct sockaddr_in server_recv_address;
            socklen_t server_recv_address_len = sizeof(server_recv_address);

            // Nhận tin nhắn từ server
            ssize_t recv_len = recvfrom(client_socket, message, sizeof(message) - 1, 0, (struct sockaddr *)&server_recv_address, &server_recv_address_len);
            if (recv_len == -1) {
                perror("Receiving message failed");
                close(client_socket);
                exit(1);
            }

            message[recv_len] = '\0';
            printf("Other: %s", message);
        }
    }

    close(client_socket);

    return 0;
}