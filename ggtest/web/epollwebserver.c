#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "wrap.h"
#include "pub.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>

void sendHead(int cfd,int code,char* info,char* filetype,int length)
{
    //发送状态行
    char buf[1024] = "";
    int n = sprintf(buf,"HTTP/1.1 %d %s\r\n",code,info);
    send(cfd,buf,n,0);
    //发送消息头
    n = sprintf(buf,"Content-Type:%s\r\n",filetype);
    send(cfd,buf,n,0);
    if(length > 0)
    {
        n = sprintf(buf,"Content-Length:%d\r\n",length);
        send(cfd,buf,n,0);
    }
    //空行
    send(cfd,"\r\n",2,0);
    
}
void sendFile(int cfd,char* path,struct epoll_event* ev,int epfd,int flag)
{
    int fd = open(path,O_RDONLY);
    if(fd < 0)
    {
        perror("");
        return;
    }
    char buf[1024] = "";
    int len = 0;
    while(1)
    {
        len = read(fd,buf,sizeof(buf));
        if(len <= 0)
        {
            break;
        }
        else
        {
            int n =send(cfd,buf,len,0);
            printf("%d\n",n);
        }
    }
    close(fd);
    if(flag == 1)
    {
        close(cfd);
        epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,ev);
    }
}

void read_client_request(int epfd,struct epoll_event* ev)
{
    //读取请求（先读取一行，把其他行读取扔掉
    char buf[1024] = "";
    char temp[1024] = "";

    int ret = Readline(ev->data.fd,buf,sizeof(buf));
    if(ret <= 0)
    {
        printf("close or err\n");
        epoll_ctl(epfd,EPOLL_CTL_DEL,ev->data.fd,ev);
        close(ev->data.fd);
        return;
    }
    printf("[%s]\n",buf);
    while(Readline(ev->data.fd,temp,sizeof(temp)) > 0);
    //printf("read ok\n");
    //解析请求
    //判断是否为get请求，get请求才处理
    //得到浏览器请求的文件，如果没有请求文件 默认./
    //判断请求文件是都存在，如果存在（普通文件，目录）
    //不存在 发送error.html
    //解析请求行[GET /a.txt HTTP/1.1]
    char method[256] = "";
    char content[256] = "";
    char protocol[256] = "";
    sscanf(buf,"%[^ ] %[^ ] %[^ \r\n]",method,content,protocol);
    printf("[%s] [%s] [%s]\n",method,content,protocol);
    if(strcasecmp(method,"get") == 0)
    {
        char* strflie = content + 1;
        strdecode(strflie,strflie);
        if(*strflie == 0)
        {
            strflie = "./";
        }
        //判断请求文件是否存在
        struct stat s;
        if(stat(strflie,&s) < 0)
        {
            //先发送报头(状态行 消息头 空行)
            //发送文件 error.html
            printf("file not found\n");
            sendHead(ev->data.fd,404,"NOT FOUND",get_mime_type("*.html"),0);
            sendFile(ev->data.fd,"error.html",ev,epfd,1);
        }
        else
        {
            //请求普通文件
            if(S_ISREG(s.st_mode))
            {
                printf("file\n");
                sendHead(ev->data.fd,200,"OK",get_mime_type(strflie),s.st_size);
                sendFile(ev->data.fd,strflie,ev,epfd,1);
            }
            //请求目录
            else if(S_ISDIR(s.st_mode))
            {
                printf("dir\n");
                //发送一个列表，网页
                sendHead(ev->data.fd,200,"OK",get_mime_type("*.html"),0);
                //发送header.html
                sendFile(ev->data.fd,"dir_header.html",ev,epfd,0);
                //列表组包
                struct dirent** mylist = NULL;
                int i,n = scandir(strflie,&mylist,NULL,alphasort);
                int len = 0;
                char buf[1500] = "";
                for(i = 0; i < n; i++)
                {
                    printf("%s\n",mylist[i]->d_name);
                    if(mylist[i]->d_type == DT_DIR)
                    {
                        len = sprintf(buf,"<li><a href=%s/ >%s</a></li>",mylist[i]->d_name,mylist[i]->d_name);
                    }
                    else
                    {
                        len = sprintf(buf,"<li><a href=%s>%s</a></li>",mylist[i]->d_name,mylist[i]->d_name);
                    }
                    send(ev->data.fd,buf,len,0);
                    free(mylist[i]);
                }
                free(mylist);
                //发送tail.html
                sendFile(ev->data.fd,"dir_tail.html",ev,epfd,1);

            }
         }
    }
}
int main(int argc,char* argv[])
{
    signal(SIGPIPE,SIG_IGN);
    //切换工作目录
    //获取当前目录的工作路径
    char pwd_path[256] = "";
    char* path = getenv("PWD");
    strcpy(pwd_path,path);
    strcat(pwd_path,"/web-http");
    chdir(pwd_path);

    short port = atoi(argv[1]);
    int i,lfd = tcp4bind(port,NULL);
    Listen(lfd,128);
    int epfd = epoll_create(1);
    struct epoll_event ev,evarry[1024];
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
    while(1)
    {
        int ret = epoll_wait(epfd,evarry,1024,-1);
        if(ret < 0)
        {
            perror("");
            break;
        }
        else
        {
            for(i = 0; i < ret; i++)
            {
                if(evarry[i].data.fd == lfd && evarry[i].events & EPOLLIN)
                {
                    struct sockaddr_in cliaddr;
                    char ip[16] = "";
                    socklen_t len = sizeof(cliaddr);
                    int cfd = Accept(lfd,(struct sockaddr*)&cliaddr,&len);
                    printf("new client connect ip = %s  port = %d\n",
                        inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,16),ntohs(cliaddr.sin_port));
                    //设置cfd为非阻塞
                    int flags = fcntl(cfd,F_GETFL);
                    flags |= SOCK_NONBLOCK;
                    fcntl(cfd,F_SETFL,flags);

                    ev.data.fd = cfd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
                }
                else if(evarry[i].events & EPOLLIN)
                {
                   read_client_request(epfd,&evarry[i]);
                }
            }
        }
    }
    return 0;
}

