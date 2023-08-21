#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include "wrap.h"
#include "pthreadpool.h"
#include "pthreadpool.cpp"

#define port 8891
#define sizemax  1024

struct clientmsg
{
    clientmsg(int epfd,int n,struct epoll_event* evarry)
    {
        this->epfd = epfd;
        this->n = n;
        this->evarry = evarry;
    }
    int epfd;
    int n;
    struct epoll_event* evarry;
};

void clientSYN(void* msginfo)
{
    clientmsg* msg = (clientmsg*)msginfo;
    int j = msg->n;
    char buf[1500] = "";
    while(1)
    {
        int ret = Read(msg->evarry[j].data.fd,buf,sizeof(buf));
        if(ret < 0)
        {   
            //如果read是非阻塞且缓存区读取干净了，errno会设置为EAGAIN
            if(errno == EAGAIN)
            {
                break;
            }
            perror("");   
            close(msg->evarry[j].data.fd);
            epoll_ctl(msg->epfd,EPOLL_CTL_DEL,msg->evarry[j].data.fd,&msg->evarry[j]);
            break;
        }
        else if (ret == 0)
        {
            printf("client close......\n");
            close(msg->evarry[j].data.fd);
            epoll_ctl(msg->epfd,EPOLL_CTL_DEL,msg->evarry[j].data.fd,&msg->evarry[j]);
            break;
        }
        else
        {
            printf("22222thread %lu is working\n",pthread_self());
            printf("%s\n",buf);
        }

    }
}

int main()
{   
    Thread_pool<clientmsg> pool(3,10);
    int lfd = tcp4bind(port,NULL);
    Listen(lfd,128);
    int epfd = epoll_create(1);
    struct epoll_event ev,evarry[sizemax];
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
    int j;
    while(1)
    {
        int n = epoll_wait(epfd,evarry,sizemax,-1);
        if(n < 0)
        {
            perror("");
            break;
        }
        else if (n == 0)
        {
            continue;
        }
        for(j = 0; j < n;j++)
        {
            if(evarry[j].data.fd == lfd && evarry[j].events & EPOLLIN)
            {
                struct sockaddr_in cliaddr;
                socklen_t len = sizeof(cliaddr);
                int cfd = Accept(lfd,(struct sockaddr*)&cliaddr,&len);
                char ip[16] = "";
                printf("new client connect ip = %s  port = %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,16),ntohs(cliaddr.sin_port));
                ev.data.fd = cfd;
                ev.events = EPOLLIN | EPOLLET;
                //设置cfd为非阻塞
                int flags = fcntl(cfd,F_GETFL);
                flags |= SOCK_NONBLOCK;
                fcntl(cfd,F_SETFL,flags);

                epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
            }
            else if(evarry[j].events & EPOLLIN)
            {
                clientmsg* clmsg = new clientmsg(epfd,j,evarry);
                Task<clientmsg> clientTask(clientSYN,clmsg);
                pool.addTask(clientTask);
            }
        }
    }
    return 0;
}
