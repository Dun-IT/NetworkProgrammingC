#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 1024

typedef struct client{
    int sockfd;
    struct sockaddr_in addr;
    char username[MAX_MSG_LEN];
    char password[MAX_MSG_LEN];
} client_t;

int check_user(char *username, char *password){
    // Mở file cơ sở dữ liệu chứa tài khoản được cấp
    FILE *fp = fopen("acc_db.txt", "r");
    if (fp == NULL){
        return -1;
    }

    // Đọc từng dòng
    char line[MAX_MSG_LEN];
    while (fgets(line, MAX_MSG_LEN, fp) != NULL){
        line[strcspn(line, "\n")] = 0;

        // Tách username và password
        char stored_username[MAX_MSG_LEN];
        char stored_password[MAX_MSG_LEN];
        sscanf(line, "%s %s", stored_username, stored_password);

        // So sánh username và password với tài khoản được cấp trong acc_db txt
        if (strcmp(username, stored_username) == 0 && strcmp(password, stored_password) == 0){
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void command_executor(int sockfd, char *command){
    char cmd[MAX_MSG_LEN];
    sprintf(cmd, "%s > out.txt", command);

    // Thực thi lệnh
    int status = system(cmd);
    if (status == 0){
        // Mở file out.txt để đọc kết quả
        FILE *fp = fopen("out.txt", "r");
        if (fp == NULL){
            return;
        }

        // Đọc từng dòng trong file và gửi về client
        char line[MAX_MSG_LEN];
        while (fgets(line, MAX_MSG_LEN, fp) != NULL){
            if (send(sockfd, line, strlen(line), 0) < 0){
                perror("send() failed");
                continue;
            }
        }

        char *msg = "\nEnter your command: ";
        if (send(sockfd, msg, strlen(msg), 0) < 0){
            perror("send() failed");
            return;
        }
        fclose(fp);
    }
    else{
        // Gửi thông báo lỗi về client
        char *msg = "Command not found!\nEnter another command: ";
        if (send(sockfd, msg, strlen(msg), 0) < 0){
            perror("send() failed");
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    // Kiểm tra đầu vào
    if (argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Khởi tạo socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0){
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // Gán địa chỉ server vào socket
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(server, MAX_CLIENTS) < 0){
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    // Khởi tạo mảng client
    client_t clients[MAX_CLIENTS];
    int number_clients = 0;

    while (1){
        // Khởi tạo mảng file descriptor
        fd_set readfds;
        FD_ZERO(&readfds);

        // Thêm socket server vào mảng file descriptor
        FD_SET(server, &readfds);

        // Thêm các socket client vào mảng file descriptor
        if (number_clients == 0){
            printf("Waiting for clients on %s:%s\n",
                   inet_ntoa(server_addr.sin_addr), argv[1]);
        }else{
            for (int i = 0; i < number_clients; i++){
                FD_SET(clients[i].sockfd, &readfds);
            }
        }

        // Chờ các kết nối mới hoặc dữ liệu từ các client
        if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0){
            perror("select() failed");
            continue;
        }

        // Kiểm tra xem server có sẵn sàng nhận kết nối mới hay không
        if (FD_ISSET(server, &readfds)){
            // Chấp nhận kết nối từ client
            struct sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_addr_len = sizeof(client_addr);
            int client_sockfd = accept(server, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_sockfd < 0){
                perror("accept() failed");
                continue;
            }

            // Thêm client vào mảng client
            if (number_clients < MAX_CLIENTS){
                clients[number_clients].sockfd = client_sockfd;
                clients[number_clients].addr = client_addr;
                strcpy(clients[number_clients].username, "");
                strcpy(clients[number_clients].password, "");
                number_clients++;
                printf("Client connectec from %s:%d\n",
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));

                // Gửi thông báo đến client yêu cầu nhập username và password
                char *msg = "Enter your \"username password\": ";
                if (send(client_sockfd, msg, strlen(msg), 0) < 0){
                    perror("send() failed");
                    continue;
                }
            }else{
                char *msg = "Too many clients\n";
                send(client_sockfd, msg, strlen(msg), 0);
                close(client_sockfd);
            }
        }

        // Kiểm tra xem có dữ liệu đến từ client nào không
        for (int i = 0; i < number_clients; i++){
            if (FD_ISSET(clients[i].sockfd, &readfds)){
                // Nhận dữ liệu từ client
                char msg[MAX_MSG_LEN];
                memset(msg, 0, MAX_MSG_LEN);
                int msg_len = recv(clients[i].sockfd, msg, MAX_MSG_LEN, 0);
                if (msg_len < 0){
                    perror("recv() failed");
                    continue;
                }else if (msg_len == 0){
                    printf("Client from %s:%d disconnected\n",
                           inet_ntoa(clients[i].addr.sin_addr),
                           ntohs(clients[i].addr.sin_port));
                    close(clients[i].sockfd);

                    // Xóa client khỏi mảng client
                    clients[i] = clients[number_clients - 1];
                    number_clients--;

                    // Xóa socket client khỏi mảng file descriptor
                    FD_CLR(clients[i].sockfd, &readfds);
                    continue;
                }else{
                    // Xóa ký tự xuống dòng ở cuối chuỗi
                    msg[strcspn(msg, "\n")] = 0;

                    // Kiểm tra xem client đã đăng nhập hay chưa
                    if (strcmp(clients[i].username, "") == 0 && strcmp(clients[i].password, "") == 0){
                        char username[MAX_MSG_LEN];
                        char password[MAX_MSG_LEN];
                        char temp[MAX_MSG_LEN];
                        int ret = sscanf(msg, "%s %s %s", username, password, temp);
                        if (ret == 2){
                            // Xác thực thông tin đăng nhập
                            int status = check_user(username, password);
                            if (status == 1){
                                // Lưu thông tin đăng nhập vào client
                                strcpy(clients[i].username, username);
                                strcpy(clients[i].password, password);

                                // Gửi thông báo đến client
                                char *msg = "Login successful\nEnter your command: ";
                                if (send(clients[i].sockfd, msg, strlen(msg), 0) < 0){
                                    perror("send() failed");
                                    continue;
                                }
                            }else if (status == 0){
                                // Gửi thông báo đến client
                                char *msg = "Login failed! Username or Password is incorrect\nPlease enter again your \"username password\": ";
                                if (send(clients[i].sockfd, msg, strlen(msg), 0) < 0){
                                    perror("send() failed");
                                    continue;
                                }
                            }else{
                                // Gửi thông báo đến client
                                char *msg = "Server error\nDatabase file not found!\nPlease enter again your \"username password\": ";
                                send(clients[i].sockfd, msg, strlen(msg), 0);
                            }
                        }else{
                            // Gửi thông báo đến client
                            char *msg = "Invalid format!\nPlease enter again your \"username password\": ";
                            send(clients[i].sockfd, msg, strlen(msg), 0);
                        }
                    }else{
                        // Xử lý lệnh
                        if (strcmp(msg, "quit") == 0 || strcmp(msg, "exit") == 0){
                            // Gửi thông báo đến client
                            char *msg = "See you again!\n";
                            if (send(clients[i].sockfd, msg, strlen(msg), 0) < 0){
                                perror("send() failed");
                                continue;
                            }

                            // Đóng kết nối
                            close(clients[i].sockfd);

                            // Xóa client khỏi mảng client
                            clients[i] = clients[number_clients - 1];
                            number_clients--;

                            // Xóa socket client khỏi mảng file descriptor
                            FD_CLR(clients[i].sockfd, &readfds);
                        }else{
                            command_executor(clients[i].sockfd, msg);
                        }
                    }
                }
            }
        }
    }

    // Đóng socket
    close(server);

    return 0;
}