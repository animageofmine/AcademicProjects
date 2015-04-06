#include "file.h"
#include <pthread.h>
#include <fstream>

extern FILE *fp;
extern pthread_mutex_t fileLock;

FDATA :: FDATA(){
	time = 0.0;
	target = new char[25];
	msgType = new char[10];
	dataLength = 0;
	TTL = 0;
	UOID = new unsigned char[20];
	data = new char[100];
	oneLine = new char[200];
	errorMsg = new char[128];
}


void FDATA :: WriteToFile(){
	sprintf(oneLine,"%c %10.03f %s %s %d %d %02x%02x%02x%02x %s",logType,time,target,msgType,dataLength,TTL,UOID[0],UOID[1],UOID[2],UOID[3],data);
   	pthread_mutex_lock(&fileLock);
   	fputs(oneLine, fp);  // writing to the file we opened
	fputs("\n",fp);
	pthread_mutex_unlock(&fileLock);


}


	