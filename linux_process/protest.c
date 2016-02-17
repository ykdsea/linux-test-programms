#include <stdlib.h>
#include <stdio.h>

void test_system(){
	system("ps aux");
}

void test_exec(){
	char *ps_argv[3]={
		"ps",
		"aux",
		0		
        };
	execve("/bin/ps",ps_argv,0);
}

int main(){
	

	//TODO: to try getopt,getoptlong

//	test_system();
	test_exec();
	printf("in main");
	return 1;

}
