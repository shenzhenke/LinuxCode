#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //用于点分十进制字符串IP地址转换为网络字节序
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>  //指定两个宏的大小
#include <string.h>
int main(int argc,char *argv[])
{
    if(argc<=2)
    {
	printf("usage: %s ip_address port_number\n",basename(argv[0]));
	return 1;
    }
    const char *ip=argv[1];
    int port=atoi(argv[2]);

    struct sockaddr_in sever_address;
    bzero(&sever_address,sizeof(sever_address));

    sever_address.sin_family=AF_INET; //地址族
    inet_pton(AF_INET,ip,&sever_address.sin_addr);  //ip地址到网络字节序的变化
    sever_address.sin_port=htons(port);   //端口主机字节序到网络字节序

    int sockfd=socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd>=0);

    if(connect(sockfd,(struct sockaddr*)& sever_address,sizeof(sever_address))<0)
    {
	printf("connection failed\n");
    }
    else
    {
	const char* oob_data="abc";
	const char* normal_data="123";
	send(sockfd,normal_data,strlen(normal_data),0);
	send(sockfd,oob_data,strlen(oob_data),MSG_OOB);
	send(sockfd,normal_data,strlen(normal_data),0);
    }
    close(sockfd);
    return 0;
}
