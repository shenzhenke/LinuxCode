#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <error.h>
#include <stdlib.h>
static void sigAlrm(int signo)
{
    return ;
}
unsigned int  mysleep(unsigned int nsecs)
{
    struct sigaction newact,oldact;
    sigset_t newmask,oldmask,suspmask; 
    unsigned int unslept=0;
    newact.sa_handler=sigAlrm;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags=0;
    if(sigaction(SIGALRM,&newact,&oldact)==0){
       alarm(nsecs);
       pause();
        unslept=alarm(0);
        return unslept;
    }
    else{
        perror("sigacion error\n");
    }
}
static void sig_int(int signo)
{
    int i;
    int j=0;
    printf("sig_int start\n");
    for(i=0;i<10;i++)
    {
        j+=i;
        printf("%d\n",j);
    }
    printf("sig_int finished\n");
    return ;
}
int main()
{
    unsigned int unslept;
    if(signal(SIGINT,sig_int)==SIG_ERR)
        perror("signal(SIGINT) error\n");
        sleep(10);
    unslept=mysleep(2);
    printf("sleep2 returned : %u\n",unslept);
    return 0;
}
