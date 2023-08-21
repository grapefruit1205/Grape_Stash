#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "wrap.h"
#include <signal.h>
#include <sys/wait.h>

void free_process(int sig)
{
  while(1)
  {
    pid_t pid = waitpid(-1,NULL,WNOHANG);
    if(pid <= 0)
    {
      break;
    }
    else
    {
      printf("childen quit pid = %d\n",pid);
    }
  }

}

int main(int argc,char*argv[])
{
  //创建阻塞集，防止父进程在注册新号之前紫禁城挂了而熬制父进程忽略信号
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set,SIGCHLD);
  sigprocmask(SIG_BLOCK,&set,NULL);

  char ip[16] = "";
  int lfd = tcp4bind(8081,NULL);
  Listen(lfd,128);
  struct sockaddr_in cliaddr;
  socklen_t len = sizeof(cliaddr);
  while(1)
  {
    int cfd = Accept(lfd,(struct sockaddr*)&cliaddr,&len);
    printf("new client connect ip = %s  port =%d\n",inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,16),ntohs(cliaddr.sin_port));

    pid_t pid = fork();
    if(pid == 0)
    {
      close(lfd);
      while(1)
      {
        char buf[1024] = "";
        int n = read(cfd,buf,sizeof(buf));
        if(n < 0)
        {
          perror("");
          close(cfd);
          exit(0);
        }
        else if (n == 0)
        {
          printf("client close");
          close(cfd);
          exit(0);
        }
        else
        {
          printf("%s\n",buf);
          write(cfd,"收到",n);
          
        }
      }
    }
    else
    {
      close(cfd);
      //回收、注册信号回调
      struct sigaction act;
      act.sa_flags = 0;
      act.sa_handler = free_process;
      sigemptyset(&act.sa_mask);
      sigaction(SIGCHLD,&act,NULL);
      sigprocmask(SIG_UNBLOCK,&set,NULL);      
     }
  }
  return 0;
}
