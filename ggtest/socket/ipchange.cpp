#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <stdio.h>
unsigned int IP;
char b[16];
char* ipip = b;
int main()
{
  inet_pton(AF_INET,"192.168.110.6",&IP);
  unsigned char* b = (unsigned char*)&IP; 
  printf("%d %d %d %d\n",*b,*(b+1),*(b+2),*(b+3));
  

  inet_ntop(AF_INET,&IP,ipip,16);
  printf("%s",ipip);
  return 0;
}
