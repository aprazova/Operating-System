#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <stdint.h>
#include <stdbool.h>

const uint32_t sizeOfSegment=64;
const uint32_t sizeOfMetadata=8;
const uint32_t bitsInByte=8;

const uint32_t lineCount=5;
uint32_t lineId[]={0,1,2,3,4};
char* lineType[]={"text",
"text",
"numeric",
"byte",
"text"};

const uint32_t typesCount=3;
uint32_t sizeOfPiece[]={16,4,1};
int segmentTypesId[]={0,1,2};
char* segmentTypes[]={"text",
"numeric",
"byte"};

const uint32_t countParameter=15;
char* parametersName[]={ "device_name",
"rom_revision",
"serial_number",
"bd_addr_part0",
"bd_addr_part1",
"bd_pass_part0",
"serial_baudrate",
"audio_bitrate",
"sleep_period",
"serial_parity",
"serial_data_bit",
"serial_stop_bit",
"bd_pass_part1",
"rom_checksum_part0",
"rom_checksum_part1"};

const uint32_t segmentsNumber[]={0,0,0,1,1,1,2,2,2,3,3,3,4,4,4};
const uint32_t positionsInSegment[]={0,1,2,0,1,2,0,1,2,0,1,2,0,1,2};
char* validValues[]={"^[a-zA-Z0-9_-]+$",
"^[a-zA-Z0-9._-]+$",
"^[A-Z0-9]+$",
"^[A-Z0-9:]+$",
"^[A-Z0-9:]+$",
"^[a-z0-9]+$",
"^(1200|2400|4800|9600|19200|115200)$",
"^(32|96|128|160|192|256|320)$",
"^(100|500|1000|5000|10000)$",
"^(N|E|O)$",
"^(5|6|7|8)$",
"^(0|1)$",
"^[a-z0-9]+$",
"^[a-z0-9]+$",
"^[a-z0-9]+$"};

int32_t segmentExist(char* input);

struct segment setSegment(int32_t index);

bool isLittleEndian();

void readHelp();

int32_t checkFirstBit(int32_t index);

void setBit(int32_t fd, uint32_t position,uint32_t numberOfSegment);

void writeParameterName(int32_t fd,int32_t segmentIdentificater,char* name);

bool checkBit(int32_t fd,uint32_t segmentNumber, uint32_t position);

void readParameterName(int32_t fd,char* typeOfSegment,uint32_t sizeOfSegment);

bool checkRegex(char * validValues, const char * name);

void parameterValidation(int32_t fd,int32_t segmentIdentificater,struct segment parameter);

void setName(char* file, char* function,char* parameterName,char* name);

void getName(char* file,char* function,char* parameterName);

void list(char* file,char* function,uint32_t startIndex,uint32_t counter,char* arguments[]);

void clearBit(int32_t fd, uint32_t position,uint32_t numberOfSegment);

void byteChange(char* file,char* parameterName,char* byte);

void createSegment(char* type,int32_t fd);

void createFile(char* file,uint32_t counter, char* arguments[]);

#endif 

