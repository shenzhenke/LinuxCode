#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
static int stop=0;

//SIGTERM信号的处理函数,触发时结束主程序中的循环
static void handle_term( int sig)
{
    stop=1;
}

int main(int argc,char *argv[])
{
    signal(SIGTERM,handle_term);
    if(argc<3)
    {
	printf("usage:%s ip_address port_number backlog\n",basename(argv[0]));
	return 1;
    }
    const char* ip=argv[1];  	 //ip地址  
    int  port=atoi(argv[2]); 	//端口号
    int backlog=atoi(argv[3]);	//atoi字符串转整型
    //backlog参数提示内核监听队列的最大长度,如果监听队列的长度超过backlog,
    //服务器不再受理新的客户连接,并且客户端收到ECONNREFUSED错误信息
	
	//创建一个 IPv4 socket地址
    int sock=socket(PF_INET,SOCK_STREAM,0); //(int domain,it type,int protocol)
    //使用哪个底层协议族+服务类型+选择一个具体协议(一般默认为0)
    //IPv4:PF_INET      IPv6:PF_INET6
    //服务类型  SOCK_STREAM(流服务) TCP协议 + SOCK_DGRAM(数据报) UDP协议
    //socket系统调用成功返回sock文件描述符,失败返回-1并设置errno

    struct sockaddr_in address;

 // sockaddr_in代表TCPIP协议族专用的sock地址结构体
    bzero(&address,sizeof(address));

 //    struct sockaddr_in 
 //   {
 //       sa_family_t sin_family ;  //地址族
 //       u_int16_t sin_port;  	//端口号,要用网络字节序表示
 //       struct in_addr sin_addr; //ipv4地址结构体
 //   }
 //   struct in_addr
 //   {
 //       u_int32_t s_addr; //ipv4地址,要用网络字节序表示
 //   }
     
    address.sin_family=AF_INET;

    inet_pton(AF_INET,ip,&address.sin_addr);
    //inet_pton函数将用字符串表示的IP地址转换为用网络字节序表示整数表示的IP地址
    address.sin_port=htons(port);

	//绑定sock文件描述符和socket地址
    int ret=bind(sock,(struct sockaddr*)&address,sizeof(address));
    //bind(int sockfd, const struct sockaddr* my_addr,socklen_t addrlen);
    //sock文件描述符 + my_addr地址 + socket地址长度绑定
    assert(ret!=-1);

	//监听socket
    ret=listen(sock,backlog);
    //int listen(int sockfd,int backlog);
    //sockfd指定被监听的socket,成功返回0,出错返回-1并设置errno
    assert(ret!=-1);

    //循环等待,直到有SIGTERM信号将它中断
    while(!stop)
    {
	sleep(1);
    }

    close(sock);
    return 0;
}

