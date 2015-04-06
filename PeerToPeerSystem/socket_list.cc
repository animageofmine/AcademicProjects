#include "socket_list.h"
#include "pthread.h"
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <unistd.h>
#include "file.h"
#include "message_header.h"
#include "message_cache.h"

#include <stdlib.h>
extern char *CurrentTime(void);
extern void SendMessage(int,char*,char*,int);
extern double startTime;
extern double GetTimeOfDay();
extern struct SocketList *socketHead;
extern pthread_mutex_t socketListLock;
extern pthread_t timer;
extern int TTL;
extern int KeepAliveTimeout;

//send check messages to all the neighbors when receive a notigy message
void SendCheck(){
	MessageHeader *header = new MessageHeader;
	header->messageType = 246;
	header->TTL = TTL;
	header->dataLength = 0;
	header->CreateHeader();
	unsigned char *UOID = new unsigned char[20];
	memcpy(&UOID[0],&header->UOID[0],20);
	FindUOID(UOID,0,1);
	SetTimeForCheck(UOID);
	memcpy(&header->header[1],&UOID[0],20);
	FDATA *write = new FDATA;
	write->logType = 's';
	write->msgType = "CKRQ";
	write->TTL = header->TTL;
	write->dataLength = header->dataLength;
	for(int index = 0;index<4;index ++){
		write->UOID[index] = header->UOID[16 + index];
	}
	write->data = "";
	//write->data[1] = '\0';
	write->time = GetTimeOfDay();	pthread_mutex_lock(&socketListLock);
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
	//if(index == sendCount){
		//printf("\n index == send count");
	//}
	//printf("\n send count = %d",sendCount);
	pthread_mutex_unlock(&socketListLock);
	SocketList *sendPort = NULL;
	index = 0;
	//sending message to all the neighbor
	while(sendCount){
		sendPort = FindSocket(sendArray[sendCount - 1]);
		if(sendPort != NULL){
			//printf("\n send port ->portNumber = %d",sendPort->portNumber);
			sprintf(write->target,"%s %d",sendPort->hostname,sendPort->portNumber);
			write->WriteToFile();
			SendMessage(sendPort->sockFd,header->header,NULL,header->dataLength);
		}
		sendCount--;
		
	}
}


//forward the check message to all the neighbors when received
void ForwardCheck(char *msgReceived,int dontSendPort){
	MessageHeader *header = new MessageHeader;
	header->ParseHeader(msgReceived);
	FDATA *write = new FDATA;
	write->logType = 'f';
	write->msgType = "CKRQ";
	short int tempTTL = (short int)header->TTL - 1;
	if(tempTTL > TTL){
		tempTTL = (short int)TTL;
	}
	write->TTL = tempTTL;
	tempTTL = tempTTL << 8;
	memcpy(&msgReceived[21],&tempTTL,1);

	
	write->dataLength = header->dataLength;
	for(int index = 0;index<4;index ++){
		write->UOID[index] = header->UOID[16 + index];
	}
	//write->data[0] = newPayload[0];
	//write->data[1] = '\0';
	write->time = GetTimeOfDay();
	//printf("\n msg received = %s and datalength = %d---------------",msgReceived,strlen(msgReceived));
	//printf("\n payload = %s---------------",newPayload);
	pthread_mutex_lock(&socketListLock);
	SocketList *current = socketHead;
	int sendCount = 0;
	//find which neighbor belong to the particular node and send join
	while(current != NULL){
		if(current->sockFd != dontSendPort){
			//sprintf(write->target,"%s %d",current->hostname,current->portNumber);
			//write->WriteToFile();
			//SendMessage(current->childSd,msgReceived,newPayload,header->dataLength);
			sendCount++;
		}
		current = current->next;
	}
	
	int sendArray[sendCount];
	int index = 0;
	current = socketHead;
	while(current != NULL){
		if(current->sockFd != dontSendPort){
			sendArray[index] = current->sockFd;
			index++;
		}
		current = current->next;
	}
	//if(index == sendCount){
	//	printf("\n index == send count");
	//}
	pthread_mutex_unlock(&socketListLock);
	SocketList *sendPort = NULL;
	index = 0;
	while(sendCount){
		sendPort = FindSocket(sendArray[sendCount - 1]);
		if(sendPort != NULL){
			sprintf(write->target,"%s %d",sendPort->hostname,sendPort->portNumber);
			write->WriteToFile();
			SendMessage(sendPort->sockFd,msgReceived,NULL,header->dataLength);
		}
		sendCount--;
		//current = current->next;
	}
	
}


