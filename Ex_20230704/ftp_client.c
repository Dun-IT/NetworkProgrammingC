#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int create_data_connection(char* ip, int port) {
    int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = inet_addr(ip);
    data_addr.sin_port = htons(port);

    if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr))) {
        perror("connect() failed");
        return -1;
    }

    return data_socket;
}

int main() {
    int ctrl_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in ctrl_addr;
    ctrl_addr.sin_family = AF_INET;
    ctrl_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ctrl_addr.sin_port = htons(21); 

    if (connect(ctrl_socket, (struct sockaddr *)&ctrl_addr, sizeof(ctrl_addr))) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];
    int len;

    // Nhận xâu chào từ server
    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("%s", buf);

    char username[64], password[64];

    printf("Nhap username: ");
    scanf("%s", username);
    printf("Nhap password: ");
    scanf("%s", password);

    // Gửi lệnh USER
    sprintf(buf, "USER %s\r\n", username);
    send(ctrl_socket, buf, strlen(buf), 0);

    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("%s", buf);

    // Gửi lệnh PASS
    sprintf(buf, "PASS %s\r\n", password);
    send(ctrl_socket, buf, strlen(buf), 0);

    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("%s", buf);

    if (strncmp(buf, "230", 3) != 0) {
        printf("Dang nhap that bai.\n");
        close(ctrl_socket);
        return 1;
    }

    // Mở kết nối dữ liệu
    // Gửi lệnh PASV hoặc EPSV
    send(ctrl_socket, "EPSV\r\n", 6, 0);
    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("%s", buf);

    int ip1, ip2, ip3, ip4, p1, p2;
    char *pos1, *pos2, *temp;
    
    if (strncmp(buf, "229", 3) == 0) {
        // Xử lý kết quả lệnh EPSV
        pos1 = strchr(buf, '(') + 1;
        pos2 = strchr(pos1, ')');
        temp = pos1;
        while (*temp != '\0') {
            if (*temp == '|')
                *temp = '.';
            temp++;
        }
        sscanf(pos1, "%d.%d.%d.%d|%d|", &ip1, &ip2, &ip3, &ip4, &p1);
    } else {
        // Xử lý kết quả lệnh PASV
        pos1 = strchr(buf, '(') + 1;
        pos2 = strchr(pos1, ')');
        sscanf(pos1, "%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &p1, &p2);
    }

    char data_ip[20];
    sprintf(data_ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    int data_port = p1 * 256 + p2;

    // Mở kết nối dữ liệu
    int data_socket = create_data_connection(data_ip, data_port);
    if (data_socket == -1) {
        return 1;
    }

    // Gửi lệnh STOR
    send(ctrl_socket, "STOR test.txt\r\n", 15, 0);
    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("%s", buf);

    // Đọc dữ liệu từ tệp tin
    FILE *file = fopen("test.txt", "rb");
    if (file == NULL) {
        perror("Cannot open file");
        close(data_socket);
        close(ctrl_socket);
        return 1;
    }

    while ((len = fread(buf, 1, sizeof(buf), file)) > 0) {
        send(data_socket, buf, len, 0);
    }

    fclose(file);
    close(data_socket);

    // Nhận phản hồi còn lại của lệnh STOR
    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("%s", buf);

    // Gửi lệnh QUIT
    send(ctrl_socket, "QUIT\r\n", 6, 0);
    len = recv(ctrl_socket, buf, sizeof(buf), 0);
    buf[len] = 0;
    printf("%s", buf);

    // Đóng socket điều khiển
    close(ctrl_socket);

    return 0;
}
