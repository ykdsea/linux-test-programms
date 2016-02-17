#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void test_tmpfile(){
	int fd = open("./testunlink",O_RDWR|O_CREAT,0777);
	printf("create test file with fd %d\n",fd);
	system("ls -al");

	int ret = unlink("./testunlink");
	printf("unlink return %d \n",ret);
	sleep(1);
	

	int backfd = dup(fd);
	close(fd);

	printf("after unlink \n");
	system("ls -al");	
	sleep(5);
	printf("after sleep to ls\n");
	system("ls -al");

	printf("to touch testunlink\n");
	system("touch testunlink");
	system("ls -al");
	sleep(5);
}


int main(){
	test_tmpfile();
	return 1;
}
