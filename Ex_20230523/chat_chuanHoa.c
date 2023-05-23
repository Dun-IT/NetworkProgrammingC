#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h> 
#include <errno.h>  
;
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSockets[MAX_CLIENTS];
    struct sockaddr_in serverAddress, clientAddress;
    fd_set readfds;
    int maxSd, activity, i, valread, sd;
    int numClients = 0;
    char buffer[BUFFER_SIZE];

    // Khởi tạo mảng 
    for (i = 0; i < MAX_CLIENTS; i++) {
        clientSockets[i] = 0;
    }

    // Tạo socket server
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập thông tin địa chỉ server
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8888);

    // Bind socket với địa chỉ và cổng
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Đang chờ kết nối từ client...\n");

    socklen_t addrlen;
    while (1) {
        // Xóa tập hợp readfds và thêm socket server vào tập hợp
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        maxSd = serverSocket;

        // Thêm các client sockets vào tập hợp
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clientSockets[i];

            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            if (sd > maxSd) {
                maxSd = sd;
            }
        }

        // Sử dụng hàm select để chờ sự kiện trên các socket
        activity = select(maxSd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("Hàm select thất bại\n");
        }

        // Xử lý kết nối mới từ client
        if (FD_ISSET(serverSocket, &readfds)) {
            int newSocket;
            addrlen = sizeof(clientAddress);
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&addrlen)) < 0) {
                perror("Chấp nhận kết nối thất bại");
                exit(EXIT_FAILURE);
            }

            // Gửi xâu chào kèm số lượng client đang kết nối
            char welcomeMessage[BUFFER_SIZE];
            sprintf(welcomeMessage, "Xin chào. Hiện có %d clients đang kết nối.\n", numClients);
            send(newSocket, welcomeMessage, strlen(welcomeMessage), 0);

            // Thêm socket mới vào mảng clientSockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    numClients++;
                    break;
                }
            }
        }

        // Xử lý dữ liệu từ các client đang kết nối
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clientSockets[i];

            if (FD_ISSET(sd, &readfds)) {
                // Đọc dữ liệu từ client
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Client đã đóng kết nối
                    getpeername(sd, (struct sockaddr *)&clientAddress, (socklen_t *)&addrlen);
                    printf("Client đã đóng kết nối, IP: %s, Port: %d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

                    // Đóng socket và xóa khỏi mảng clientSockets
                    close(sd);
                    clientSockets[i] = 0;
                    numClients--;
                } else {
                    // Chuẩn hóa xâu ký tự
                    int j;
                    int word_start = 1;
                    for (j = 0; j < valread; j++) {
                        if (isspace(buffer[j])) {
                            word_start = 1;
                        } else {
                            if (word_start) {
                                buffer[j] = toupper(buffer[j]);
                                word_start = 0;
                            } else {
                                buffer[j] = tolower(buffer[j]);
                            }
                        }
                    }
                    buffer[j] = '\0';

                    // Gửi kết quả chuẩn hóa xâu cho client
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    return 0;
}
