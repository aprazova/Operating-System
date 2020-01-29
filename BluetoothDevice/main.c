#include "declarations.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <err.h>
#include <fcntl.h>
#include <string.h>
#include <regex.h>

struct segment{
	char* name;
	uint32_t segNumber;
	uint32_t positionNumber;
	char* values;
}__attribute__((packed));

int32_t segmentExist(char* input){

	for(uint32_t i=0; i<countParameter;i++){
		if(!strcmp(input,parametersName[i])){
			return i;
		}
	}
	return -1;
}

struct segment setSegment(int32_t index){
	struct segment result;
	result.name=parametersName[index];
	result.segNumber=segmentsNumber[index];
	result.positionNumber=positionsInSegment[index];
	result.values=validValues[index];
	return result;
}

void readHelp(){

	int32_t fd=open("helpFile.txt",O_RDONLY);
   if(fd==-1){
      errx(5,"Can not open helpFile.txt in read mode.");
   }

   char buff[4096];
   int32_t read_size;
   while((read_size=read(fd,&buff,4069))>0){
      write(1,&buff,read_size);
   }
   
   close(fd);
}


bool isLittleEndian()
{
	uint32_t num = 0x01234567;
	return *((uint8_t *)&num) == 0x67;
}

int32_t checkFirstBit(int32_t index){

	uint32_t segmentN=segmentsNumber[index];
	int32_t lineIndex=-1;
	for(uint32_t i=0; i<lineCount;i++){
		if(segmentN==lineId[i]){
			lineIndex=i;
		}
	}
	if(lineIndex==-1){
		errx(4,"Does not exist segment with this parameter.");
	}
	char* lineT=lineType[lineIndex];
	int32_t bit=-1;
	for(unsigned int i=0; i<typesCount;i++){
		if(!strcmp(lineT,segmentTypes[i])){
			bit=segmentTypesId[i];
		}		
	}
	return bit;
}

void setBit(int32_t fd, uint32_t position,uint32_t numberOfSegment){
	off_t offset=sizeOfSegment*numberOfSegment+1+position/bitsInByte;
	lseek(fd,offset,SEEK_SET);
	char buff;
	read(fd,&buff,sizeof(char));
	buff |= 1<<(bitsInByte-1-position);
	lseek(fd,-1,SEEK_CUR);
	write(fd,&buff,sizeof(char));
}

void writeParameterName(int32_t fd,int32_t segmentIdentificater,char* name){

	if(!strcmp(segmentTypes[segmentIdentificater],"text")){
      size_t len=strlen(name);
		if((write(fd,name,len))!=(int)len){
		   errx(5,"Error with write in file.");
	   }
		
      char buff=0;
		for(uint32_t i=len;i<sizeOfPiece[segmentIdentificater];i++)
		{	
			write(fd,&buff,sizeof(char));
	 	}

	} else if (!strcmp(segmentTypes[segmentIdentificater],"numeric")){
		int32_t number=atoi(name);
		if(write(fd,&number,sizeof(int32_t))!=sizeof(int32_t)){
			 errx(5,"Error with write in file.");
      }

	}else {
		size_t len=strlen(name);
		if((write(fd,name,len))!=(int)len){
   	   errx(5,"Error with write in file.");
	   }
	}
}

bool checkBit(int32_t fd,uint32_t segmentNumber, uint32_t position){
	
	lseek(fd,segmentNumber*sizeOfSegment+1+position/bitsInByte,SEEK_SET);
	char buff;
	read(fd,&buff,sizeof(char));
	uint32_t result=((buff>>(bitsInByte-1-position)) & 1);
	if(result==1) return true;
	else return false;
}

void readParameterName(int32_t fd,char* typeOfSegment,uint32_t sizeOfParameter){

	if(!strcmp(typeOfSegment,"text")){
		char buff[sizeOfParameter];
      ssize_t read_size=read(fd,&buff,sizeOfParameter);
		
      if (read_size==0){
		   errx(5,"Can not read the segment.");
	   }	
      write(1,&buff,sizeOfParameter);
   } else if (!strcmp(typeOfSegment,"numeric")){
	   int32_t buff;
      ssize_t read_size=read(fd,&buff,sizeOfParameter);
      if(read_size==0){
		   errx(5,"Can not read the segment.");	
      }
      printf("%d",buff);
   
   } else {
   char buff[sizeOfParameter];
      ssize_t read_size=read(fd,&buff,sizeOfParameter);
		if (read_size==0){
		   errx(5,"Can not read the segment.");
	   }
      write(1,&buff,sizeOfParameter);
   }
   printf("\n");
}

bool checkRegex(char * validValues, const char * name){
	regex_t regex;

	if (regcomp(&regex, validValues, REG_EXTENDED) != 0)
	{
		errx(4, "Incorrect regex fot this parameter.");
	}

	if (!(regexec(&regex, name, 0, NULL, 0) == 0))
	{
		regfree(&regex);
		return false;
	}

	regfree(&regex);
	return true;
}

