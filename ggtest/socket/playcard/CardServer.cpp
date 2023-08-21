#include <sys/types.h>
#include <fcntl.h>
#include "pthreadpool.h"
#include "pthreadpool.cpp"
#include "CardBase.h"
using namespace std;

#define sizemax  4096
clientmsg* AllClinetMsg;
//任务回调函数
void clientSYN(void* msginfo)
{
    clientmsg* msg = (clientmsg*)msginfo;
    int cfd = msg->ev->data.fd;
    char buf[1500] = "";
    while(1)
    {
        //memset(buf,'\0',sizeof(buf));
        int ret = read(cfd,buf,sizeof(buf));
        if(ret < 0)
        {   
            //如果read是非阻塞且缓存区读取干净了，errno会设置为EAGAIN
            if(errno == EAGAIN)
            {
                break;
            }
            perror("");   
            close(cfd);
            epoll_ctl(msg->epfd,EPOLL_CTL_DEL,cfd,msg->ev);
            break;
        }
        else if (ret == 0)
        {
            printf("client close......\n");
            close(cfd);
            epoll_ctl(msg->epfd,EPOLL_CTL_DEL,cfd,msg->ev);
            pthread_mutex_lock(&AllClinetMsg->msg);
            std::vector<int>::iterator it = find(AllClinetMsg->player.begin(),AllClinetMsg->player.end(),cfd);
            if(it != AllClinetMsg->player.end())
            {
                AllClinetMsg->player.erase(it);
                cout<<AllClinetMsg->player.size()<<endl;
            }
            else
            {
                cout<<"not found"<<endl;
            }
            for(int i = 0; i < AllClinetMsg->room.size(); i++)
            {
                if(AllClinetMsg->room[i]->player1 == cfd || AllClinetMsg->room[i]->player2 == cfd || AllClinetMsg->room[i]->player3 == cfd)
                {
                }
            }
            pthread_mutex_unlock(&AllClinetMsg->msg);
            break;
        }
        else
        {
            gamerun(msg,cfd,buf);   
        }
    }

}

int main()
{   
    AllClinetMsg = new clientmsg;
    pthread_mutex_init(&roomLock,NULL);
    pthread_mutex_init(&AllClinetMsg->msg,NULL);
    Thread_pool<clientmsg> pool(5,10);
    int lfd = tcp4bind(8899,NULL);
    Listen(lfd,128);
    int epfd = epoll_create(1);
    struct epoll_event ev,evarry[sizemax];
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
    int j;
    //加入房间任务
    Task<clientmsg> JoRoom(joinRoom,AllClinetMsg);
    pool.addTask(JoRoom);
    Task<clientmsg> DeRoom(destoryRoom,AllClinetMsg);
    pool.addTask(DeRoom);
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
                //记录cfd
                pthread_mutex_lock(&AllClinetMsg->msg);
                AllClinetMsg->player.push_back(cfd);
                pthread_mutex_unlock(&AllClinetMsg->msg);
            }
            else if(evarry[j].events & EPOLLIN)
            {   
                clientmsg* clmsg = new clientmsg(epfd,&evarry[j]);
                pthread_mutex_lock(&AllClinetMsg->msg);
                clmsg->player = AllClinetMsg->player;
                clmsg->room = AllClinetMsg->room;
                clmsg->room_idArr = AllClinetMsg->room_idArr;
                pthread_mutex_unlock(&AllClinetMsg->msg);
                pthread_mutex_init(&clmsg->msg,NULL);
                Task<clientmsg> clientTask(clientSYN,clmsg);
                pool.addTask(clientTask);
            }
        }
    }
    delete AllClinetMsg;
    return 0;
}