//forward the join message to all the neighbors when received in case of beacon and non beacon
void ForwardMessage(char *message,char * newPayload,int length,int dontSendPort){
	FDATA writeData;	
	writeData.logType = 'f';
	for(int index = 0;index<4;index ++){
		writeData.UOID[index] = message[17 + index];
	}
	MessageHeader header;
	header.ParseHeader(message);
	writeData.msgType = "JNRQ";
	writeData.dataLength = header.dataLength;
	short int tempTTL = header.TTL;
	if(tempTTL > TTL){
		tempTTL = TTL;
	}
	tempTTL  = tempTTL << 8;
	memcpy(&header.header[21],&tempTTL,1);

	writeData.TTL = tempTTL;
	writeData.time = GetTimeOfDay();
	short int portNumber;
	memcpy(&portNumber,&newPayload[4],2);
	char *tempHost = new char[20];
	if(tempHost == NULL){
		printf("\n Memory allocation failed");
		exit(0);
	}
	//char *payload = new char[25];
	
	memcpy(&header.header[0],&message[0],27);
	//printf("\n tempHost = '");
	for(int index = 0;index < header.dataLength - 6;index ++){
		tempHost[index] = newPayload[index + 6];
		//printf("%c",tempHost[index]);
	}
	//printf("'");
	//printf("\n before temphost");
	//tempHost[header.dataLength - 2] = '\0';
	//printf("\n after temphost");
	sprintf(writeData.data,"%s_%d",tempHost,portNumber);
	delete tempHost;


	pthread_mutex_lock(&socketListLock);
	SocketList *current = socketHead;
	int sendCount = 0;
	while(current != NULL){
		if(current->portNumber != dontSendPort){
			sendCount++;
		}
		current = current->next;
	}
	int sendArray[sendCount];
	int index = 0;
	current = socketHead;
	while(current != NULL){
		if(current->portNumber != dontSendPort){
			sendArray[index] = current->sockFd;
			index++;
		}
		current = current->next;
	}
	//if(index == sendCount){
	//	printf("\n index == send count");
	//}
	pthread_mutex_unlock(&socketListLock);
	SocketList *sendPort = NULL;
	index = 0;
	while(sendCount){
		sendPort = FindSocket(sendArray[sendCount - 1]);
		if(sendPort != NULL){
			sprintf(writeData.target,"%s %d",sendPort->hostname,sendPort->portNumber);
			writeData.WriteToFile();
			SendMessage(sendPort->sockFd,message,newPayload,header.dataLength);
		}
		sendCount--;
		//current = current->next;
	}

}

//find the socket node to which a socket is assigned
struct SocketList *FindSocket(int sock){
	pthread_mutex_lock (&socketListLock);
	struct SocketList *current = socketHead;
	//printf("\n after sockethead");
	while(current != NULL){
			if(current->sockFd == sock){
			//printf("\n Socket Found");
			pthread_mutex_unlock(&socketListLock);
			return current;
		}
		current = current->next;
	}
	//printf("\n Socket not Found = %d",sock);
	pthread_mutex_unlock(&socketListLock);
	return NULL;
}


//add socket to a list so that its status can be foubnd any time
void AddSocket(int sock,char *hostname,int portNumber){
	struct SocketList *temp = new struct SocketList;
	temp->sockFd = sock;
	temp->next = NULL;
	temp->timer = 0;
	temp->timeLeftToSend = (int)KeepAliveTimeout/2;
	temp->timeLeftToReceive = 0;
	temp->hostname = new char[20];
	temp->next = NULL;
	if(hostname != NULL){
		sprintf(temp->hostname,"%s",hostname);
	}
	//strcpy(temp->hostname,hostname);
	temp->portNumber = portNumber;
	pthread_mutex_init(&temp->socketLock,NULL);
	SocketList_Add(temp);
}

