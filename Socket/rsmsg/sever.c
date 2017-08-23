#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <assert.h>

#define BUF_SIZE 1024
int main(int argc,char *argv[])
{
    if(argc<=2)
    {
	printf("%s,usage:ip_addresss port_number\n",basename(argv[0]));
	return 1;
    }
    const char *ip=argv[1];
    int port=atoi(argv[2]);
    struct sockaddr_in sever_address;
    bzero(&sever_address,sizeof(sever_address));
    sever_address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&sever_address.sin_addr);
    //ip本机转网络 什么地址族 + ip src地址 + ip dest地址
    sever_address.sin_port =htons(port);

    int sock=socket(PF_INET,SOCK_STREAM,0);
    //什么协议族 + 服务类型 + 具体协议(一般前两个就指定具体的协议了,所以设置为默认)
    assert(sock>0);

    int ret=bind(sock,(struct sockaddr *)&sever_address,sizeof(sever_address));
    //绑定sock文件描述符 + sock地址
    //sock文件描述符 + 强转的sock的通用地址 +地址长度
    assert(ret!=-1);

    ret = listen(sock,5);
    //监听sock,第二个参数提示内核监听队列的最大长度,一般完整链接有backlog+1个
    //即处于ESTABLISHED的连接,其余都处于SYN_RCVD状态
    assert(ret!=-1);

    struct sockaddr_in client;
    socklen_t client_addrlength=sizeof(client);
    int connfd=accept(sock,(struct sockaddr *)&client,&client_addrlength);
    if(connfd<0)
    {
	printf("errno is:%d\n",errno);
    }
    else
    {
	char buffer[BUF_SIZE];

	memset(buffer,'\0',BUF_SIZE);
	ret =recv(connfd,buffer,BUF_SIZE-1,0);
	printf("got %d bytes of normal_data '%s'\n",ret,buffer);


	memset(buffer,'\0',BUF_SIZE);
	ret =recv(connfd,buffer,BUF_SIZE-1,MSG_OOB);
	printf("got %d bytes of normal_data '%s'\n",ret,buffer);


	memset(buffer,'\0',BUF_SIZE);
	ret =recv(connfd,buffer,BUF_SIZE-1,0);
	printf("got %d bytes of normal_data '%s'\n",ret,buffer);

	close(connfd);
    }

    close(sock);
    return 0;
}
