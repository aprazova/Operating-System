#ifndef HEADER_FILE
#define HEADER_FILE

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>
#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h> 
#include <errno.h> 

const char* semFirst="First";
const char* semSecond="Second";
const char* semThird="Third";

const char* shmName="/Shared_memory";
const unsigned int shared_memory_size=32;

#endif
              

