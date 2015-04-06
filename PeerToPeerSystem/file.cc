#include "file.h"
#include <pthread.h>
#include <fstream>
#include <string.h>

extern FILE *fp;
extern pthread_mutex_t fileLock;
extern char *logFilePath;
//extern char *LogFilename;
FDATA :: FDATA(){
	time = 0;
	target = new char[25];
	msgType = new char[10];
	dataLength = 0;
	TTL = 0;
	UOID = new unsigned char[20];
	data = new char[100];
	oneLine = new char[200];
	errorMsg = new char[128];
}

//writing logs to file
void FDATA :: WriteToFile(){
	sprintf(oneLine,"%c %10.03f %s %s %d %d %02x%02x%02x%02x %s",logType,time,target,msgType,dataLength,TTL,UOID[0],UOID[1],UOID[2],UOID[3],data);
   	pthread_mutex_lock(&fileLock);
   	fp = fopen(logFilePath, "a");
   	fputs(oneLine, fp);  // writing to the file we opened
	fputs("\n",fp);
	fclose(fp);
	pthread_mutex_unlock(&fileLock);


}
//writing errors to file
void FDATA :: WriteErrorToFile(){
	//int i;
	pthread_mutex_lock(&fileLock);
	//time=0;strcpy(target,"null");strcpy(msgType,"null");
	//dataLength=0;TTL=0;strcpy(data,"null");
	//for(i=0;i<20;i++)
	//UOID[i]=(unsigned char)'0';
	//strcpy(oneLine,"null");	
	fp = fopen(logFilePath, "a");
	fputs(errorMsg,fp);
	fputs("\n",fp);
	fclose(fp);
	pthread_mutex_unlock(&fileLock);
}
