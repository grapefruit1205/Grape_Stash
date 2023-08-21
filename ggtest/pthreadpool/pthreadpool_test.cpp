#include "pthreadpool.h"
#include "pthreadpool.cpp"

void taskfunction(void* arg)
{
  int num = *(int*)arg;
  printf("thread %d is working,number = %d\n",(int)pthread_self(),num);
  //sleep(1);
}

int main()
{
  Thread_pool<int> pool(5,20);
  for(int i = 0;i < 20000;i++)
  {
    int* num = new int(i+100);
    pool.addTask(Task<int>(taskfunction,num));
  }
  getchar();
  return 0;
}
