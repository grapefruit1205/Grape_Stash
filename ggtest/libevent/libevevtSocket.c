#include <stdio.h>
#include <event.h>
#include "wrap.h"

#define port 8888
#define arrlen 128

struct cfdroot
{
    int fd;
    struct event* evf;
};

struct cfdroot fdArry[arrlen];

void cfdcb(int cfd,short event,void* arg)
{
    char buf[1500] = "";
    int i;
    int n = read(cfd,buf,sizeof(buf));
    struct event* eva = NULL;
    if(n <= 0)
    {
        for(i=0;i < arrlen;i++)
        {
            if(fdArry[i].fd == cfd)
            {
                eva = fdArry[i].evf;
                fdArry[i].evf = NULL;
                fdArry[i].fd = -1;
                break;
            }
        }
        close(cfd);
        event_del(eva);
    }
    else
    {
        printf("%s\n",buf);
    }
}

void lfdcb(int lfd,short event,void* arg)
{
    int i;
    int cfd = Accept(lfd,NULL,NULL);
    struct event_base* baseroot = (struct event_base*)arg;
    struct event* cfdev = event_new(baseroot,cfd,EV_READ | EV_PERSIST,cfdcb,NULL);
    for(i=0;i < arrlen;i++)
    {
        if(fdArry[i].fd < 0)
        {
            fdArry[i].fd = cfd;
            fdArry[i].evf = cfdev;
            break;
        }
    }
    event_add(cfdev,NULL);
}
int main()
{   int i;
    for(i = 0;i < 128;i++)
    {
        fdArry[i].fd = -1;
        fdArry[i].evf = NULL;
    }
    int lfd = tcp4bind(port,NULL);
    Listen(lfd,128);
    struct event_base* baseroot = event_base_new();
    struct event* ev = event_new(baseroot,lfd,EV_READ|EV_PERSIST,lfdcb,baseroot);
    event_add(ev,NULL);
    event_base_dispatch(baseroot);
    close(lfd);
    event_base_free(baseroot);
    return 0;
}
