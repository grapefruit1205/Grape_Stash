#include "CardBase.h"
#include <string>
card::card(int color, int value)
{
    this->color = color;
    this->value = value;
}
card::~card()
{

}


GameInit::~GameInit()
{

}
GameInit::GameInit()
{
    //创建牌堆
    int color = 0;
    int value = 0;
    int sum = 0;
    int flag = 0;
    srand((unsigned int)time(NULL));
    while(Cards.size() < 54)
    {
        color = rand()%4 + 1;
        value = rand()%16 + 1;
        sum = color*100 + value;
        if(Cards.size() == 0) 
        {
            Cards.push_back(sum);
        }
        else
        {
            for(int j = 0; j < Cards.size(); j++)
            {
                if(value != 16)
                {
                    if(sum == Cards[j])
                    {
                        flag = 1;
                        break;
                    }
                    else
                    {

                    }
                }
                else
                {
                    if(Cards[j] == 116 || Cards[j] == 216)
                    {
                        if(sum == 116 || sum == 216)
                        {
                            flag = 1;
                            break;
                        }   
                    }
                    if(Cards[j] == 316 || Cards[j] == 416)
                    {
                        if(sum == 316 || sum == 416)
                        {
                            flag = 1;
                            break;
                        }   
                    }
                }
            }
            if(flag == 0)
            {
                Cards.push_back(sum);
            }
            else
            {
                flag = 0;
            }
        }
    }
    //分牌
    int n = 53;
    while(Cards.size() > 3)
    {
        landlorCard.push_back(Cards[n]);
        Cards.pop_back();
        n--;
        farmer1Card.push_back(Cards[n]);
        Cards.pop_back();
        n--;
        farmer2Card.push_back(Cards[n]);
        Cards.pop_back();
        n--;
    }
    if(Cards.size() == 3)
    {
        landlorCard.push_back(Cards[0]);
        landlorCard.push_back(Cards[1]);
        landlorCard.push_back(Cards[2]);
    }
}
//处理客户端发过来的消息
void Playcard::playerMsg_handle()
{

}
void Playcard::pushCard()
{

}
void Playcard::settle_accout()
{

}
//创建房间号
int roomid_create(void* pt)
{
    clientmsg* ptr = static_cast<clientmsg*>(pt);
    int id;
agin:
    id = rand()%300 + 1;
    if(ptr->room_idArr.size() == 0)ptr->room_idArr.push_back(id);
    if(find(ptr->room_idArr.begin(),ptr->room_idArr.end(),id) == ptr->room_idArr.end())
    {
        ptr->room_idArr.push_back(id);
    }
    else
    {
        goto agin;
    }
    return id;
}
//建立房间
void joinRoom(void* pt)
{
    clientmsg* ptr = static_cast<clientmsg*>(pt);
    while(1)
    {
        pthread_mutex_lock(&ptr->msg);
        if(ptr->player.size() >=  3)
        {
            //初始化游戏
            GameInit* tmp = new GameInit;
            tmp->nowgame = new Playcard;
            ptr->room.push_back(tmp);
            //获取客户端cfd
            tmp->player1 = ptr->player[0];
            ptr->player.erase(ptr->player.begin());
            tmp->player2 = ptr->player[0];
            ptr->player.erase(ptr->player.begin());
            tmp->player3 = ptr->player[0];
            ptr->player.erase(ptr->player.begin());
            //分配房间号
            tmp->room_id = roomid_create(ptr);
            //定义玩家序号
            Write(tmp->player1,"id:1",4);
            Write(tmp->player2,"id:2",4);
            Write(tmp->player3,"id:3",4);
            //Task<clientmsg> roomtask();
        }
        pthread_mutex_unlock(&ptr->msg);
    }
}
//每两秒进行回收房间
void destoryRoom(void* pt)
{
    int roomid;
    clientmsg* ptr = static_cast<clientmsg*>(pt);
    while(1)
    {
        pthread_mutex_lock(&ptr->msg);
        for(int i = 0; i < ptr->room.size(); i++)
        {
            if(ptr->room[i]->gameover == 1)
            {   
                roomid = ptr->room[i]->room_id;
                for(int j = 0; j < ptr->room_idArr.size(); j++ )
                {
                    if(roomid == ptr->room_idArr[j])
                    {
                        ptr->room_idArr.erase(ptr->room_idArr.begin() + j);
                    }
                }
                delete ptr->room[i];
                ptr->room.erase(ptr->room.begin() + i);
            }
        }
        pthread_mutex_unlock(&ptr->msg);
        sleep(2);
    }
}
//判断是哪个房间的消息
GameInit*  whoRoomMsg(void* pt,int cfd)
{
    clientmsg* ptr = static_cast<clientmsg*>(pt);
    pthread_mutex_lock(&ptr->msg);
    for(int i = 0; i < ptr->room.size(); i++)
    {
        if(cfd == ptr->room[i]->player1 || cfd == ptr->room[i]->player2 || cfd == ptr->room[i]->player3)
        {
            return ptr->room[i]; 
        }
    }
    pthread_mutex_unlock(&ptr->msg);
    return nullptr;
}

