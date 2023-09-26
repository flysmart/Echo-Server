#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>

struct sockInfo{
    int fd;             //通信的文件描述符
    struct sockaddr_in addr;
    pthread_t tid;          //线程号
};

struct sockInfo sockinfos[128];

void *working(void *arg){
    //子线程和客户端通信

    struct sockInfo *pinfo = (struct sockInfo *)arg;
    //获取客户端信息
    char cliIp[16];
    inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, cliIp, sizeof(cliIp));
    unsigned short cliPort = ntohs(pinfo->addr.sin_port);
    printf("client is : %s, port is %d\n", cliIp, cliPort);

    //接收客户端发来的数据
    char recvBuf[1024];
    while(1){
        int len = read(pinfo->fd, &recvBuf, sizeof(recvBuf));
        if(len == -1){
            perror("read");
            exit(-1);
        }else if(len > 0){
            printf("receive client: %s\n", recvBuf);
        }else {
            printf("client closed...\n");
            break;      //要有，否则打印两次
        }

        write(pinfo->fd, recvBuf, strlen(recvBuf)+1); 
    }
    close(pinfo->fd);
    return NULL;
}

int main(){

    //创建socket
    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    if(lfd == -1){
        perror("socket");
        exit(-1);
    }

    //绑定
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(lfd,(struct sockaddr *)&saddr, sizeof(saddr));
    if(ret == -1){
        perror("bind");
        exit(-1);
    }

    //监听
    ret = listen(lfd, 128);
    if(ret == -1){
        perror("listen");
        exit(-1);
    }

    //初始化数据
    int max = sizeof(sockinfos)/sizeof(sockinfos[0]);
    for(int i = 0; i < max; i++){
        bzero(&sockinfos[i], sizeof(sockinfos[i]));
        sockinfos[i].fd = -1;
        sockinfos[i].tid = -1;
    }

    //不断循环等待客户端连接
    while(1){

        //接受连接
        struct sockaddr_in cliaddr;
        int len = sizeof(cliaddr);
        int cfd = accept(lfd, (struct sockaddr*)&cliaddr, &len);
        if(cfd == -1){
            if(errno == EINTR){
                continue;
            }
            perror("accept");
            exit(-1);
        }

        //每一个连接进来，创建一个子线程和客户端通信
        struct sockInfo *pinfo;
        for(int i = 0; i < max; i++){
            //从这个数组中遭到一个可以用的sockInfo元素
            if(sockinfos[i].fd == -1){
                pinfo = &sockinfos[i];
                break;
            }
            if(i == max-1){
                sleep(1);
                i--;
            }
        }
        pinfo->fd = cfd;
        memcpy(&pinfo->addr, &cliaddr, len);

        pthread_create(&pinfo->tid, NULL, working, pinfo);
        pthread_detach(pinfo->tid);

    }
    close(lfd);
    return 0;
}