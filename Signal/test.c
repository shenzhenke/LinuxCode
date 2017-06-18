#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <error.h>
void catchsignal(int sig)
{
    printf("I am a father my pid is:%d , receive signal is %d\n",getpid(),sig);
}
int main()
{
    signal(SIGCHLD,catchsignal);
    pid_t id=fork();
    if(id==0){
        //child
       int count=0;
       while(count<4){
           printf("I am a child,my pid is: %d ,my father's pid is: %d \n",getpid(),getppid());
           sleep(1);
           count++;
       }
       exit(0);
    }
    else if(id>0){
        //father
        while(1){
            sleep(2);
            printf("I am a father,running\n");
        }
    }
    else{
        perror("fork error\n");
    }

    return 0;
}
