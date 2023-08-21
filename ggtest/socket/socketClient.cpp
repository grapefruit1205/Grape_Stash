#include <stdio.h>
#include <stdlib.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
const char *ip = "192.168.100.6";
int main()
{
  int fd = socket(AF_INET,SOCK_STREAM,0);
    printf("%d\n",1);
  struct sockaddr_in addr;
  inet_pton(AF_INET,ip,&addr.sin_addr.s_addr);
  addr.sin_port = htons(8080);
  addr.sin_family = AF_INET; 
  printf("%d\n",2);
  int ret = connect(fd,(struct sockaddr*)&addr,sizeof(addr));
  printf("%d\n",ret);

  char buff[1024] = "a88888";
  while(1)
  {
    //int n = read(STDIN_FILENO,buff,sizeof(buff));
    write(fd,buff,6);
    printf("%s\n",buff);
    sleep(1);
    fflush(stdout);
  }
  close(fd);

  return 0;
}
