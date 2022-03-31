## main.c

连接ftp服务器，并用anonymous用户名登录，密码设置为123

```c
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

	printf("请输入密码：");                              //向服务器发送密码
	scanf("%s",pass);
	sprintf(buf,"PASS %s\n",pass);
	send(client,buf,strlen(buf),0);

	_recive(client,buf,buf_size,ret,230);            //验证
```

连接失败界面：

![image-20220331131128052](https://raw.githubusercontent.com/moli531/img/main/202203311311382.png)

登陆成功界面：

![image-20220331131008532](https://raw.githubusercontent.com/moli531/img/main/202203311310865.png)

登陆失败界面：

![image-20220331131224129](https://raw.githubusercontent.com/moli531/img/main/202203311312588.png)

quit命令

```c
int _QUIT(int client,char* buf,size_t buf_size,size_t ret)
{
	_send(client,buf,buf_size,"QUIT\n",5);

	_recive(client,buf,buf_size,ret,221);
}
```

调用quit函数退出：

![image-20220331131437741](https://raw.githubusercontent.com/moli531/img/main/202203311314272.png)

ls或dir命令，为数据连接创建新的listen套接字，并以PORT命令通知server端其IP地址和监听端口号

```c
int _PORT(int client,char* buf,size_t buf_size,size_t ret,\
			struct sockaddr_in server_addr,size_t addrlen)
{
    SOCKET data_sock;
	data_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);   
	unsigned char p1,p2;
	int server_port = p1*256+p2;
	if(0 > data_sock)
	{
		printf("socket error!");
		return 0;
	}

    _send(client,buf,buf_size,"PORT 127,0,0,1,21,20\r\n",30) //向服务器发送PORT指令

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
int _LS(int client,char* buf,size_t buf_size,size_t ret,\
			struct sockaddr_in server_addr,size_t addrlen)
{
	int data_sock = _PORT(client,buf,buf_size,ret,server_addr,addrlen);    

	_send(client,buf,buf_size,"LIST\n",9);                    //向服务器发送LIST指令
	char buf1[1000000] = {};
	recv(data_sock,buf1,sizeof(buf1),0);
	printf("%s\n",buf1);
	close(data_sock);

	memset(buf,0,buf_size);
	_recive(client,buf,buf_size,ret,150);
    memset(buf,0,buf_size);
	_recive(client,buf,buf_size,ret,226);
}
```



但是PORT成功后会报425 Can't open data connection for transfer of "/"的错误

![image-20220331131800353](https://raw.githubusercontent.com/moli531/img/main/202203311318638.png)

FileZilla界面：

![image-20220331132244989](https://raw.githubusercontent.com/moli531/img/main/202203311322580.png)



