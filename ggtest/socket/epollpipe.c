#include "wrap.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>

int main()
{
  int pipefd[2];
  pipe(pipefd);
  pid_t pid = fork();
  if(pid == 0)
  {
    close(pipefd[0]);
    while(1)
    {
      sleep(3);
      write(pipefd[1],"hello",5);
    }
  }
  else
  {
   close(pipefd[1]);
   struct epoll_event evarry[20];
   int i,j;
   int epfd = epoll_create(1);
   struct epoll_event ev;
   ev.events = EPOLLIN;
   ev.data.fd = pipefd[0];
   epoll_ctl(epfd,EPOLL_CTL_ADD,pipefd[0],&ev);
   while(1)
   {
     char buf[1500];
     memset(buf,0,sizeof(buf));
     for(i = 0;i<20;i++)
     {
       evarry[i].data.fd = -1;
     }
     int n = epoll_wait(epfd,evarry,20,-1);
     for(j = 0;j<n;j++)
     {
       if(evarry[j].data.fd != -1)
       {
          int ret = Read(evarry[j].data.fd,buf,sizeof(buf));
          if(ret <= 0 )
          {
            epoll_ctl(epfd,EPOLL_CTL_DEL,evarry[j].data.fd,&ev);
            break;
          }
          printf("我是你爹+%s\n",buf);
       }
     }
   }
  }
  return 0;
}
