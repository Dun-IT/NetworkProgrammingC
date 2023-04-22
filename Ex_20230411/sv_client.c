#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>

#define MAX_DATA_SIZE 1024
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

    //Nhap thong tin sinh vien
    while (1)
    {
        char mssv[10], ho_ten[50], ngay_sinh[20], diem_tb[10], confirm[5];
    
        printf("Nhap MSSV: ");
        fgets(mssv, sizeof(mssv), stdin);
        mssv[strcspn(mssv, "\n")] = '\0'; 

        printf("Nhap ho ten: ");
        fgets(ho_ten, sizeof(ho_ten), stdin);
        ho_ten[strcspn(ho_ten, "\n")] = '\0'; 

        printf("Nhap ngay sinh (dd/mm/yy): ");
        fgets(ngay_sinh, sizeof(ngay_sinh), stdin);
        ngay_sinh[strcspn(ngay_sinh, "\n")] = '\0'; 

        printf("Nhap diem trung binh: ");
        fgets(diem_tb, sizeof(diem_tb), stdin);
        diem_tb[strcspn(diem_tb, "\n")] = '\0'; 

        //Dong goi thong tin
        int lenBuf = strlen(mssv) + strlen(ho_ten) + strlen(ngay_sinh) + strlen(diem_tb) + 6;
        char *data = (char *)malloc(lenBuf);
        sprintf(data, "%s,%s,%s,%s\n", mssv, ho_ten, ngay_sinh, diem_tb);

        if(send(client, data, lenBuf, 0) < 0) {
            perror("send() failed");
            exit(1);
        }

        free(data);
        printf("Ban co muon tiep tuc nhap? (y/n) ");
        fgets(confirm, sizeof(confirm), stdin);
        confirm[strcspn(confirm, "\n")] = '\0';
        if (confirm[0] == 'n' || confirm[0] == 'N') {
            break;
        }
        printf("send() successfully\n");
    }
    close(client);
}
