#include <sys/select.h>
#include<stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "wrap.h"
#include <sys/time.h>
#include <pthread.h>
#define port 8882

int main(int argc,char* argv[])
{
  //创建套接字
  int lfd = tcp4bind(port,NULL);
      printf("%d\n",lfd);
  //监听
 Listen(lfd,128);
 int maxfd = lfd;//最大文件描述符
 fd_set oldset;
 fd_set rset;
 FD_ZERO(&oldset);
 FD_ZERO(&rset);
 //将lfd加入到oldset集合
 FD_SET(lfd,&oldset);
 while(1)
 {
   rset = oldset;
   int n = select(maxfd+1,&rset,NULL,NULL,NULL);
   if(n < 0)
   {
     perror("");
     break;
   }
   else if (n == 0 )
   {
      continue;
   }
   else{
     //lfd
     if(FD_ISSET(lfd,&rset)){
       struct sockaddr_in cliaddr;
       socklen_t len = sizeof(cliaddr);
       char IP[16];
       //提取新连接
       int cfd = Accept(lfd,(struct sockaddr*)&cliaddr,&len);
       printf("new client connect ip = %s  port = %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,IP,16),ntohs(cliaddr.sin_port));
      //将cfd添加至oldset
      FD_SET(cfd,&oldset);
      pthread_t p = pthread_self();
      printf("文件描述符 = %d ，线程号 = %d\n",cfd,(int)p);
      if(cfd > maxfd) 
        maxfd = cfd;
      if(--n == 0)
        continue;
     }
     int i = 0;
     for(i = lfd + 1;i <= maxfd;i++)
     {
       if(FD_ISSET(i,&rset))
       {
          char buf[1500] = "";
          int ret = Read(i,buf,sizeof(buf));
          if(ret < 0)
          {
            perror("");
            //close(i);
            //FD_CLR(i,&oldset);
            if(i != maxfd)
            {
              dup2(maxfd,i);
              close(maxfd);
              FD_CLR(maxfd,&oldset);
              maxfd--;
            }
            continue;
          }
          else if (ret == 0)
          {
            //close(i);
            //FD_CLR(i,&oldset);
            if(i != maxfd)
            {
              dup2(maxfd,i);
              close(maxfd);
              FD_CLR(maxfd,&oldset);
              maxfd--;
            }
            continue;
          }
          else
          {
            printf("%s\n",buf);
            write(i,"hello",5);
          }
       }
     }
   }
 }
  
  
  //读写
}

