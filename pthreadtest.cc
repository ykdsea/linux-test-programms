#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int retval = 1;


void* sem_test_routine(void* args){
	printf("sem-test thread startup\n");
	sem_t* semptr = (sem_t*)args;
        sem_wait(semptr);
	printf("child thread wait sem ok\n");
	sleep(2);
	sem_post(semptr);
	printf("child thread release sem\n");	
	retval = 999;
	pthread_exit((void*)&retval);
	printf("sem-test thread end\n");
	return NULL;
}



void test_sem(){
	printf("start to test thread semphore\n");
	sem_t testsem;
	sem_init(&testsem,0,1);
	pthread_t childthread;
	pthread_create(&childthread,NULL,sem_test_routine,(void*)&testsem);
	
	sleep(2);
	sem_wait(&testsem);
	printf("main thread get sem \n");
	sleep(3);
	printf("main thread destory sem\n");
	sem_destroy(&testsem);

	
	int* value = 0;
	pthread_join(childthread,(void**)&value);
	printf("get return value from child thread %d\n",*value);

	//try to join a already finished thread if ok		

}



int main(){
	test_sem();
	return 1;
}