void parameterValidation(int32_t fd,int32_t segmentIdentificater,struct segment parameter){


	if(segmentIdentificater==-1){
      close(fd);
      errx(6,"Can not find this type in list with segment's type.");
   }

   lseek(fd,sizeOfSegment*(parameter.segNumber),SEEK_SET);  
	char byte;

	ssize_t read_size;
   if((read_size=read(fd,&byte,sizeof(char)))==0){
      close(fd);
      errx(5,"Can not read first byte from segment.");
   }
   if(segmentTypesId[segmentIdentificater]!=(int)byte){
      close(fd);
      errx(8,"Wrong first byte for this segment.");
   }
}

void setName(char* file, char* function,char* parameterName,char* name){

	int32_t fd=open(file,O_RDWR);
	      
	if(fd==-1){
		 errx(3,"Failled to open %s in read mode.",file);
   }	
   
   int32_t indexInParametersName=segmentExist(parameterName);
   if(indexInParametersName==-1){
      close(fd);
		errx(9,"The segment does not exist");
	}
	
   struct segment parameter;
	parameter=setSegment(indexInParametersName);
	if(!checkRegex(parameter.values,name)){
		close(fd);
      errx(10,"The name who you want does not match regex.");
	}

   int32_t segmentIdentificater=checkFirstBit(indexInParametersName); 
   parameterValidation(fd,segmentIdentificater,parameter);

   if(strlen(name)>sizeOfPiece[segmentIdentificater] && strcmp(segmentTypes[segmentIdentificater],"numeric")){
      close(fd);
      errx(11,"Wanted name is too long.");
   }

	off_t offset=sizeOfSegment*parameter.segNumber + sizeOfMetadata+sizeOfPiece[segmentIdentificater]*parameter.positionNumber;
	if((offset+sizeOfPiece[segmentIdentificater])>countParameter*sizeOfSegment){
		close(fd);
      errx(11,"Segmen is out of range.");
	}

	if(!strcmp(function,"-s")){
		setBit(fd,parameter.positionNumber,parameter.segNumber);
	}	

	lseek(fd,offset,SEEK_SET); 
	writeParameterName(fd,segmentIdentificater,name);
	close(fd);
}



void getName(char* file,char* function,char* parameterName){

   int32_t fd=open(file,O_RDWR);      
	if(fd==-1){
	   errx(3,"Failled to open %s in read mode.",file);
   }	

	int32_t indexInParametersName=segmentExist(parameterName);
	if(indexInParametersName==-1){
	   close(fd);
   	errx(9,"The segment does not exist");
	}

	struct segment parameter;
	parameter=setSegment(indexInParametersName);
   int32_t segmentIdentificater=checkFirstBit(indexInParametersName);
   parameterValidation(fd,segmentIdentificater,parameter); 

	if(!strcmp(function,"-g") && !checkBit(fd,parameter.segNumber,parameter.positionNumber)){

	   printf("The bit is not set.\n");
	   goto END;
   }

   off_t offset=sizeOfMetadata + parameter.positionNumber*sizeOfPiece[segmentIdentificater]+parameter.segNumber*sizeOfSegment;

   lseek(fd,offset,SEEK_SET);
   readParameterName(fd,segmentTypes[segmentIdentificater] ,sizeOfPiece[segmentIdentificater]);

   END: ; 
   close(fd);
}


void list(char* file,char* function,uint32_t startIndex,uint32_t counter,char* arguments[]){

   int32_t fd=open(file,O_RDONLY);   
	if(fd==-1){
	   errx(3,"Failled to open %s in read and write mode.",file);
	}

	for(uint32_t i=startIndex; i<counter; i++){

      int indexInParametersName=segmentExist(arguments[i]);
		if(indexInParametersName==-1){
			printf("Parameter %s does not exist.\n",arguments[i]);
         goto NEXT;
		}
		struct segment parameter;
		parameter=setSegment(indexInParametersName);

      int segmentIdentificater=checkFirstBit(indexInParametersName); 
      parameterValidation(fd,segmentIdentificater,parameter);

		if(!strcmp(function,"-l") && !checkBit(fd,parameter.segNumber,parameter.positionNumber)){
         goto NEXT;
      }
      
      if(counter!=countParameter){

         off_t offset=sizeOfMetadata + parameter.positionNumber*sizeOfPiece[segmentIdentificater]+parameter.segNumber*sizeOfSegment;
         lseek(fd,offset,SEEK_SET);

         readParameterName(fd,segmentTypes[segmentIdentificater] ,sizeOfPiece[segmentIdentificater]);
      } else {
         printf("%s\n",parameter.name);
      }
      NEXT: ;

      lseek(fd,0,SEEK_SET);
   }
   
   close(fd);
}



void clearBit(int32_t fd, uint32_t position,uint32_t numberOfSegment){
	lseek(fd,sizeOfSegment*numberOfSegment+1+position/bitsInByte,SEEK_SET);
	char buff;
	read(fd,&buff,sizeof(char));
	buff &= ~(1<<(bitsInByte-1-position));
	lseek(fd,-1,SEEK_CUR);
	write(fd,&buff,sizeof(char));
}


