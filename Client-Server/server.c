#include <stdbool.h>
#include <sys/wait.h>
#include "comm.h"

int32_t fd;
char* addr;

bool isFile(char* file){
   struct stat file_data;
   stat(file,&file_data);
	if(S_ISREG(file_data.st_mode) && file_data.st_size!=8*sizeof(uint32_t) ){
		errx(2,"The file is not in request format.");
	}
   return S_ISREG(file_data.st_mode);
}

void createFile(char* path){

   int32_t fd=open(path,O_CREAT|O_WRONLY,0666);
   if(fd==-1){
      errx(3,"Cannot create the file.");
   }
   uint32_t balance=0;
   for(uint32_t i=0;i<8;i++){
      if(write(fd,&balance,sizeof(uint32_t))<1){
         errx(4,"Canot write in %s.",path);
      }
   }
   close(fd);
}

uint32_t readBalance(int fd,char* input){
   
   char account=input[0];
	off_t offset;
   switch(account){
      case 'A': offset=0; break;
      case 'B': offset=1; break;
      case 'C': offset=2; break;
      case 'D': offset=3; break;
      case 'E': offset=4; break;
      case 'F': offset=5; break;
      case 'G': offset=6; break;
      case 'H': offset=7; break;
	default: offset=0;
      }
	lseek(fd,offset*sizeof(uint32_t),SEEK_SET);
	uint32_t balance;
	if(read(fd,&balance,sizeof(uint32_t))<1){
         	errx(4,"Cannot read from file.");		
	}
	return balance;
}

bool makeTransaction(int fd,char* input,int sum){
   char account=input[0];
	off_t offset;
   switch(account){
      case 'A': offset=0; break;
      case 'B': offset=1; break;
      case 'C': offset=2; break;
      case 'D': offset=3; break;
      case 'E': offset=4; break;
      case 'F': offset=5; break;
      case 'G': offset=6; break;
      case 'H': offset=7; break;
	default: offset=0;
      }
	lseek(fd,offset*sizeof(uint32_t),SEEK_SET);
	uint32_t balance;
	if(read(fd,&balance,sizeof(uint32_t))<1){
        	errx(4,"Cannot read from file.");
	}
	if( (int)(balance+sum) <0 || (balance+sum)>UINT32_MAX){
		return false;
	}
	balance+=sum;
	lseek(fd,-sizeof(uint32_t),SEEK_CUR);
	if(write(fd,&balance,sizeof(uint32_t))<1){
        	errx(4,"Canot write in file.");
	}
	return true;
}


bool checkAccount(char* input){
   
   if(strlen(input)!=1){
      return false;
   }
   char account=input[0];
   switch(account){
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
      case 'H': return true;
      default: return false;
      }
}

void handle_msg(char* msg,int fd,char* addr,int errn){
	
	close(fd);
	sem_unlink(semFirst);
	sem_unlink(semSecond);
	sem_unlink(semThird);
	munmap(addr,shared_memory_size);
	shm_unlink(shmName);
	errno=errn;
	err(3,"%s command failed to complete. ",msg);	

}

void handle_error(int sigNum){
	
	close(fd);
	sem_unlink(semFirst);
	sem_unlink(semSecond);
	sem_unlink(semThird);
	munmap(addr,shared_memory_size);
	shm_unlink(shmName);
	(void) sigNum;
	exit(EXIT_SUCCESS);	
}

int main(int argc, char* argv[]){
	signal(SIGINT,handle_error);

   if(argc!=2){
      errx(2,"Incorrect input.");
   }

   if(!isFile(argv[1])){
      createFile(argv[1]);
   }

   fd=open(argv[1],O_RDWR);
   if(fd==-1){
      err(2,"Could not open %s in read and write mode.",argv[1]);
   }
	sem_t* sem_idFirst=sem_open(semFirst,O_CREAT,0600,0);
	if(sem_idFirst==SEM_FAILED){
		int errn=errno;
		close(fd);
		errno=errn;
		err(2,"Cannot create first semaphore. ");
	}	
	sem_t* sem_idSecond=sem_open(semSecond,O_CREAT,0600,0);
	if(sem_idSecond==SEM_FAILED){
		int errn=errno;
		close(fd);
		sem_unlink(semFirst);
		errno=errn;
		err(2,"Cannot create second semaphore. ");
	}
	sem_t* sem_idThird=sem_open(semThird,O_CREAT,0600,0);
	if(sem_idThird==SEM_FAILED){
		int errn=errno;
		close(fd);
		sem_unlink(semFirst);
		sem_unlink(semSecond);
		errno=errn;
		err(2,"Cannot create third semaphore. ");
	}
	int fdShared=shm_open(shmName,O_CREAT|O_TRUNC|O_RDWR,0666);
	if(fdShared==-1){
		int errn=errno;
		close(fd);
		sem_unlink(semFirst);
		sem_unlink(semSecond);
		sem_unlink(semThird);
		errno=errn;
		err(2,"Cannot create shared memory. ");
	}

	if(ftruncate(fdShared, shared_memory_size)==-1){
		err(3,"Cannot resize the shared memory. ");
	}

	addr=mmap(NULL,shared_memory_size,PROT_READ|PROT_WRITE,MAP_SHARED,fdShared ,0);
	
	while(1){
		
		if(sem_post(sem_idFirst)==-1){
			handle_msg("Sem_post",fd,addr,errno);
		}

		if(sem_wait(sem_idSecond)==-1){
			handle_msg("Sem_wait",fd,addr,errno);
		}

		uint32_t length=strlen(addr);
		
		char* account=malloc(length);
		strcpy(account,addr);
		if(!checkAccount(account)){			
			strcpy(addr,"-1");
			goto NEXT;  
		} else {

			uint32_t balance=readBalance(fd,account);
			sprintf(addr,"%d ",balance);
		}
		if(sem_post(sem_idThird)==-1){
			free(account);
			handle_msg("Sem_post",fd,addr,errno);
		}

		if(sem_wait(sem_idSecond)==-1){
			free(account);
			handle_msg("Sem_wait",fd,addr,errno);
		}

		int sum=atoi(addr);
		if(sum<INT16_MIN || sum>INT16_MAX){
			strcpy(addr,"-1");
			goto NEXT;
		}
		if(!makeTransaction(fd,account,sum)){
			strcpy(addr,"-2");
		}	
			
		NEXT: ;
		free(account);
		if(sem_post(sem_idThird)==-1){
			handle_msg("Sem_post",fd,addr,errno);
		}
		if(sem_wait(sem_idSecond)==-1){
			handle_msg("Sem_wait",fd,addr,errno);
		}
		pid_t pid=atoi(addr);
		waitpid(pid,NULL,0);
	}
}
