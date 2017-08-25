//--------------------------------封装3种线程同步机制的类--------------------------------


#ifndef LOCKER_H
#define LOCKER_H
#include <exception>
#include <pthread.h>
#include <semphore.h>

//封装信号量的类
class sem 
{
public:
	sem()
	{
	    if(sem_init(&m_sem,0,0)!=0)


/*-------------------------------初始化信号量-----------------------------------------------
 		int sem_init(sem_t * sem,int pshared,unsigned int value);
pshared参数指定信号量的类型,值为0,表明这个信号量是当前进程的局部信号量,否则为多进程之间共享
value指定信号量的初始值,不能初始化一个已经初始化的信号量
------------------------------------------------------------------------------------------*/


	    {
		//构造函数没有返回值,可以通过抛异常来报告错误
		throw std::exception();
	    }
	}

	~sem()
	{
	    sem_destroy(&m_sem);

/*------------------------------------销毁信号量--------------------------------------------
 		   int sem_destroy(sem_t *sem);
	  销毁信号量,释放其内核资源,不可销毁一个正在被线程等待的信号量
-----------------------------------------------------------------------------------------*/

	}

	//等待信号量
	bool wait()
	{
	    return sem_wait(&m_sem)==0;

/*----------------------------------------------------------------------------------------------
 		(1)		int sem_wait(sem_t *sem);
		(2)		int sem_trywait(sem_t *sem);(非阻塞版本)
(1)	以原子操作方式将信号量减1。如果信号量的值为0,则sem_wait将被阻塞,直到这个信号量具有非0值
(2)	信号量值非0时,对信号量执行减1操作,为0时,返回-1并设置errno为EAGAIN
---------------------------------------------------------------------------------------------*/

	}

	//增加信号量
	bool post()
	{
	    return sem_post(&m_sem);

/*--------------------------------------V操作-------------------------------------------------
 	                    	int sem_post(sem_t *sem);
            使信号量+1,当信号量的值大于0时,其他调用的sem_wait等待信号量的线程将被唤醒
 -------------------------------------------------------------------------------------------*/

	}
private:
	sem_t m_sem;
}

//封装互斥锁的类

class locker
{
public:
	locker()
	{
	    if(pthread_mutex_init(&m_mutex,NULL)!=0)

/*---------------------------初始化互斥锁------------------------------------------------------
 	 int pthread_mutex_init(pthread_mutex_t *mutex,const pthread_mutexattr_t *mutexattr)
	 mutex指向要操作的互斥锁,mutexattr指定互斥锁的属性,如果设置为NULL,代表默认属性
	 如果一个mute是全局的或者静态分配的,可以调用宏来将互斥锁的各个字段初始化为0
	 pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
----------------------------------------------------------------------------------------------*/
	    
	    {
		throw std::exception();
	    }
	}


	~locker()
	{
	    pthread_mutex_destroy(&m_mutex);

/*------------------------------------销毁互斥锁-----------------------------------------------
 		int   pthread_mutex_destroy(pthread_mutex_t *mutex);
				销毁互斥锁释放其内核资源
----------------------------------------------------------------------------------------------*/

	}

	bool lock()
	{
	    return pthread_mutex_lock(&m_mutex);

/*-------------------------------------加锁(原子操作)-------------------------------------------
 	(1)	int   pthread_mutex_lock(pthread_mutex_t *mutex);
	(2)	int   pthread_mutex_trylock(pthread_mutex_t *mutex);
对于lock而言,如果目标互斥锁已经被锁上,则pthread_mutex_lock调用将阻塞,直到占用者将其解锁
对于trylock,一个线程既想获得锁,又不想被挂起等待,可以调用trylock,如果互斥锁已经加锁,则它
返回EBUSY错误码
----------------------------------------------------------------------------------------------*/

	}

	bool unlock()
	{
	    return pthread_mutex_unlock(&m_mutex);

/*--------------------------------------解锁------------------------------------------------------
 		int pthread_mutex_unlock(pthread_mutex_t * mutex);
---------------------------------------------------------------------------------------------*/	

	}

private:
	pthread_mutex_t m_mutex;
}


class cond
{
public:
	cond()
	{
        if(pthread_mutex_init(&m_mutex,NULL)!=0)
        {
            throw std::exception();
        }
        if(pthread_cond_init(&m_cond,NULL)!=0)

/*-------------------------------------------------------------------------------------
           int pthread_cond_init(pthread_cond_t *cond,pthread_condattr_t *cond_attr);
第二个参数指定属性和互斥锁差不多,如果cond为全局或者静态变量考虑用宏初始化
pthread_cond_t cond =PTHREAD_COND_INITIALIZER;
 -------------------------------------------------------------------------------------*/

        {
            //构造函数一旦出现问题,就应该立即释放已经分配成功的资源
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
	}
	~cond
	{
		pthread_mutex_destroy(&m_mutex);
		pthread_cond_destroy(&m_cond);

/*--------------------------------销毁条件变量----------------------------------------------
            int pthread_cond_destroy(pthread_cond_t *cond);
            销毁一个正在等待的条件变量将失败并且返回EBUSY
 -----------------------------------------------------------------------------------------*/

	}

	bool wait()
	{
	    int ret=0;
	    pthread_mutex_lock(&m_mutex);
	    ret=pthread_cond_wait(&m_cond,&m_mutex);

/*-------------------------------------------------------------------------------------------
        int pthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex);
条件变量+互斥锁配对使用(互斥锁用于同步线程对共享数据的访问,条件变量用于在线程之间同步共享数据的值)
mutex保护条件变量的互斥锁,以保证pthread_cond_wait操作的原子性,该函数不会错过任何目标条件变量的变化
该函数主要执行三步：
(1)释放mutex
(2)阻塞等待
(3)当被唤醒时,重新获得mutex并返回
-------------------------------------------------------------------------------------------- */
	    pthread_mutex_unlock(&m_mutex);
	    return ret==0;
	}

	bool signal()
	{
	    return pthread_cond_signal(&m_cond)==0;

/*-------------------------------------------------------------------------------------------
      (1)  int pthread_cond_signal(pthread_cond_t *cond);
      (2)  int pthread_cond_broadcast(pthread_cond_t *cond);
(1)用于唤醒一个等待的目标条件变量的线程,至于是哪一个取决于优先级和调度策略
(2)用广播的方式唤醒所有等待目标条件变量的线程
如何等待唤醒指定线程？
1.定义一个能够唯一表示目标线程的全局变量,在唤醒等待条件变量的线程前先设置该变量为目标线程
2.然后采用广播的方式唤醒所有等待条件变量的目标线程
3.这些被唤醒的线程都检查该变量是否是自己,是就执行后续代码,不是就返回继续等待

------------------------------------------------------------------------------------------- */
	}
private:
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
}

#endif 
