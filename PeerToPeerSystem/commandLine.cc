#include <pthread.h>
#include <stdio.h>
#include "socket_list.h"
#include "file.h"
#include "message_header.h"
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "message_cache.h"
#include <ctype.h>

extern int JoinTimeout;
extern int initFilePresent;
extern int node[10];
//for(int index = 0;index< 10;index ++){
//	node[index] = -1;
//}
extern int links[50];
extern int linkIndex;
extern int nodeIndex;

extern MessageCache *cacheHead;
extern pthread_mutex_t socketListLock;
struct hostLocation{
	int portNumber;
	char *hostname;
	int flag;
	int sockFd;
};

extern FILE *fp;
extern char *CurrentTime(void);
extern int SendMessage(int,char *,char *,int);
extern SocketList *socketHead;
extern int tcpSock;
extern double GetTimeOfDay();
extern double startTime;
extern int myPortNumber;
extern int alarmInt;

void *InputThread(void *myData){
	int flag = 0;
	hostLocation *nodeLocation;
	nodeLocation = (struct hostLocation *)myData;
	char *command = NULL;
	int spaceCount = 0;
	int index1 =0,index2 = 0,index3 = 0,index4 = 0;
	char *name1,*name2,*name3,*name4;
	while(1){
		
		name1 = new char[20];
		name2 = new char[20];
		name3 = new char[20];
		name4 = new char[20];
		command = new char[50];
		if(alarmInt == 1){
			
			exit(0);
		}
		usleep(10000);
		if(initFilePresent == 0){
			//printf("\n Please Wait. Joining with other nodes in the network");
			//sleep(JoinTimeout);
		}
		printf("\nservant:%d>",myPortNumber);
		gets(command);
		if(strlen(command) == 0){
			continue;
		}
		//parse the input copmmand
		for(int index = 0;index < (int)strlen(command);index++){
			if(command[index] == ' '){
				spaceCount++;
				continue;
			}
			if(spaceCount == 0){
				name1[index1] = tolower(command[index]);
				index1++;
			}
			if(spaceCount == 1){
				name2[index2] = tolower(command[index]);
				index2++;
			}
			if(spaceCount == 2){
				name3[index3] = (command[index]);
				index3++;
			}
			if(spaceCount == 3){
				name4[index4] = (command[index]);
				index4++;
			}

		}
		name1[index1] = '\0';
		name2[index2] = '\0';
		name3[index3] = '\0';
		name4[index4] = '\0';
		spaceCount = 0;
		index1 = index2 = index3 = index4 = 0;
		//printf("\n command = %s",command);
		//printf("\n name1 = '%s' name 2 = '%s' name3 = '%s' name4 = '%s'",name1,name2,name3,name4);
		//sending shutdown
		if(strcmp(name1,"shutdown") == 0){
			//printf("\n in shutdown");

			if(socketHead != NULL){
				pthread_mutex_lock(&socketListLock);
				struct SocketList *current = socketHead;
				pthread_mutex_unlock(&socketListLock);
				MessageHeader *header;
				header = new MessageHeader;
				header->messageType = 247;
				header->TTL = 1;
				header->dataLength = 1;	
				header->CreateHeader();
				unsigned char *UOID = new unsigned char[20];
				memcpy(&UOID[0],&header->UOID[0],20);
				while(1){
					
					
					//printf("\n UOID = '");
					//for(int index = 0;index <20;index ++){
					//	printf("%02x",UOID[index]);
					//}
					//printf("'");

					FDATA *writeData;
					writeData = new FDATA;
					writeData->logType = 's';
					for(int index = 0;index<4;index ++){
						writeData->UOID[index] = UOID[16 + index];
					}
					pthread_mutex_lock(&socketListLock);
					//struct SocketList *current = socketHead;
					int sock = current->sockFd;
					sprintf(writeData->target,"%s %d",current->hostname,current->portNumber);
					if(current->next != NULL){
						current = current->next;
					}else{
						flag = 1;
					}
					writeData->data[0] = '1';
					pthread_mutex_unlock(&socketListLock);
					writeData->msgType = "NTFY";		
					writeData->dataLength = 1;
					//printf("\n parsedMsg.TTL ------>= %d",parsedMsg.TTL);
					writeData->TTL = header->TTL;
					writeData->time = GetTimeOfDay();
					
					//printf("\n sending notify");
					SendMessage(sock,header->header,"1",1);
					usleep(10000);
					writeData->WriteToFile();
					delete writeData;
					delete header;
					if(flag == 1){
						break;
					}
				}
			}
			//PrintSocket();

		    printf("\n Shutdown time. Please send requests later. Thank You\n");
		    close(tcpSock);
		    //set the alarm flag
		    //intFlag = 1;
		    fclose(fp);
		    exit(0);
		}
		//sending status
		if((strcmp(name1,"status") == 0) && (strcmp(name2,"neighbors")==0)){
			
			
			int TTL = atoi(name3);
		
					
					for(int index = 0;index< 10;index ++){
						node[index] = -1;
					}
					for(int index = 0;index < 50;index++){
						links[index] = -1;
					}
					linkIndex= 0;
					nodeIndex = 0;
					node[nodeIndex++] = myPortNumber; 
				if(TTL > 0){
					//creating header
					MessageHeader *header;
					header = new MessageHeader;
					header->messageType = 172;
					header->TTL = TTL;
					header->dataLength = 1;	
					header->CreateHeader();
					unsigned char *UOID = new unsigned char[20];
					memcpy(&UOID[0],&header->UOID[0],20);
					FindUOID(UOID,0,1);
					FDATA *writeData;
					writeData = new FDATA;
					writeData->logType = 's';
					for(int index = 0;index<4;index ++){
						writeData->UOID[index] = UOID[16 + index];
					}
					writeData->data[0] = '1';
					writeData->msgType = "STRQ";		
					writeData->dataLength = 1;
					//printf("\n parsedMsg.TTL ------>= %d",parsedMsg.TTL);
					writeData->TTL = header->TTL;
					writeData->time = GetTimeOfDay();
					pthread_mutex_lock(&socketListLock);
					SocketList *current = socketHead;
					int sendCount = 0;
					while(current != NULL){
						
							sendCount++;
						
						current = current->next;
					}
					int sendArray[sendCount];
					int index = 0;
					current = socketHead;
					while(current != NULL){
						
						sendArray[index] = current->sockFd;
						index++;
						
						current = current->next;
					}
					pthread_mutex_unlock(&socketListLock);
					SocketList *sendPort = NULL;
					index = 0;
					while(sendCount){
						sendPort = FindSocket(sendArray[sendCount - 1]);
						if(sendPort != NULL){
							sprintf(writeData->target,"%s %d",sendPort->hostname,sendPort->portNumber);
							writeData->WriteToFile();
							SendMessage(sendPort->sockFd,header->header,"1",header->dataLength);
						}
						sendCount--;
						//current = current->next;
					}
				
					sleep(3);
				}
					
					pthread_mutex_lock(&socketListLock);
					
					SocketList *tempCurr = socketHead;
					if(TTL == 0){
						//node[nodeIndex++] = myPortNumber;
						while(tempCurr != NULL){
							node[nodeIndex++] = tempCurr->portNumber;
							tempCurr = tempCurr->next;
						}
					}
					SocketList *tempCurrent = socketHead;
					while(tempCurrent != NULL){
						if(tempCurrent->timer!=0){
							links[linkIndex++] = myPortNumber;
							links[linkIndex++] = tempCurrent->portNumber;
							
						}
						tempCurrent = tempCurrent->next;
					}
					//writing to nam file
					//printf("\n link index = %d",linkIndex);
					pthread_mutex_unlock(&socketListLock);
					char *writeNam = NULL;
					FILE *nam;
					if((nam = fopen(name4,"w")) == NULL){
						printf("\n Nam Open Failed");
						exit(0);
					}else{
						writeNam = new char[40];
						sprintf(writeNam,"V -t * -v 1.0a5");
						fputs(writeNam,nam);
						fputs("\n",nam);
						delete writeNam;
						for(int index = 0;index<nodeIndex;index++){
							writeNam = new char[40];
							sprintf(writeNam,"n -t * -s %d -c red -i black",node[index]);
							fputs(writeNam,nam);
							fputs("\n",nam);
							delete writeNam;
						}
						for(int index = 0;index<linkIndex;index++){
							writeNam = new char[40];
							sprintf(writeNam,"l -t * -s %d -d %d -c blue",links[index],links[index+1]);
							fputs(writeNam,nam);
							fputs("\n",nam);
							delete writeNam;
							index++;
						}
						
					}
					fclose(nam);
					if(flag == 1){
						break;
					}
					
				
				
			
		}
		
		delete name1;
		delete name2;
		delete name3;
		delete name4;
		delete command;

	}
	return (void *)0;
}


