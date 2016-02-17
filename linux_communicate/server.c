/*
sky.zhou

Test:
1,pipe使用，父进程和子进程
2,epoll 测试，父进程通过epool 检测 fd。
3，named pipe，通过epool 检测。





*/

#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>



//pipe fd
int mPipeFds[2];
int mPipeFds2[2];
int contentLen = 20;



struct poll_user{
	int fd;
	int test;
	void (*handler)(char*)  ;
};

void dumpContent(char *content){
	printf("Content in event is %s \n",content);
}


int readEpoll(){

	

}


//create epoll
int mEpollFd = -1;
int testEpollWithPipe(){
	mEpollFd = epoll_create(16);
	
	pipe(mPipeFds);
	pipe(mPipeFds2);
	

	
	int pid = fork();
	if(pid == -1)
		goto TEST_1_EXIT;

	if(pid == 0){
		printf("child process say hello\n");
		char clientContent[20];
		memset(clientContent,0,20);
		sprintf(clientContent,"client hello\n");
		
		write(mPipeFds[1],clientContent,strlen(clientContent));

		printf("child process to receive from server\n");
		

		printf("read content ");

		char readContent[20];
		memset(readContent,0,20);
		int readlen = -1;
		while(readlen <= 0){
			readlen = read(mPipeFds2[0],readContent,(size_t)20);
			if(readlen > 0)
				printf("client read message %s \n",readContent);
			else if(readlen == 0)
				printf("server closed!!");
			sleep(1);
		}

		return 1;
	} else {
		struct poll_user userdata;
		userdata.fd = mPipeFds[0];
		userdata.test = 1111;
		userdata.handler = &dumpContent;

		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.ptr = (void*)(&userdata);
		epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mPipeFds[0],&event);
		
		struct epoll_event eventItems[5];

		int waititems = epoll_wait(mEpollFd,eventItems,5,-1);
		if(waititems > 0){
			int i = 0;			
			for( i = 0;i < waititems;i++){
				if(eventItems[i].events == EPOLLIN){
					char content[20];
					memset(content,0,20);
					struct poll_user *data = (struct poll_user *)eventItems[i].data.ptr;
					int readlen = read(data->fd,content,20);
					printf("server get event %d ,test data %d \n",i,data->test);
					data->handler(content);
				} else {
					printf("server get events not EPOLLIN");				
				}
			}

			char wContent[20];
			memset(wContent,0,20);
			sprintf(wContent,"server say hi\n");
			write(mPipeFds2[1],wContent,strlen(wContent));
		}
		close(mPipeFds2[1]);
		printf("closed mPipeFds2[1] = %d\n",mPipeFds2[1]);
	}
	



TEST_1_EXIT:
	close(mEpollFd);
	mEpollFd = -1;
	close(mPipeFds[0]);
	close(mPipeFds[1]);
	close(mPipeFds2[0]);
	close(mPipeFds2[1]);
}


#define FIFO_FILE_PATH "FIFO_TEST"
int testNamedPipe(){
	unlink(FIFO_FILE_PATH);
	if(-1 == mkfifo(FIFO_FILE_PATH,0666))
		return -1;
	

	int pid = fork();
	if(pid==0){
		printf("child process to read\n");
		int childfd = open(FIFO_FILE_PATH,O_RDONLY);
		printf("client open fifo return fd %d \n",childfd);
		char content[20];
		memset(content,0,20);
		read(childfd,content,20);
		printf("client read content %s \n",content);

		if(childfd >=0 )
			close(childfd);

		return 1;
	}else{
		printf("parent process to write\n");
		int parentfd = open(FIFO_FILE_PATH,O_WRONLY);
		printf("server open fifo return fd %d \n",parentfd);
		write(parentfd,"Say hello",10);
		printf("write content to client\n");
		if(parentfd >=0 )
			close(parentfd);
		return 0;
	}
	
	
}

void testPopen(){
	FILE* file = popen("ps","r");
	char content[256];
	memset(content,0,256);
	fread(content,256,1,file);
	printf("get content from ps \n");
	printf("%s\n",content);

	pclose(file);

}