void byteChange(char* file,char* parameterName,char* byte){

   int32_t fd=open(file,O_RDWR);      
	if(fd==-1){
	   errx(3,"Failled to open %s in read and write mode.",file);
   }

   int32_t indexInParametersName=segmentExist(parameterName);
	if(indexInParametersName==-1){
	   close(fd);
   	errx(9,"The segment does not exist");
	}
	
   struct segment parameter;
	parameter=setSegment(indexInParametersName);

   int segmentIdentificater=checkFirstBit(indexInParametersName); 
   if(segmentIdentificater==-1){
      close(fd);
      errx(9,"Does not exist this type of segment.");
   }
   
   lseek(fd,sizeOfSegment*(parameter.segNumber),SEEK_SET); 
   char byteInFile;
   if(read(fd,&byteInFile,sizeof(char))==0){
      close(fd);
      errx(5,"Can not read first byte from file.");
   }
   if(segmentTypesId[segmentIdentificater]!=(int)byteInFile){
      close(fd);
      errx(5,"Wrong first byte for this segment.");
   }

	if(!strcmp(byte,"0")){
      clearBit(fd, parameter.positionNumber,parameter.segNumber);
   } else if (!strcmp(byte,"1")){
      setBit(fd, parameter.positionNumber,parameter.segNumber);
   } else {
      close(fd);
      errx(2,"Incorrect input. Last argument must be 0 or 1.Use option -h for more information.");
   }

   close(fd);
}

void createSegment(char* type,int32_t fd){

	for(uint32_t i=0; i<typesCount;i++){
		if(!strcmp(type,segmentTypes[i])){
			int32_t typeId=segmentTypesId[i];
			char id=typeId;
			if(write(fd,&id,sizeof(char))<1){
	         close(fd);
            errx(5,"Can not create %d segment.",i+1);
         }
      }
   }

   char buff=0;
   for(uint32_t i=0;i<sizeOfSegment-1;i++){
      if(write(fd,&buff,sizeof(char))<1){
         close(fd);
         errx(5,"Can not make %s segment.",type);
      }
   }

   return;	
}

void createFile(char* file,uint32_t counter, char* arguments[]){

   if(counter%2==0 || strcmp(arguments[3],"0")){
      errx(2,"Incorrect input. Not every segment has a type or the first segment is not 0.");
   }		
   
   for(uint32_t i=3; i<counter-2;i+=2){
	   if((atoi(arguments[i])+1)!=atoi(arguments[i+2])){
         errx(2,"Segments' number are not sequential.The file can not be create.");
      }
   }

   int32_t fd=open(file,O_CREAT|O_TRUNC|O_WRONLY,S_IRWXU|S_IRWXG);
   if(fd==-1){
      errx(4,"Can not create or write in %s.",file);
   }
	
   for(uint32_t i=4;i<counter;i+=2){

	   if(!strcmp(arguments[i],"t")){
         createSegment("text",fd);
      } else if (!strcmp(arguments[i],"b")){
         createSegment("byte",fd);
      } else if (!strcmp(arguments[i],"n")){
      createSegment("numeric",fd);
      } else {
         close(fd);
         errx(5,"Does noe exist '%s' type.\n",arguments[i]);
      }
   }         
   close(fd);
}

int main(int argc,char* argv[]){
   
   if(argc==1){
      errx(2,"Not enought arguments.Use option -h for more information.");
   }
	
   if(!isLittleEndian()){
      errx(3,"The program is made for little endian architecture.Your architectyre is big endian.");
}
   setbuf(stdout,NULL);
   struct stat file;
   stat(argv[1],&file);
   if(S_ISREG(file.st_mode)){
      
      if((!strcmp(argv[2],"-s") || !strcmp(argv[2],"-S")) && argc==5){ 
	      setName(argv[1],argv[2],argv[3],argv[4]);

      } else if((!strcmp(argv[2],"-g") || !strcmp(argv[2],"-G"))&& argc==4){
	      getName(argv[1],argv[2],argv[3]);

      } else if(!strcmp(argv[2],"-l") || !strcmp(argv[2],"-L")){

		   if(argc>3){

            list(argv[1],argv[2],3,(uint32_t)argc,argv);

         } else if (argc==3){  

	         list(argv[1],argv[2],0,countParameter,parametersName);

         }

      } else if(!strcmp(argv[2],"-b") && argc==5){

	      byteChange(argv[1],argv[3],argv[4]);

      } else if(argc>3 && !strcmp(argv[2],"-c") && (uint32_t)argc<=(sizeOfSegment*2 + 3)){

         createFile(argv[1],argc,argv);

      }else {

            errx(2,"Incorrect input.Use option -h for more information.");
      }
  
   } else {

      if(!strcmp(argv[1],"-h")){  
	      readHelp();  
   
      } else if(argc>3 && !strcmp(argv[2],"-c") && (uint32_t)argc<=(sizeOfSegment*2 + 3)){

         createFile(argv[1],argc,argv);
	   
      } else {

         errx(2,"Incorrect input.Use option -h for more information.");  
      }  
           
   }
   exit(1);
}

