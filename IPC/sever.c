#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <error.h>
#include <sys/shm.h> //shared memory  共享内存用来传递数据
#include <sys/msg.h> //message queue  消息队列用来修改共享内存之后,通知服务器读取
#include <sys/sem.h> //semaphore      信号量用来同步
#define PATHNAME "."
#define PROJ_ID 0
//消息队列结构
struct msg_form{
    long mtype; //消息类型
    char mtext; //消息数据
};

//联合体,用于初始化semctl
union semun{
    int val ;                   //用于SETVAL命令
    struct semid_ds* buf;       //用于IPC_STAT和IPC_SET命令
    unsigned short* array;      //用于GETALL和SETALL命令
};

//
int init_sem(int sem_id,int val)
{
    union semun tmp;
    tmp.val=val;
    if(semctl(sem_id,0,SETVAL,tmp)==-1){
//sem_id	
	perror("init_sem failed\n");
	return -1;
    }
    return 0;
}

//struct sembuf{
//    unsigned short int sem_num; //信号量集中信号的编号
//    short int sem_op;           //指定操作类型,可选值为正整数、负整数和0.
//    short int sem_flg;      
//    //可选值为IPC_NOWAIT(无论操作是否成功,semop调用都将立即返回)
//    //IPC_UNDO,当进程退出时取消正在进行的semop操作
//}

//pv操作
//执行p操作申请信号量的值减1
int sem_p(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num=0;
    sem_b.sem_op=-1;
    sem_b.sem_flg=SEM_UNDO;
    if(semop(sem_id,&sem_b,1)==-1){
        perror("sem_p failed\n");
        return -1;
    }
    return 0;
}

//执行v操作释放信号量的值加1
int sem_v(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num=0;
    sem_b.sem_op=1;
//op > 0,表示semop的信号量的值semval增加到sem_op
    sem_b.sem_flg=SEM_UNDO;
    if(semop(sem_id,&sem_b,1)==-1){
        perror("sem_v failed\n");
        return -1;
    }
    return 0;
}

//删除信号量
int del_sem(int sem_id)
{
    union semun tmp;
    if(semctl(sem_id,0,IPC_RMID,tmp)==-1){
        perror("delete sem failed\n");
        return -1;
    }
    return 0;
}

//创建信号量
int create_sem(key_t key)
{
    int sem_id;
  sem_id= semget(key,1,IPC_CREAT|0666);
     if( sem_id==-1){
//num_sems（第二个参数）表示创建多少个信号量集
//sem_flags 参数指定一组标志,低端的9个比特位表示该信号量的权限
//IPC_CREAT|IPC_EXCL标志创建全新唯一的信号量集
      perror("semget error\n");
      exit(-1);
  }
  init_sem(sem_id,1);
  return sem_id;
}

int main()
{
    key_t key;
    int shmid,semid,msqid;
    char *shm;
    char data[]="this is sever";
    struct shmid_ds buf1; //用于删除共享内存
    struct msqid_ds buf2; //用于删除消息队列
    struct msg_form msg; //消息队列用于通知对方更新了共享内存
    

    //获取key值
    key=ftok(PATHNAME,PROJ_ID);
    if(key<0){
        perror("ftok error\n");
        exit(1);
    }

    //创建共享内存
    shmid=shmget(key,1024,IPC_CREAT|0666);
    if(shmid==-1){
        perror("shmget failed\n");
        exit(1);
    }

    //挂接共享内存
     shm=(char*)shmat(shmid,NULL,0);
     //shm_addr设置为NULL(第二个参数),表示被关联的地址由操作系统决定,这样可以确保代码的可移植性 
     //shmflg未设置,则共享内存被关联到addr指定的地质处
     if((int)shm ==-1){
        perror("shmat failed\n");
        exit(1);
    }
   
   //创建消息队列
    msqid=msgget(key,IPC_CREAT|0777);
    if(msqid==-1){
        perror("msgget failed\n");
        exit(1);
    }
    

   //创建信号量
   semid=create_sem(key);

   //读数据
   while(1){
  	   msgrcv(msqid,&msg,1,888,0); //读取数据类型为888的数据
         //读数据在哪个消息队列+读取的数据存储在哪里+读取多长的数据+读取什么类型的数据+msgflg
     
	if(msg.mtext=='q')  //代表退出
         {
             break;
         }


        //读取共享内存的数据
         if(msg.mtext=='r'){
            sem_p(semid);   //读之前申请信号量
            printf("%s\n",shm);
            sem_v(semid);  //读之后释放信号量 
         }
   }
    
    //断开共享内存的连接
    shmdt(shm);
    
    //删除消息队列、共享内存、信号量

    shmctl(shmid,IPC_RMID,&buf1);
    msgctl(msqid,IPC_RMID,&buf2);
    del_sem(semid);
    return 0;
}

