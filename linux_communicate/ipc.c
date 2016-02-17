#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>


union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
           };


#define KEY_PROC_ID 11

int systemv_sem_test(){
 	
	
	key_t key = ftok(".",KEY_PROC_ID);
        int semid = semget(key,1, IPC_CREAT);
	//first get a P op
	union semun semval;
	semval.val = 2;

	int err = semctl(semid,0,SETVAL,semval);	

	printf("sem V two return err %d\n",err);

	int pid = fork();

	if(pid == 0){
		key_t key =  ftok(".",KEY_PROC_ID);
		int semid = semget(key, 1, 0);
		printf("client get semid %d, to V 1\n",semid);
		
		struct sembuf opbuf;
		opbuf.sem_num = 0;
		opbuf.sem_op = -1;
		opbuf.sem_flg = SEM_UNDO;
		semop(semid,&opbuf,1);

		printf("client sem V 1 end!\n");
		semop(semid,&opbuf,1);
                printf("client sem V 2 end\n");
		exit(0);
	}else{
		key_t key = ftok(".",KEY_PROC_ID);
		int semid = semget(key,1, 0);
		printf("server get semid %d, op a sem wait \n",semid);
		

		struct sembuf opbuf;
		opbuf.sem_num = 0;
		opbuf.sem_op = 0;
		opbuf.sem_flg = SEM_UNDO;
	
		semop(semid,&opbuf,1);
		printf("server sem wait 1 end!\n");
//		semop(semid,&opbuf,1);
		printf("server sem wait 2 ok \n");
	
		
		return 1;
	}


}



int systemv_shm_test(){
	

	int pid =  fork();
	if(pid == 0){
	
	}else{
	
	}

}


int main(){
#if 0
	printf("Test system v semphor\n");
	systemv_sem_test();
	printf("Test end");
#endif
#if 1
	printf("Test shared mem \m");
	systemv_shm_test();
	printf("Test end");
#endif

	return 1;
}
