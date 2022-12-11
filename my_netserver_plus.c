#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define SERVER_PORT 7777
#define BUF_SIZE 512
#define MAX_CONNECT 50
#define MAX_THREAD 10

unsigned int online_number=0;


struct client_struct
{
    int client_socketfd;
    char client_ip[20];
    char client_name[20];
};



//创建TCP socket连接，返回套接字描述符
int create_socket()
{
    int server_socketfd;
    server_socketfd = socket(AF_INET,SOCK_STREAM,0);
    if (server_socketfd <= 0) {
    	perror("create socket error");
    	exit(EXIT_FAILURE);
    }
    return server_socketfd;
}

//初始化server
void init_server(int server_socketfd)
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

void client_handle(void *Myclient)
{
    struct client_struct *myclient = (struct client_struct *)Myclient;
    char recv_buf[BUF_SIZE];
    int ret;
    time_t curr_time;
    //向 client 发送说明
    ret = send(myclient->client_socketfd, "Welcome! Please input your name:\n(ENTER ‘EXIT’ TO EXIT)\n", 62, 0);
    if(0 > ret){
        perror("send error");
        close(myclient->client_socketfd);
        return;
    }
    puts("wait for user input name ...");
    recv(myclient->client_socketfd, recv_buf, BUF_SIZE, 0);
    strncpy(myclient->client_name,recv_buf,20);
    int len = strlen(myclient->client_name);
    myclient->client_name[len-1]='\0';
    printf("%s join the chatroom\n",myclient->client_name);
    for( ; ; ) {
    memset(recv_buf, 0, BUF_SIZE);
    
    //接收来自client的数据包
    ret = recv(myclient->client_socketfd, recv_buf, BUF_SIZE, 0);
    if(0 >= ret) {
        perror("recv error");
        close(myclient->client_socketfd);
        break;
    }
    
    //打印数据包
    len = strlen(recv_buf);
    recv_buf[len-1] = '\0';
    curr_time = time(NULL); // 获取当前时间
    printf("********************\n%s",ctime(&curr_time));
    if (strncmp("EXIT",recv_buf,4) == 0) {
        printf("%s (ip: %s) exit...\n",myclient->client_name, myclient->client_ip);
        close(myclient->client_socketfd);
        break;
    }
    printf("%s (ip: %s): %s\n",myclient->client_name, myclient->client_ip, recv_buf);

    }
    online_number--;
    printf("Currently, there are %d people online in the chatroom\n",online_number);
    pthread_exit(NULL);

}

void distribute_client_thread(int server_socketfd)
{
    struct sockaddr_in client_addr = {0};
    char ip_str[20] = {0};
    int addrlen = sizeof(client_addr);
    pthread_t thread=0;
    
    struct client_struct *myclient_struct = malloc(sizeof(struct client_struct));
    //阻塞，等待client连接
    printf("Currently, there are %d people online in the chatroom\n",online_number);
    printf("****************************************\n");
    myclient_struct->client_socketfd = accept(server_socketfd, (struct sockaddr *)&client_addr, &addrlen);
    if (myclient_struct->client_socketfd <= 0) {
        perror("accept error");
        close(server_socketfd);
        exit(EXIT_FAILURE);
    }
    printf("****************************************\n");
    printf("There is a user connection!\n");
    online_number++;
    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, sizeof(ip_str));
    printf("The user ip is %s, port is %d\n",ip_str, client_addr.sin_port);
    strncpy(myclient_struct->client_ip, ip_str, 20);

    pthread_create(&thread, NULL, (void *)client_handle, (void *)myclient_struct);
}



void main(void)
{
    int server_socketfd;
    int connect_count = 0;

    server_socketfd = create_socket();
    
    init_server(server_socketfd);

    puts("chatroom setup success!");
    //设置最大连接次数
    for( ; ; ) {
    distribute_client_thread(server_socketfd);
    }
    close(server_socketfd);
    exit(EXIT_SUCCESS);
}

