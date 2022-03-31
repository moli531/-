#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <Ws2tcpip.h>


//向服务器端发送指定序列
int _send(int client,char* buf,size_t buf_size,\
			char* ORDER,size_t ORDER_size)
{
	sprintf(buf,"%s",ORDER);		//将要发送给服务器的指令写入buf缓冲区
	send(client,buf,strlen(buf),0);	//将buf内的指令发送给服务器
}

//接受服务器端反馈信息
int _recive(int client, char* buf, size_t buf_size,size_t ret,size_t _ret)
{
	recv(client,buf,buf_size,0);		//从服务器端接受，接受内存容存入buf
	printf("recv:%s\n",buf);
	sscanf(buf,"%d",&ret);				//buf内的内容写入变量ret
	if(ret != _ret)
	{
		printf("操作失败!请检查命令并重试!\n");
		return 0;
	}
}


//建立数据传输通道
int _PORT(int client,char* buf,size_t buf_size,size_t ret,\
			struct sockaddr_in server_addr,size_t addrlen)
{
    SOCKET data_sock;
	data_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);                    //建立数据传输专用socket
	unsigned char p1,p2;
	int server_port = p1*256+p2;
	if(0 > data_sock)
	{
		printf("socket error!");
		return 0;
	}

    _send(client,buf,buf_size,"PORT 127,0,0,1,21,20\r\n",30);		//向服务器发送PORT指令并接收返回值

	_recive(client,buf,buf_size,ret,227);

    sprintf(buf,"PORT 127,0,0,1,%d,%d\r\n", p1, p2);
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    listen(server_port,64);

    int ret2;
    ret2 = connect(data_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret2 < 0)
    {
        printf("Connect error!\n");
        closesocket(data_sock);
        return 0;
    }
    printf("Succeed connecting!\n");


	return data_sock;
}

//查看当前目录详情列表
int _LS(int client,char* buf,size_t buf_size,size_t ret,\
			struct sockaddr_in server_addr,size_t addrlen)
{
	int data_sock = _PORT(client,buf,buf_size,ret,server_addr,addrlen);    //建立数据传输通道

	_send(client,buf,buf_size,"LIST\n",9);                    //向服务器发送指令
	char buf1[1000000] = {};
	recv(data_sock,buf1,sizeof(buf1),0);
	printf("%s\n",buf1);
	close(data_sock);

	memset(buf,0,buf_size);
	_recive(client,buf,buf_size,ret,150);
    memset(buf,0,buf_size);
	_recive(client,buf,buf_size,ret,226);
}



//结束ftp进程
int _QUIT(int client,char* buf,size_t buf_size,size_t ret)
{
	_send(client,buf,buf_size,"QUIT\n",5);

	_recive(client,buf,buf_size,ret,221);
}

//主函数
int main(int argc,const char* argv[])
{
    int ret1;
    WSADATA wsaData;
    struct sockaddr_in server_addr;
    SOCKET client;
    WSAStartup(0x202, &wsaData);

	client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(0 > client)
	{
		printf("socket error!");
		return 0;
	}
	char InputIP[20];
    printf("ftp> open ");
    scanf("%s",&InputIP);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(21);
	server_addr.sin_addr.S_un.S_addr = inet_addr(InputIP);            //输入你的目标ip

    ret1 = connect(client, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret1 < 0)
    {
        printf("Connect error!\n");
        closesocket(client);
        return 0;
    }
    printf("Succeed connecting!\n");

	char buf[4096] = {};
	size_t buf_size = sizeof(buf),ret = 0;

	_recive(client,buf,buf_size,ret,220);
	printf("连接服务器成功\n");

	char name[20] = {};
	printf("请输入用户名：");                            //向服务器端发送用户名
	scanf("%s",name);
	sprintf(buf,"USER %s\n",name);
	send(client,buf,strlen(buf),0);

	_recive(client,buf,buf_size,ret,331);             //验证用户名是否正确

	char pass[20] = {};
	printf("请输入密码：");                              //向服务器发送密码
	scanf("%s",pass);
	sprintf(buf,"PASS %s\n",pass);
	send(client,buf,strlen(buf),0);

	_recive(client,buf,buf_size,ret,230);            //验证



	char arr[20] = {};

	for(;;)
	{
		printf("ftp>");
		scanf("%s",arr);
		if(0 == strcmp(arr,"ls")||0 == strcmp(arr,"dir"))
			_LS(client,buf,buf_size,ret,server_addr,sizeof(server_addr));     //ls命令
		if(0 == strcmp(arr,"quit"))
		{
			_QUIT(client,buf,buf_size,ret);                    //bye命令
			return 0;
		}
	}
}
