#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT 7777
#define BUF_SIZE 512
#define MAX_CONNECT 10

int server_socketfd;
int client_socketfd;
int connect_count = 0;



//创建TCP socket连接，返回套接字描述符
void create_socket()
{
    server_socketfd = socket(AF_INET,SOCK_STREAM,0);
    if (server_socketfd <= 0) {
    	perror("create socket error");
    	exit(EXIT_FAILURE);
    }
    printf("server_socketfd: %d\n",server_socketfd);
}

//初始化server
void init_server()
{
    struct sockaddr_in server_addr = {0};
    int ret;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    
    //套接字绑定ip和端口
    ret = bind(server_socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
    	perror("bind error");
    	close(server_socketfd);
    	exit(EXIT_FAILURE);
    }
    
    //开始监听端口
    ret = listen(server_socketfd, 50);
    if (ret < 0) {
    	perror("listen error");
    	close(server_socketfd);
    	exit(EXIT_FAILURE);
    }
    

}

void client_handle()
{
    struct sockaddr_in client_addr = {0};
    char ip_str[20] = {0};
    char recv_buf[BUF_SIZE];
    int addrlen = sizeof(client_addr);
    int ret;
    
    //阻塞，等待client连接
    printf("waiting client connect...\n");
    client_socketfd = accept(server_socketfd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_socketfd <= 0) {
        perror("accept error");
        close(server_socketfd);
        exit(EXIT_FAILURE);
    }
    
    printf("client connect!\n");
    connect_count++;
    printf("now connect_count is %d (maximum number is %d)\n",connect_count, MAX_CONNECT);
    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, sizeof(ip_str));
    printf("client ip is %s\n",ip_str);
    printf("client port is %d\n",client_addr.sin_port);

    //向 client 发送说明
    ret = send(client_socketfd, "ENTER ‘EXIT’ TO EXIT", 25, 0);
    if(0 > ret){
        perror("send error");
        close(client_socketfd);
        return;
    }

    for( ; ; ) {
    memset(recv_buf, 0, BUF_SIZE);
    
    //接收来自client的数据包
    ret = recv(client_socketfd, recv_buf, BUF_SIZE, 0);
    if(0 >= ret) {
        perror("recv error");
        close(client_socketfd);
        break;
    }
    
    //打印数据包
    printf("from client msg: %s\n",recv_buf);
    if (strncmp("EXIT",recv_buf,4) == 0) {
        printf("client exit...\n");
        close(client_socketfd);
        break;
    }
    
    }
    

}

void main(void)
{
    create_socket();
    
    init_server();

    //设置最大连接次数
    while(connect_count<MAX_CONNECT) client_handle();

    printf("connect_count is %d, server will end",connect_count);
    close(server_socketfd);
    exit(EXIT_SUCCESS);
}
