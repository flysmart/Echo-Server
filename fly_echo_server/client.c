//TCP通信的客户端

#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

int main(){

    //创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        perror("socket");
        exit(-1);
    }

    //连接服务器端
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.88.129", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(9999);
    int ret = connect(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(ret ==-1){
        perror("connect");
        exit(-1);
    }

    //通信
    char recvBuf[1024];
    int i = 0;
    while(1){
        sprintf(recvBuf, "data : %d\n", i++);
        //sleep(1);  放这里会产生问题
        //给服务器端发送数据
        write(fd, recvBuf, strlen(recvBuf)+1);  // sizeof(recvBuf)+1是为了带上结束符
        
        int len = read(fd, recvBuf, sizeof(recvBuf)); 
        if(len == -1){
            perror("read");
            exit(-1);
        }else if(len > 0){
            printf("receive server: %s\n", recvBuf);
        }else {
            printf("server closed...\n");
            break;
        }

        sleep(1);
    }

    //关闭连接
    close(fd);
    return 0;

}