int testSocketPair(){
	int fds[2];
	socketpair(AF_UNIX,SOCK_SEQPACKET, 0,fds);
	int serverfd = fds[0];
	int clientfd = fds[1];

	int pid = fork();
	if(pid == 0){
		char content[20];
		memset(content,35,10);
		write(clientfd,content,10);
		sleep(1);

		memset(content,0,20);
		read(clientfd,content,10);
		printf("client read content %s \n",content);

		close(serverfd);
		close(clientfd);
			
	}else{
		char content[20];
		memset(content,0,20);
		sleep(1);
		read(serverfd,content,10);
		printf("server read content %s \n",content);
		
		memset(content,33,20);
		write(serverfd,content,10);
		sleep(1);

		close(serverfd);
		close(clientfd);
	}

	return 1;
}
#define SOCKET_NAME  "socket_server"

int startSocketClient(){
	sleep(1);
	int client_fd = socket(AF_UNIX,SOCK_SEQPACKET,0);
	struct sockaddr_un server_addr;
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path,SOCKET_NAME);

	connect(client_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
	
	char content[20];
	memset(content,0,20);
	recv(client_fd,content,20,0);

	char content2[20];
	memset(content2,0,20);
	recv(client_fd,content2,10,0);

	printf("read content %s\n",content);
	printf("read content2 %s\n",content2);
	close(client_fd);
	
	printf("client end \n");
	return 1;
}



int startSocketServer(){
	
	int server_fd = socket(AF_UNIX,SOCK_SEQPACKET,0);
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path,SOCKET_NAME);
	int addr_len = sizeof(server_addr);
	
	bind(server_fd,(struct sockaddr*)&server_addr,addr_len);

	listen(server_fd,5);

	addr_len = sizeof(client_addr);
	int writefd = accept(server_fd,(struct sockaddr*)&client_addr,&addr_len);
	printf("server accept %d \n",writefd);
	
	char writecontent1[10];
	memset(writecontent1,33,10);
	char writecontent2[5];
	memset(writecontent2,35,5);

	send(writefd,writecontent1,10,0);
	send(writefd,writecontent2,5,0);
	
	sleep(3);
	close(writefd);
	printf("server end \n");

	close(server_fd);

	unlink(SOCKET_NAME);
	return 1;
}

int testSocket(int type){
	if(type == 1)
		return testSocketPair();
	
	int pid = fork();
	if(pid == 0) {
		startSocketClient();
	} else {
		startSocketServer();
	}
}


void signalhandler(int signum){
	printf("get signal %d \n",signum);
}


int testOneTimeSnapShot(){
	(void) signal(SIGINT,signalhandler);
	do{
		sleep(1);
	}while(1);
	
	return 1;
}

int testAlarmAndKill(){
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = signalhandler;
	sa.sa_flags=0;

	sigaction(SIGALRM,&sa,0);
	sigaction(SIGINT,&sa,0);	
	printf("start test alarm\n");	
	alarm(2);
	alarm(1);
	
	pause();
	printf("pause end\n");
	pause();
	printf("pause 2 end\n");
	

	return 1;
}


int testSignal(int type){
	if(type == 1)
		return testOneTimeSnapShot();

	if(type == 2)
		return testAlarmAndKill();

}


int main(){
	
	#if 0
	printf("Test  popen start**************\n");
	testPopen();
	printf("Test popen end **************\n");
	#endif

	#if 0
	printf("Test epoll with pipe start **************\n");
	testEpollWithPipe();
	printf("Test epoll with pipe end **************\n");
	#endif

	#if 0
	printf("Test named pipe start **************\n");
	testNamedPipe();
	printf("Test named pipe end **************\n");
	#endif

	#if 0
	printf("Test socket pair start **************\n");
	testSocket(1);
	printf("Test socket pair end **************\n");
	#endif

	#if 0
	printf("Test socket start **************\n");
	testSocket(2);
	printf("Test socket end **************\n");
	#endif

	#if 1
	printf("Test signal start **************\n");
	//testSignal(1);
	testSignal(2);
	printf("Test signal end **************\n");
	#endif


	return 1;
}