//update the timer related to a socket when a check message is received
void UpdateTimer(int sock,int timer){
	pthread_mutex_lock (&socketListLock);
	struct SocketList *current = socketHead;
	while(current != NULL){
		if(current->sockFd == sock){
			pthread_mutex_lock(&current->socketLock);
			//printf("\n Socket Found");
			current->timer = timer;
			pthread_mutex_unlock(&current->socketLock);
		}
		current = current->next;
	}
	//Socket not Found in update socket= 
	pthread_mutex_unlock (&socketListLock);
	
}

//add socket to the list
void SocketList_Add(struct SocketList *temp){
	//printf("\n before lock acquire");
	pthread_mutex_lock (&socketListLock);
	struct SocketList *current = socketHead;
	//printf("\n SocketList_Add");
	if(socketHead == NULL){
		socketHead = temp;
	}else{
		while(current->next !=  NULL){
			//printf("\n current->sockfd = %d",current->sockFd);
			current = current->next;
		}
		current->next = temp;
	}
	pthread_mutex_unlock(&socketListLock);
}


//update a socket with a port number
void UpdateSocket(int sock,char *hostname,int portNumber){
	pthread_mutex_lock (&socketListLock);
	struct SocketList *current = socketHead;
	while(current != NULL){
		if(current->sockFd == sock){
			pthread_mutex_lock(&current->socketLock);
			//printf("\n Socket Found");
			sprintf(current->hostname,"%s",hostname);
			current->portNumber = portNumber;
			pthread_mutex_unlock(&current->socketLock);
		}
		current = current->next;
	}
	//Socket not Found in update socket=
	pthread_mutex_unlock (&socketListLock);
}


//reset the keepalive timer
void ReceivedKeepAlive(int sock){
	pthread_mutex_lock(&socketListLock);
	struct SocketList *current = socketHead;
	while(current != NULL){
		if(current->sockFd == sock){
			pthread_mutex_lock(&current->socketLock);
			current->timeLeftToReceive = 0;
			pthread_mutex_unlock(&current->socketLock);
			//Timer Reset");
			break;
		}
		current = current->next;
	}
	//printf("\n before mutex release");
	pthread_mutex_unlock(&socketListLock);
	//printf("\n after mutex release");
}

//delete socket from the list when connectionlost with the neighbor
void SocketList_Delete(struct SocketList *delNode){
	pthread_mutex_lock(&socketListLock);
	struct SocketList *current = socketHead;
	struct SocketList *temp;
	if(socketHead == delNode){
		//printf("\n sokct hed 1");
		temp = socketHead->next;
		//close(socketHead->sockFd);
		// In delete sokcet childSd 
		free(socketHead);
		//socketHead = NULL;
		socketHead = temp;
		//printf("\n sokct hed 2");
	}else{
		while(current != NULL){
			if(current->next == delNode){
				//printf("\n sokct hed 3");
				temp = current->next;
				//close(current->next->sockFd);
				current->next = current->next->next;
				//In delete sokcet childSd 
				free(temp);
				//printf("\n sokct hed 4");
				break;
			}else{
				//printf("current->next");
				current = current->next;
			}
		}
	}
	//printf("\n Socket Deleted----------------------------------");
	pthread_mutex_unlock(&socketListLock);
}
//debug function no use in the program
void PrintSocket(){
	
	
	//printf("\n rc = %d",rc);
	SocketList *current = NULL;
	pthread_mutex_lock(&socketListLock);
	current  = socketHead;
	while(current != NULL){
		//printf("\n current->sockFd = %d",current->sockFd);
		//close(current->sockFd);
		//rc = pthread_cancel(current->self);
		//printf("\n rc = %d",rc);
		current = current->next;
	}
	pthread_mutex_unlock(&socketListLock);
}


//update which thread is related to which socket so that all threads can be deleted when program shuts down
void Pthread_Update(int sock,pthread_t self){
	pthread_mutex_lock (&socketListLock);
	struct SocketList *current = socketHead;
	while(current != NULL){
		if(current->sockFd == sock){
			pthread_mutex_lock(&current->socketLock);
			//Socket Found in pthread update");
			current->self = self;
			pthread_mutex_unlock(&current->socketLock);
		}
		current = current->next;
	}
	//Socket not Found in update socket
	pthread_mutex_unlock (&socketListLock);

}

//struct SocketList *FindNode(struct SocketList *

