#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "wrap.h"

typedef struct c_info
{
  int cfd;
  struct sockaddr_in clientaddr;
}clientinfo;

void* client_function(void* arg)
{
  char buf[1024];
  while(1)
  {
    clientinfo *info1 = (clientinfo*)arg;
    int n = read(info1->cfd,buf,sizeof(buf));
    if(n < 0)
    {
      perror("");
      break;
    }
    else if(n == 0)
    {
      printf("client close");
      break;
    }
    else{
      printf("%s\n",buf);
      memset(buf,0,sizeof(buf));
      write(info1->cfd,"hello",5);
    }
  }
  return NULL;
}

int main(int argc,char* argv[])
{
  if(argc < 2)
  {
    printf("argc < 2???  \n ./a.out 8000 \n");
    return 0;
  }
  short port = atoi(argv[1]);
  int lfd = tcp4bind(port,NULL);
  Listen(lfd,128);
  struct sockaddr_in cliaddr;
  socklen_t len = sizeof(cliaddr);
  char ip[16] = "";
  clientinfo *info;
  while(1)
  {
    int cfd = Accept(lfd,(struct sockaddr*)&cliaddr,&len);
    printf("new client connect ip = %s port = %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,16),ntohs(cliaddr.sin_port));
    pthread_t tid;
    info = (clientinfo*)malloc(sizeof(clientinfo));
    info->cfd = cfd;
    info->clientaddr = cliaddr;
    pthread_create(&tid,NULL,client_function,info);
    pthread_detach(tid);
  }

  return 0;
}
