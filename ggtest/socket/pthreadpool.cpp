#include "pthreadpool.h"

template<typename T>
TaskQueue<T>::TaskQueue()
{
  pthread_mutex_init(&m_mutex,NULL);
}

template<typename T>
TaskQueue<T>::~TaskQueue()
{
  pthread_mutex_destroy(&m_mutex);
  
}
template<typename T>
void TaskQueue<T>::addTask(Task<T> task)
{
  pthread_mutex_lock(&m_mutex);
  mTaskQ.push(task);
  pthread_mutex_unlock(&m_mutex);

}

template<typename T>
void TaskQueue<T>::addTask(void(*f)(void* arg),void* arg)
{
  pthread_mutex_lock(&m_mutex);
  mTaskQ.push(Task<T>(f,arg));
  pthread_mutex_unlock(&m_mutex);

}

template<typename T>
Task<T> TaskQueue<T>::takeTask()
{
  Task<T> P;
  pthread_mutex_lock(&m_mutex);
  if(mTaskQ.size()>0)
  {
    P = mTaskQ.front();
    mTaskQ.pop();
  }
  pthread_mutex_unlock(&m_mutex);
  return P;
}

template<typename T>
Thread_pool<T>::Thread_pool(int min,int max)
{
  //实例化线程池
  do{
      taskQ = new TaskQueue<T>;
      threadIDs = new pthread_t[max];
      if(threadIDs == NULL)
      {
        cout<<"malloc threadIDs fail...";
      }
      memset(threadIDs,0,max);
      minNum = min;
      maxNum = max;
      busyNum = 0;
      liveNum = min;
      if(pthread_mutex_init(&mutexPool,NULL) != 0 || pthread_cond_init(&notEmpty,NULL) != 0)
      {
        cout<<"mutex or cond init fail....";
        break;
      }
  
      shutdown = false;
      pthread_create(&managerID,NULL,manager,this);
      for(int i = 0;i < min; i++)
      {
        pthread_create(&threadIDs[i],NULL,woker,this);
      }
      return;
  }while(0);
  //释放资源
  if(threadIDs) delete[] threadIDs;
  if(taskQ) delete taskQ;
}

template<typename T>
Thread_pool<T>::~Thread_pool()
{
  //关闭线程池
  shutdown = 1;
  //阻塞回收管理者线程
  pthread_join(managerID,NULL);
  //唤醒消费者线程
  for(int i = 0;i < liveNum;i++)
  {
    pthread_cond_signal(&notEmpty);
  }
  //释放堆内存
  delete taskQ;
  delete[] threadIDs;
  pthread_mutex_destroy(&mutexPool);
  pthread_cond_destroy(&notEmpty);
}

template<typename T>
int Thread_pool<T>::getLiveNum()
{
  pthread_mutex_lock(&mutexPool);
  int livenum = this->liveNum;
  pthread_mutex_unlock(&mutexPool);
  return livenum;
}

template<typename T>
int Thread_pool<T>::getBusyNum()
{
  pthread_mutex_lock(&mutexPool);
  int busynum = this->busyNum;
  pthread_mutex_unlock(&mutexPool);
  return busynum;
}

template<typename T>
void Thread_pool<T>::addTask(Task<T> task)
{
  if(shutdown)
  {
    return;
  }
  //添加任务
  taskQ->addTask(task);
  pthread_cond_signal(&notEmpty);
}

template<typename T>
void* Thread_pool<T>::woker(void* arg)
{
  Thread_pool* pool = static_cast<Thread_pool*>(arg);
  while(1)
  {
    pthread_mutex_lock(&pool->mutexPool);
    while(pool->taskQ->getTasknum() == 0 && !pool->shutdown)
    {
      //阻塞工作线程
      pthread_cond_wait(&pool->notEmpty,&pool->mutexPool);
      //判断是否销毁线程
      if(pool->exitNum > 0)
      {
        pool->exitNum--;
        if(pool->liveNum > pool->minNum)
        {
          pool->liveNum--;
          pthread_mutex_unlock(&pool->mutexPool);
          pool->threadExit();
        }
      }
    }
    //判断线程池是否关闭
    if(pool->shutdown)
    {
      pthread_mutex_unlock(&pool->mutexPool);
      pool->threadExit();
    }
    //从任务队列中取出任务
    Task<T> task = pool->taskQ->takeTask();
    pool->busyNum++;
    pthread_mutex_unlock(&pool->mutexPool);

    task.function(task.arg);
    delete task.arg;
    task.arg = NULL;
    pthread_mutex_lock(&pool->mutexPool);
    pool->busyNum--;
    pthread_mutex_unlock(&pool->mutexPool);

  }
  return NULL;
}

template<typename T>
void* Thread_pool<T>::manager(void* arg)
{
  Thread_pool* pool = static_cast<Thread_pool*>(arg);
  while(!pool->shutdown)
  {
    sleep(1);
    //取出线程池中任务的数量和当前线程的数量
    pthread_mutex_lock(&pool->mutexPool);
    int queuesize = pool->taskQ->getTasknum();
    int liveNum = pool->liveNum;
    int busyNum = pool->busyNum;
    pthread_mutex_unlock(&pool->mutexPool);
    printf("---当前有%d个线程在工作----剩余任务%d个----存活线程%d个\n",busyNum,queuesize,liveNum);
    //添加线程
    //任务个数>存活线程个数 &&  存活线程个数<最大线程数
    if(queuesize>liveNum && liveNum<pool->maxNum)
    {
      pthread_mutex_lock(&pool->mutexPool);
      int counter = 0;
      for(int i=0;i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum;i++)
      {
        if(pool->threadIDs[i] == 0)
        {
          pthread_create(&pool->threadIDs[i],NULL,woker,pool);
          counter++;
          pool->liveNum++;
        }
      }
      pthread_mutex_unlock(&pool->mutexPool);
    }

    //删除线程
    //忙的线程个数*2 < 存活线程个数 && 存活线程个数 > 最小线程数
    if(busyNum*2 < liveNum && liveNum > pool->minNum)
    {
      pthread_mutex_lock(&pool->mutexPool);
      pool->exitNum = NUMBER;
      pthread_mutex_unlock(&pool->mutexPool);
      for(int i = 0;i < NUMBER ;i++)
      {
        pthread_cond_signal(&pool->notEmpty);
      }
    }
  }
  return NULL;
}

template<typename T>
void Thread_pool<T>::threadExit()
{
  pthread_t tid = pthread_self();
  for(int i = 0;i < maxNum; i++)
  {
    if(tid == threadIDs[i])
    {
      threadIDs[i] = 0;
      break;
    }
  }
  pthread_exit(NULL);
}
