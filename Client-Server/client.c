#include "comm.h"

void handle_msg(char* msg,char* addr,sem_t* sem_idFirst,sem_t* sem_idSecond,sem_t* sem_idThird,int errn){
	
	sem_close(sem_idFirst);
	sem_close(sem_idSecond);
	sem_close(sem_idThird);
	munmap(addr,shared_memory_size);
	errno=errn;
	err(3,"%s command failed to complete.",msg);	
}


int main(int argc, char* argv[]){
	
   if(argc!=2){
      errx(2,"Incorrect input.");
   }

	sem_t* sem_idFirst=sem_open(semFirst,O_EXCL);
	if(sem_idFirst==SEM_FAILED){
		errx(2,"Server is not connect.");
	}
	sem_t* sem_idSecond=sem_open(semSecond,O_EXCL);
	if(sem_idSecond==SEM_FAILED){
		int errn=errno;
		sem_close(sem_idFirst);
		errno=errn;
		err(2,"Cannot connect to the second semaphore.");
	}
	
	sem_t* sem_idThird=sem_open(semThird,O_EXCL);
	if(sem_idThird==SEM_FAILED){
		int errn=errno;
		sem_close(sem_idFirst);
		sem_close(sem_idSecond);
		errno=errn;
		err(2,"Cannot connect to the third semaphore.");
	}

	int fd=shm_open(shmName,O_RDWR,0644);
	if(fd==-1){
		int errn=errno;
		sem_close(sem_idFirst);
		sem_close(sem_idSecond);
		sem_close(sem_idThird);
		errno=errn;
		err(2,"Cannot connect to the shared memory.");
	}
	
	char* addr=mmap(NULL,shared_memory_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	
	if(sem_wait(sem_idFirst)==-1){
		handle_msg("Sem_wait", addr, sem_idFirst, sem_idSecond, sem_idThird,errno);
	}

	strcpy(addr,argv[1]);
	
	if(sem_post(sem_idSecond)==-1){
		handle_msg("Sem_post", addr, sem_idFirst, sem_idSecond, sem_idThird,errno);
	} 

	if(sem_wait(sem_idThird)==-1){
		handle_msg("Sem_wait", addr, sem_idFirst, sem_idSecond, sem_idThird,errno);
	}

	if(!strcmp(addr,"-1")){

		printf("The account does not exist.\n");
		goto NEXT ;

	}

	printf("%s\n",addr);	
	if(scanf("%s",addr)==-1){
		handle_msg("Cannot read from stdin. Scanf", addr, sem_idFirst, sem_idSecond, sem_idThird,errno);
	}
	
	if(sem_post(sem_idSecond)==-1){
		handle_msg("Sem_post", addr, sem_idFirst, sem_idSecond, sem_idThird,errno);
	}
	if(sem_wait(sem_idThird)==-1){
		handle_msg("Sem_wait", addr, sem_idFirst, sem_idSecond, sem_idThird,errno);
	}
	
	if(!strcmp(addr,"-1")){

		printf("The number is not in range of int32_t.\n");
	} else if(!strcmp(addr,"-2")){

		printf("Transaction cannot be done.\n");
	}
	
	NEXT: ;

	sprintf(addr,"%d",getpid());
	if(sem_post(sem_idSecond)==-1){
		handle_msg("Sem_post", addr, sem_idFirst, sem_idSecond, sem_idThird,errno);
	}
	sem_close(sem_idFirst);
	sem_close(sem_idSecond);
	sem_close(sem_idThird);
	munmap(addr,shared_memory_size);
   exit(0);
}
