#ifndef _PTHREADPOOL_H_
#define _PTHREADPOOL_H_

#include <pthread.h>
#include <queue>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <sys/epoll.h>
using namespace std;
struct clientmsg
{
    int efd;
    int lfd;
    int n;
    struct epoll_event ev;
    struct epoll_event* evarry;
}clmsg;

template <typename T>
struct Task 
{
  Task()
  {
    function = NULL;
    arg = NULL;
  }
  Task(void (*fun)(void* arg),void* arg)
  {
    this->arg = (T*)arg;
    this->function = fun;
  }
  T* arg;
  void(*function)(void* arg);
};

template <typename T>
class TaskQueue 
{
public:
  TaskQueue();
  ~TaskQueue();
  //添加任务
  void addTask(Task<T> task);
  void addTask(void(*fun)(void*arg),void* arg);
  //取出一个任务
  Task<T> takeTask();
  //获取当前任务个数
  inline size_t getTasknum()
  {
    return mTaskQ.size();
  }
private:
  queue< Task<T> > mTaskQ;  
  pthread_mutex_t m_mutex;
};

template <typename T>
class Thread_pool
{
public:
  Thread_pool(int min,int max);
  ~Thread_pool();
  void addTask(Task<T> task);
  int getBusyNum();
  int getLiveNum();
private:
  //工作的线程
  static void* woker(void* arg);
  //管理线程任务函数
  static void* manager(void* arg);
  void threadExit();
private:
  TaskQueue<T>* taskQ;

  pthread_t managerID;  //管理者线程
  pthread_t* threadIDs;  //工作者线程
  int minNum;
  int maxNum;
  int busyNum;
  int liveNum;
  int exitNum;
  pthread_mutex_t mutexPool;
  pthread_cond_t notEmpty;
  int shutdown;
  static const int NUMBER = 2;
};
  
#endif
