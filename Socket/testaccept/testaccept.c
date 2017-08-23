#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc,char* argv[])
{
    if(argc<=2)
    {
	printf("usage: %s ip_address port_number\n",basename(argv[0]));
	return 1;
    }

    const char* ip=argv[1];
    int port=atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;  //  地址族
    //ip地址转换--------->网络字节序
    inet_pton(AF_INET,ip,&address.sin_addr);  //ipv4结构体

    address.sin_port=htons(port); //主机字节序和网络字节序的交换

    int sock=socket(PF_INET,SOCK_STREAM,0); //协议族 + 服务类型 + 具体协议(默认)
    assert(sock>=0);

    int ret=bind(sock,(struct sockaddr*) &address,sizeof(address));
    assert(ret!=-1);
    //sock文件描述符 + sockaddr_in文件描述符强转 +地址的长度
    //将sock文件描述符与sock地址相绑定  

    ret=listen(sock,5);
    assert(ret!=-1);
    //监听哪个sock + 提示内核监听队列的最大长度(实际值为最大长度 + 1)
    //监听socket  

    sleep(20);
    struct sockaddr_in client;
    socklen_t client_addrlength=sizeof(client);
    int connfd=accept(sock,(struct sockaddr*)&client,&client_addrlength);
   //执行过listen系统调用的监听sock
   //(执行过listen调用,处于LISTEN状态的socket称为监听socket,
   //处于ESTABLISHED状态的socket称为连接socket)
   //要接收连接的socket + 接收连接的sockaddr的地址 + 地址长度 



    if(connfd<0)
    {
	printf("errno is:%d\n",errno);
    }
    else
    {
	//成功接收打印出客户端的IP地址和端口号
	char remote[INET_ADDRSTRLEN];
	printf("connect with ip:%s and port : %d\n",  \
	inet_ntop(AF_INET,&client.sin_addr,remote,INET_ADDRSTRLEN),ntohs(client.sin_port));
	close(connfd);
    }
    close(sock);
    return 0;
}
