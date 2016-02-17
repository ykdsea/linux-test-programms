#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>

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



bool condFlag = false;
pthread_mutex_t* gMutexptr;

void *cond_test_routine(void* arg){
	printf("CHILDTHREAD:start test routine\n");	
	pthread_cond_t* condptr = (pthread_cond_t*)arg;
	pthread_mutex_t* mutexptr = gMutexptr;
	sleep(1);
	printf("CHILDTHREAD: to wait mutex\n");
	pthread_mutex_lock(mutexptr);
	
	sleep(2);
	condFlag = true;

	pthread_mutex_unlock(mutexptr);
	printf("CHILDTHREAD: to signal conditon\n");
	pthread_cond_signal(condptr);
	

	printf("CHILETHREAD:end test routine\n");
	return NULL;
}

void test_condition(){
	printf("MAINTHREAD:start test\n");
	static pthread_cond_t testcond;
	pthread_cond_init(&testcond,NULL);
	

        static pthread_mutex_t testmutex;
	pthread_mutex_init(&testmutex,NULL);
	gMutexptr = &testmutex;	

	pthread_t childthread;
	pthread_create(&childthread,NULL,cond_test_routine,(void*)&testcond);	


	pthread_mutex_lock(&testmutex);
	while(condFlag==false){
		printf("MAINTHREAD: to wait conditon\n");
		pthread_cond_wait(&testcond,&testmutex);
	}
	sleep(1);
	printf("MAINTHREAD: wait flag %d, unlock mutex\n",condFlag);
	pthread_mutex_unlock(&testmutex);

	pthread_cond_destroy(&testcond);
	pthread_mutex_destroy(&testmutex);
	printf("MAINTHREAD:test condtion end\n");
}




void test_process_mutex(){
        pthread_mutex_t* mutexptr = (pthread_mutex_t*)mmap(NULL,sizeof(pthread_mutex_t),PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);
	if(mutexptr == NULL){
		printf("MMAP FIAL, ERROR TO RETURN\n");
        	return;
	}


	pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr,1);
	pthread_mutex_init(mutexptr,&mutexattr);
	int count = 0;	

	if(true){//mutex pass with fork
		int cpid = fork();
		if(cpid == 0){
			printf("CHILDPROCESS: to lock mutex\n");
			pthread_mutex_lock(mutexptr);
			sleep(5);
			printf("CHILDPROCESS: to unlock mutex\n");
			pthread_mutex_unlock(mutexptr);
			printf("CHILDPROCESS: test mutex end\n");
			munmap(mutexptr,sizeof(pthread_mutex_t));
			return;
		}else{
			sleep(2);
			printf("MAINPROCESS: to wait lock\n");
			pthread_mutex_lock(mutexptr);
			printf("MAINPROCESS: wait lock ok\n");
			sleep(2);
			printf("MAINPROCESS: unlock and exit\n");
			pthread_mutex_unlock(mutexptr);	
		}
			
	}



	
	pthread_mutexattr_destroy(&mutexattr);
        pthread_mutex_destroy(mutexptr);
	munmap(mutexptr,sizeof(pthread_mutex_t));
}


int main(){
	//test_sem();
	//test_condition();
	test_process_mutex();

	return 1;
}

