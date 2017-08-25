#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
pthread_mutex_t mutex;
void *anther(void *arg)
{
    printf("in child pthread,lock the mutex\n");
    pthread_mutex_lock(&mutex);
    sleep(5);
    pthread_mutex_unlock(&mutex);
}
void prepare()
{
    pthread_mutex_lock(&mutex);
}

void infork()
{
    pthread_mutex_unlock(&mutex);
}

int main()
{
    pthread_mutex_init (&mutex,NULL);  //第二个参数为互斥锁的属性
    pthread_t id;
    pthread_create(&id,NULL,anther,NULL);


/*----------------------------------------------------------------------------
 int pthread_create(pthread_t *id,const pthread_attr_t* attr,
 			void* (*start_routine)(void*),void* arg)
 attr指定线程属性,start_routine参数指定新线程将运行的函数以及其参数
 成功返回0,失败返回错误码
 ---------------------------------------------------------------------------*/



    //父进程中的主线程暂停一秒钟,以确保在执行fork之前,子线程已经开始并获得互斥变量
    sleep(1);
    pthread_atfork(prepare,infork,infork);

/*-------------------------------------------------------------------------------------------
该函数保证了fork调用之后,都拥有一个清楚的锁状态,解决之前的问题:
互斥锁已经被加锁,而且不是被调用fork函数的那个线程锁住的,而是由其他线程锁住的,在这种情况下
子进程若再次对该互斥锁执行加锁操作就会导致死锁
----------------------------------------------------------------------------------------------*/

    int  pid=fork();
    if(pid==0){
	printf("I am in the child,want to get lock\n");
	pthread_mutex_lock(&mutex);
	printf("I can not run to here ,oop...\n");
	pthread_mutex_unlock(&mutex);
    }else if(pid<0){
	pthread_join(id,NULL);


/*---------------------------------------------------------------------------------------------
 	int pthread_join(pthread_t thread,void **retval);
thread 参数指定线程的标识符,retval参数则是目标线程返回的退出信息
该函数会一直阻塞,直到被回收的线程结束为止,成功返回0,失败返回错误码
EDEADLK 可能引起死锁,比如两个线程互相针对对方调用pthread_join,或者线程对自身调用
EINVAL  目标线程不可回收,或者已经有其他线程在回收该目标线程
ESRCH   目标线程不存在
 ---------------------------------------------------------------------------------------------*/ 



/*--------------------------------------线程退出方式--------------------------------------------
 	void pthread_exit(void *retval);
通过retval参数向线程的回收者传递其退出信息,它执行完之后不会返回调用者,而且永远不会失败
retval应该是全局或者malloc分配的变量,不能在线程函数的栈上分配

	int pthread_cancel(pthread_t thread);

-------------------------------------------------------------------------------------------- */ 


/*---------------------------------------分离线程----------------------------------------------
 	pthread_detach(pthread_self()); //子线程
	或
	pthread_detach(pthread_id);    //父线程调用(非阻塞版)
	在任何一个时间点上,线程是可结合的或者可分离的,默认是可结合的,
	一个可结合的线程是可以被其他线程杀死或者回收的,在这之前他的存储器资源(如栈)是不会被回收的
	一个可分离的线程是不能被其他线程所回收或杀死的,它的存储器资源在它运行结束之后自动释放
	如果一个可结合的线程没有被join就会产生类似于僵尸进程的问题
 ---------------------------------------------------------------------------------------------*/

	pthread_mutex_destroy(&mutex);
    }else{
	wait(NULL);  //阻塞式等待任一子进程
    }
    
    pthread_join(id,NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}