void gamerun(void* pt,int cfd,char* buf)
{
    char sendBuf[512] = "";
    string receive = buf;
    cout<<receive<<endl;
    GameInit* temp = whoRoomMsg(pt,cfd);
    if(temp == nullptr)
    {
        perror("");
        return;
    }
    if(strstr(buf,"idSuccess") != NULL)
    {
        temp->readyplayer++;
        cout<<temp->readyplayer<<endl;
    }
    if(strstr(buf,"idError") != NULL)
    {
        if(cfd == temp->player1)Write(cfd,"id:1",4);
        if(cfd == temp->player2)Write(cfd,"id:2",4);
        if(cfd == temp->player3)Write(cfd,"id:3",4);
    }
    
    //pthread_mutex_lock(&roomLock);
    if(temp->readyplayer == 3)
    {
        temp->readyplayer = 0;
        cout<<"start"<<endl;
        //开始游戏
        strncpy(sendBuf,"id:1,type:2",sizeof("id:1,type:2"));
        Write(temp->player1,sendBuf,sizeof(sendBuf));	
        strncpy(sendBuf,"id:2,type:2",sizeof("id:2,type:2"));
        Write(temp->player2,sendBuf,sizeof(sendBuf));	
        strncpy(sendBuf,"id:3,type:2",sizeof("id:3,type:2"));
        Write(temp->player3,sendBuf,sizeof(sendBuf));	
    }
    //pthread_mutex_unlock(&roomLock);
    cout<<"卡这里"<<endl;
    if(receive.find("type:1") != string::npos)
    {

    }
    if(receive.find("type:2") != string::npos)
    {
        cout<<888888<<endl;
        catchLandlor(temp,receive);
    }
    if(receive.find("type:3") != string::npos)
    {

    }
    if(receive.find("type:4") != string::npos)
    {

    }
}
//抢地主
void catchLandlor(GameInit* temp,string msg)
{
    string m_msg = msg;
    char m_sendmsg[1500];
    GameInit* m_room = temp;
    if(m_msg.find("id:1") != string::npos && m_msg.find("value:1") != string::npos)
    {
        //pthread_mutex_lock(&roomLock);
        if(m_room->nowgame->callnum == 0){ 

            m_room->nowgame->play1call = 0;
            m_room->nowgame->whocall = 1;
            strncpy(m_sendmsg,"id:1,type:2,value:1,next:2,call:1,catch:0",sizeof("id:1,type:2,value:1,next:2,call:1,catch:0"));
        }
        else{
            m_room->nowgame->play1catch = 0;
            strncpy(m_sendmsg,"id:1,type:2,value:1,next:2,call:0,catch:1",sizeof("id:1,type:2,value:1,next:2,call:0,catch:1"));
        }
        cout<<"==============="<<endl;
        m_room->nowgame->callnum++;
        //pthread_mutex_unlock(&roomLock);

        if(int ret = whoLandlord(m_room) >= 0)
        {
            cout<<"找到地主"<<ret<<endl;
            return;
        }
        //pthread_mutex_lock(&roomLock);
        Write(m_room->player1,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player2,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player3,m_sendmsg,sizeof(m_sendmsg));	
        //pthread_mutex_unlock(&roomLock);
    }
    if(m_msg.find("id:1") != string::npos && m_msg.find("value:0") != string::npos)
    {
        //pthread_mutex_lock(&roomLock);
        m_room->nowgame->callnum++;
        //pthread_mutex_unlock(&roomLock);
        if(int ret = whoLandlord(m_room) >= 0)
        {
            cout<<"找到地主"<<ret<<endl;
            return;
        }
        strncpy(m_sendmsg,"id:1,type:2,value:0,next:2",sizeof("id:1,type:2,value:0,next:2"));
        //pthread_mutex_lock(&roomLock);
        Write(m_room->player1,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player2,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player3,m_sendmsg,sizeof(m_sendmsg));	
        //pthread_mutex_unlock(&roomLock);

    }
    if(m_msg.find("id:2") != string::npos && m_msg.find("value:1") != string::npos)
    {
        //pthread_mutex_lock(&roomLock);
        cout<<"1"<<endl;
        if(m_room->nowgame->callnum == 1){
            cout<<"2"<<endl;
            if(m_room->nowgame->play1call == 1 && m_room->nowgame->play2call == 1 && m_room->nowgame->play3call == 1){
                m_room->nowgame->whocall = 2;
                m_room->nowgame->play2call = 0;
                strncpy(m_sendmsg,"id:2,type:2,value:1,next:3,call:1,catch:0",sizeof("id:2,type:2,value:1,next:3,call:1,catch:0"));
            }
            else{
                m_room->nowgame->play2catch = 0;
                strncpy(m_sendmsg,"id:2,type:2,value:1,next:3,call:0,catch:1",sizeof("id:2,type:2,value:1,next:3,call:0,catch:1"));
            }
        }
        else{
            strncpy(m_sendmsg,"id:2,type:2,value:1,next:3,call:0,catch:1",sizeof("id:2,type:2,value:1,next:3,call:0,catch:1"));
            m_room->nowgame->play2catch = 0;
            cout<<"3"<<endl;
        }
        m_room->nowgame->callnum++;
        //pthread_mutex_unlock(&roomLock);
        if(int ret = whoLandlord(m_room) >= 0)
        {
            cout<<"找到地主"<<ret<<endl;
            return;
        }
        //pthread_mutex_lock(&roomLock);
        Write(m_room->player1,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player2,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player3,m_sendmsg,sizeof(m_sendmsg));	
        //pthread_mutex_unlock(&roomLock);

    }
    if(m_msg.find("id:2") != string::npos && m_msg.find("value:0") != string::npos)
    {
        //pthread_mutex_lock(&roomLock);
        m_room->nowgame->callnum++;
        //pthread_mutex_unlock(&roomLock);
        if(int ret = whoLandlord(m_room) >= 0)
        {
            cout<<"找到地主"<<ret<<endl;
            return;
        }
        strncpy(m_sendmsg,"id:2,type:2,value:0,next:3",sizeof("id:2,type:2,value:0,next:3"));
        //pthread_mutex_lock(&roomLock);
        Write(m_room->player1,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player2,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player3,m_sendmsg,sizeof(m_sendmsg));	
        //pthread_mutex_unlock(&roomLock);

    }
    if(m_msg.find("id:3") != string::npos && m_msg.find("value:1") != string::npos)
    {
        //pthread_mutex_lock(&roomLock);
        if(m_room->nowgame->play1call == 1 && m_room->nowgame->play2call == 1){
            m_room->nowgame->play3call = 0;
            strncpy(m_sendmsg,"id:3,type:2,value:1,next:1,call:1,catch:0",sizeof("id:3,type:2,value:1,next:1,call:1,catch:0"));
        }
        else{
            m_room->nowgame->play3catch = 0;
            strncpy(m_sendmsg,"id:3,type:2,value:1,next:1,call:0,catch:1",sizeof("id:3,type:2,value:1,next:1,call:0,catch:1"));
        }
        m_room->nowgame->callnum++;
        //pthread_mutex_unlock(&roomLock);
        if(int ret = whoLandlord(m_room) >= 0)
        {
            cout<<"找到地主"<<ret<<endl;
            return;
        }
        //pthread_mutex_lock(&roomLock);
        Write(m_room->player1,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player2,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player3,m_sendmsg,sizeof(m_sendmsg));	
        //pthread_mutex_unlock(&roomLock);

    }
    if(m_msg.find("id:3") != string::npos && m_msg.find("value:0") != string::npos)
    {
        //pthread_mutex_lock(&roomLock);
        m_room->nowgame->callnum++;
        //pthread_mutex_unlock(&roomLock);
        if(int ret = whoLandlord(m_room) >= 0)
        {
            cout<<"找到地主"<<ret<<endl;
            return;
        }
        strncpy(m_sendmsg,"id:3,type:2,value:0,next:1",sizeof("id:3,type:2,value:0,next:1"));
        //pthread_mutex_lock(&roomLock);
        Write(m_room->player1,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player2,m_sendmsg,sizeof(m_sendmsg));	
        Write(m_room->player3,m_sendmsg,sizeof(m_sendmsg));	
        //pthread_mutex_unlock(&roomLock);

    }
    cout<<m_sendmsg<<endl;
}

