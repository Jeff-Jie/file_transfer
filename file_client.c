#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
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
	if(argc != 2)
	{
		fprintf(stderr, "参数错误\n");
		exit(1);
	}

	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in myaddr;

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(1111);

/*	if(bind(sock, (struct sockaddr*)&myaddr, sizeof(myaddr)) == -1)
	{
		perror("bind");
		//exit(1);
	}*/

	struct sockaddr_in service_addr;
	service_addr.sin_family = AF_INET;
	service_addr.sin_addr.s_addr = /*inet_addr("118.126.115.151")*/INADDR_ANY;
	service_addr.sin_port = htons(atoi(argv[1]))/*htons(24324)*/;

	if(connect(sock, (struct sockaddr*)&service_addr, sizeof(service_addr)) == 0)
	{
		f_info f;
		int new_f, s = 0, r;
		char buff[1024];
		recv(sock, &f, sizeof(f), 0);

		new_f = open(f.name, O_WRONLY | O_CREAT, 0666);
		printf("%d\n", s = f.len);

		while(1)
		{
			r = recv(sock, buff, sizeof(buff), 0);
			write(new_f, buff, r);
			if(!(s -= r)) break;
		}

		close(new_f);
	}
	else perror("connect");

	close(sock);

	return 0;
}

