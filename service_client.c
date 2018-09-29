#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>


#pragma pack(1)
typedef struct
{
        char name[51];
        unsigned int len;
}f_info;

#pragma pack()


int main(int argc, char** argv)
{
	if(argc != 3)
	{
		perror("参数错误\n");
		exit(1);
	}

	int sock_listen;

	sock_listen = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;//指定IP地址为本机任意IP
	//myaddr.sin_addr.sin_addr = inet_addr("ip地址");将字符串形式的IP地址转换为一个无符号32位整型（网络字节序）
	myaddr.sin_port = htons(atoi(argv[1]));//指定端口号
	//htons函数：将主机字节序表示的short类型数据转换为网络字节序表示
	if(bind(sock_listen, (struct sockaddr*)&myaddr, sizeof(myaddr)))//绑定套接字地址
	{
		perror("bind");
		exit(1);
	}

	listen(sock_listen, 5);//指定套接字设置为监听状态，参数2：连接等待队列的长度

	//将套接字的SO_REUSEADDR属性设置为1，表示允许地址复用
	int optval = 1;
	setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));//设置套接字属性

	int sock_conn;//连接套接字,用于和相应的客户端通信
	struct sockaddr_in clnaddr;
	socklen_t len;

	signal(SIGCHLD, SIG_IGN);

	while(1)
	{
		len = sizeof(clnaddr);
		sock_conn = accept(sock_listen, (struct sockaddr*)&clnaddr, &len);
		//接受一个客户端连接请求，没有的话accept函数会堵塞程序的运行,直到成功接受到一个连接或出错

		if(sock_conn != -1)
		{
			pid_t pid;

			pid = fork();
			if(pid == -1)
			{
				perror("fork");
				close(sock_conn);
				continue;
			}
			if(pid > 0) continue;

			printf("ip:%s host:%d上线\n", inet_ntoa(clnaddr.sin_addr), ntohs(clnaddr.sin_port));

			int p, size = 0;
			char* c;
			char buff[1024] = "";
			struct stat st;
			f_info f;

			if(stat(argv[2], &st))
			{
				perror("stat");
				exit(1);
			}

			f.len = st.st_size;
			strcpy(f.name, (c = strrchr(argv[2], '/'))? c + 1: argv[2]);

			send(sock_conn, &f, sizeof(f), 0);

			p = open(argv[2], O_RDONLY);

			while((size = read(p, buff, sizeof(buff))) != -1)
			{
				if(send(sock_conn, buff, size, 0) == -1) break;
			}

			printf("ip:%s host:%d下线\n", inet_ntoa(clnaddr.sin_addr), ntohs(clnaddr.sin_port));
			close(p);
			close(sock_conn);

			if(pid == 0) exit(0);
		}
		else
		{
			fprintf(stderr, "accept error:%s\n", strerror(errno));
		}
	}

	close(sock_listen);

	return 0;
}