//判断地主
int whoLandlord(GameInit* temp)
{
    GameInit* m_room = temp;
    //pthread_mutex_lock(&roomLock);
    if(m_room->nowgame->callnum == 3)
    {
        if(m_room->nowgame->play1call == m_room->nowgame->play2call == m_room->nowgame->play3call == 1){return 0;}//没有人叫地主
        if(m_room->nowgame->play1call == 0 && m_room->nowgame->play2call == 1 && m_room->nowgame->play3call == 1 && m_room->nowgame->play2catch == 1 && m_room->nowgame->play3catch == 1){return 1;}//1号地主
        if(m_room->nowgame->play2call == 0 && m_room->nowgame->play3call == 1 && m_room->nowgame->play1call == 1 && m_room->nowgame->play3catch == 1){return 2;}//2号地主
        if(m_room->nowgame->play3call == 0 && m_room->nowgame->play1call == 1 && m_room->nowgame->play2call == 1){return 3;}//3号地主
    }
    if(m_room->nowgame->callnum == 4)
    {
        if(m_room->nowgame->whocall == 1)
        {
            if(m_room->nowgame->play1catch == 1){
                if(m_room->nowgame->play2catch == 0){
                    if(m_room->nowgame->play3catch == 0){return 3;}//3号地主
                    else{return 2;}//2号地主
                }
                else if(m_room->nowgame->play3catch == 0){return 3;}//3号地主
            }
            else{return 1;}//1号地主
        }
    }
    if(m_room->nowgame->callnum == 5)
    {
        if(m_room->nowgame->whocall == 2)
        {
            if(m_room->nowgame->play2catch == 0){return 2;}//2号地主
            else
            {
                if(m_room->nowgame->play1catch == 0){return 1;}//1号地主
                else if(m_room->nowgame->play3catch == 0){return 3;}//3号地主
            }
        }
    }
    //pthread_mutex_unlock(&roomLock);
    return -1;
}
