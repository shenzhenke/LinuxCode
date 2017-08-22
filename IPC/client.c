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

//初始化信号量
int init_sem(int sem_id,int val)
{
    union semun tmp;
    tmp.val=val;
    if(semctl(sem_id,0,SETVAL,tmp)==-1){ 
//sem_id指定被操作的信号量集,0即sem_num参数指定被操作的信号量在信号量集的编号
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



int main()
{
    key_t key;
    int shmid,semid,msqid;
    char *shm;

    struct msg_form msg; //消息队列用于通知对方更新了共享内存
    

    //获取key值
    key=ftok(PATHNAME,PROJ_ID);
    if(key<0){
        perror("ftok error\n");
        exit(1);
    }

    //创建共享内存
   
    shmid=shmget(key,1024,0);
    if(shmid<0){
        //最后一个参数设置为0表示获取已有的共享内存段
        perror("shmget failed\n");
        exit(1);
    }

    //挂接共享内存
     shm= (char*)shmat(shmid,NULL,0);
     //shm_addr设置为NULL(第二个参数),表示被关联的地址由操作系统决定,这样可以确保代码的可移植性 
     //shmflg未设置,则共享内存被关联到addr指定的地质处
    if((int)shm ==-1){
       perror("shmat failed\n");
        exit(1);
    }
   
   //获得消息队列
    msqid=msgget(key,0);
    if(msqid==-1){
        perror("msgget failed\n");
        exit(1);
    }

   //获得信号
   semid=semget(key,0,0);
 
   if(semid==-1){
        perror("gain semid failed\n");
        exit(1);
   }

   printf("..............................................\n");
   printf("................... IPC ......................\n");
   printf(".........input s send message to sever........\n");
   printf(".............input q to quit..................\n");
   printf("..............................................\n");

   int flag=1;
   //读数据
   while(flag){
         char c;
         scanf("%c",&c);
         switch(c)
         {
             case 'r':
	     {
		 printf("Date to send:");
		 sem_p(semid);
		 scanf("%s",shm);
		 sem_v(semid);

		 //清空缓冲区
		 while((c=getchar())!='\n'&&c!=EOF);
		 msg.mtype=888;
		 msg.mtext='r';
		 msgsnd(msqid,&msg,sizeof(msg.mtext),0);
		 break;
	     }
             case 'q':
	     {
		 printf("quit\n");
		 msg.mtype=888;
		 msg.mtext='q';
		 msgsnd(msqid,&msg,sizeof(msg.mtext),0);
		 flag=0;
		 break;
	     }	
             default:
             printf("wrong input\n");
             while((c=getchar())!='\n'&&c!=EOF);
         }    
   }
    
    //断开共享内存的连接
    shmdt(shm);
    return 0;
}

