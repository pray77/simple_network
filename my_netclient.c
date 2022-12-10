#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define SERVER_PORT 7777 //服务器的端口号
#define SERVER_IP "127.0.0.1" //服务器的 IP 地址
#define BUF_SIZE 512

int server_sokcetfd;
char send_buf[BUF_SIZE];
char recv_buf[BUF_SIZE];
int ret;

void connect_to_server()
{
    struct sockaddr_in server_addr = {0};

    //打开套接字，得到套接字描述符
    server_sokcetfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > server_sokcetfd) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    // 调用 connect 连接远端服务器 
    server_addr.sin_family = AF_INET;  //协议
    server_addr.sin_port = htons(SERVER_PORT); //端口号
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);//IP 地址
    ret = connect(server_sokcetfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (0 > ret) {
        perror("connect error");
        close(server_sokcetfd);
        exit(EXIT_FAILURE);
    }
    printf("connect server success ...\n\n");    
}

void recv_from_server()
{
    //接收来自server的数据包
    ret = recv(server_sokcetfd, recv_buf, BUF_SIZE, 0);
    if(0 >= ret) {
        perror("recv error");
        close(server_sokcetfd);
    }
    printf("from server msg: %s\n",recv_buf);   
}

void send_to_server()
{
    /* 向服务器发送数据 */
    for ( ; ; ) {
    // 清理缓冲区
    memset(send_buf, 0x0, BUF_SIZE);
    // 接收用户输入的字符串数据
    printf("Please enter a string: ");
    fgets(send_buf, BUF_SIZE, stdin);
    // 将用户输入的数据发送给服务器
    ret = send(server_sokcetfd, send_buf, BUF_SIZE, 0);
    if(0 > ret){
        perror("send error");
        break;
    }
    //输入了"EXIT"，退出循环
    if(0 == strncmp(send_buf, "EXIT", 4))
        break;
    }

}

int main(void)
{
    connect_to_server(); //连接服务器
    
    recv_from_server();  //接收来自服务器的数据
    
    send_to_server();    //发送数据给服务器
    
    close(server_sokcetfd);
    exit(EXIT_SUCCESS);
}
