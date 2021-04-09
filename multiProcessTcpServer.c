#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
// signal과 wait함수관련 헤더 추가
#include <signal.h>
#include <sys/wait.h>


void errProc();
void errPrint();
void sig_exit(int sig) {//signal 함수에서 사용할 핸들러이다. 
	//signal()함수를 사용하여 자식 프로세스가 종료되었다는 SIGCHLD가 전달되면 함수 sig_exit을 호출하여 wait(0)을 통해 좀비가 된 프로세스가 종료되게한다.
	wait(0);
}
int main(int argc, char** argv)
{
	int srvSd, clntSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen, readLen, strLen;
	char rBuff[BUFSIZ];
	pid_t pid;
	signal(SIGCHLD, sig_exit);//signal()함수 사용 이렇게 사용하는 이유는 wait()함수는 블럭 함수이기 때문에 부모 프로세스가 진행되지 못하는 상황이 발생할 수 있기때문에 이렇게 signal()함수를 사용하여 적절하게 wait()함수가 불러지도록 한다.

	if(argc != 2) {
		printf("Usage: %s [port] \n", argv[0]);
		exit(1);
	}
	printf("Server start...\n");

	srvSd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(srvSd == -1 ) errProc();

	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(atoi(argv[1]));

	if(bind(srvSd, (struct sockaddr *) &srvAddr, sizeof(srvAddr)) == -1)
		errProc();
	if(listen(srvSd, 5) < 0)
		errProc();
	clntAddrLen = sizeof(clntAddr);
	while(1)
	{
		clntSd = accept(srvSd, (struct sockaddr *)
					 &clntAddr, &clntAddrLen);
		if(clntSd == -1) {
			errPrint();
			continue;	
		}
		printf("client %s:%d is connected...\n",
			inet_ntoa(clntAddr.sin_addr),
			ntohs(clntAddr.sin_port));	
		pid = fork();
		if(pid == 0) { /* child process */
			close(srvSd);
			while(1) {
				readLen = read(clntSd, rBuff, sizeof(rBuff)-1);
				if(readLen == 0) break;
				rBuff[readLen] = '\0';
				printf("Client(%d): %s\n",
					ntohs(clntAddr.sin_port),rBuff);
				write(clntSd, rBuff, strlen(rBuff));
			}
			printf("Client(%d): is disconnected\n",
				ntohs(clntAddr.sin_port));
			close(clntSd);
			return 0;
		}	
		else if(pid == -1) errProc("fork");
		else { /*Parent Process*/
			close(clntSd);
		}	
	}
	close(srvSd);
	return 0;
}

void errProc(const char *str)
{
    fprintf(stderr,"%s: %s \n",str, strerror(errno));
    exit(1);
}

void errPrint(const char *str)
{
	fprintf(stderr,"%s: %s \n",str, strerror(errno));
}
