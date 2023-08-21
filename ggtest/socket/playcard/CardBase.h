#pragma once 
#include "wrap.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <string.h>
using namespace std;
class  Playcard;
class GameInit;

static pthread_mutex_t roomLock;
//任务传递参数结构
class clientmsg
{
    public:
    clientmsg(int epfd,struct epoll_event* ev)
    {
        this->epfd = epfd;
        this->ev = ev;
    }
    clientmsg(){};
    int epfd;
    struct epoll_event* ev;
    //在线等待玩家数量
    vector<int> player;
    //房间
    vector<GameInit*> room;
    //房间号id
    vector<int> room_idArr;
    pthread_mutex_t msg;
};


class card{
    public:
        card(int color,int value);
        ~card();
    private:
        int color;
        int value;
};

class GameInit{
    public:
        GameInit();
        ~GameInit();
        void InitCards();

        Playcard* nowgame;
        int gameover = 0;
        int room_id;
        //客户端文件描述符
        int player1;
        int player2;
        int player3;

        int readyplayer = 0;

        
        vector<int> Cards;
        vector<int> landlorCard;
        vector<int> farmer1Card;
        vector<int> farmer2Card;

};

class Playcard{
    public:
        //处理客户端消息
        void playerMsg_handle();
        //出牌
        void pushCard();
        //结算
        void settle_accout();

        int landlor_remaining_cardNum;
        int famer1_remaining_cardNum;
        int famer2_remaining_cardNum;

        vector<int> landlorNowCard;
        vector<int> farmer1NowCard;
        vector<int> farmer2NowCard;
        
        int callnum = 0;

        int whocall;

        int play1call = 1;
        int play2call = 1;
        int play3call = 1;

        int play1catch = 1;
        int play2catch = 1;
        int play3catch = 1;
};

int roomid_create(void* ptr);
void joinRoom(void* ptr);
void destoryRoom(void* ptr);
GameInit* whoRoomMsg(void* ptr,int cfd);
void gamerun(void* ptr,int cfd,char* buf);
//抢地主
void catchLandlor(GameInit* temp,string msg);
//地主判断
int whoLandlord(GameInit* temp);
