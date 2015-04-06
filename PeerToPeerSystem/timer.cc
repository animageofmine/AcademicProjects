#include "timer.h"
#include "message_header.h"
#include "file.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "message_cache.h"
#include <stdlib.h>
#include "socket_list.h"
extern struct SocketList *socketHead;
extern pthread_mutex_t socketListLock;
extern pthread_mutex_t MessageCacheLock;
extern void SendMessage(int,char*,char*,int);
extern MessageCache *cacheHead;
extern int KeepAliveTimeout;
extern char * CurrentTime(void);
extern void* ResetAll(void*);
extern double GetTimeOfDay();
/*(struct SocketList{
	int sockFd;
	int timer;
	pthread_mutex_t socketLock;
	struct SocketList *next;
	int timeLeftToSend;
	int timeLeftToReceive;
	char *hostname;
	int portNumber;
	char type;
	pthread_t self;
};*/
extern int dontReset;
extern double GetTimeOfDay();
extern double startTime;
extern void SocketList_Delete(struct SocketList *);
extern int shutFlag;
//timer resets after every one sedcond
void *Timer(void *noData){
	//infinite while loop for a timer
	while(1){
		if(shutFlag ==1){
			exit(1);
		}
		sleep(1);
		if(shutFlag ==1){
			exit(1);
		}
		//printf("\n timer--");
		pthread_mutex_lock (&socketListLock);
		int pflag = 0;
		if(socketHead != NULL){
			struct SocketList *current = socketHead;
			//decreasinf the time related to each neighbor and decrease it by 1 second
			while(current != NULL){
				if(current->timer != 0){
					pthread_mutex_lock(&current->socketLock);
					if((current->timeLeftToSend == 0) || (current->timeLeftToReceive == 7)){
						if(current->timeLeftToSend == 0){
							current->timeLeftToSend = (int)KeepAliveTimeout/2;
							pthread_t timer;
							pthread_create(&timer, NULL, SendKeepAlive, (void *)current);
						}
						if(current->timeLeftToReceive == KeepAliveTimeout){
							//Connection Lost");
							//pthread_mutex_unlock (&socketListLock);
							//close(current->sockFd);
							//printf("\n time 1--------------");
							pflag = 1;
							//SocketList_Delete(current);
							//SendCheck();
							//pthread_mutex_lock (&socketListLock);
						}
					}else{
						current->timeLeftToSend--;
						current->timeLeftToReceive++;
					}
					pthread_mutex_unlock(&current->socketLock);
				}
				if(pflag == 1){
					SocketList *temp = current->next;
					close(current->sockFd);
					SocketList_Delete(current);
					current = temp;
				}
				if(current!= NULL){
					current = current->next;
				}
				if(pflag == 1){
					//printf("\ time time -----------------");
				}
				//printf("\n time 2");
				pflag = 0;
			}
		}
		pthread_mutex_unlock (&socketListLock);
		pthread_mutex_lock (&MessageCacheLock);
		//decrease the time related to each message in the cache
		struct MessageCache *temp = NULL;
		if(cacheHead != NULL){
			if(cacheHead->timeLeft == 0){
				//printf("\n UOID head node deleted");
				temp = cacheHead;
				cacheHead = cacheHead->next;
				delete temp;
			}
			struct MessageCache *cacheNode = cacheHead;
			if(cacheNode != NULL){
				while(cacheNode->next != NULL){
					if(cacheNode->next->timeLeft <= 0){
							//printf("\n UOID node deleted");
							temp = cacheNode->next;
							cacheNode->next = cacheNode->next->next;
							delete temp;
					}
					cacheNode = cacheNode->next;
					if(cacheNode == NULL){
						break;
					}
					
				}	
			}			
			MessageCache *currentCache = cacheHead;
			//printf("\n decreasing timw");
			while(currentCache != NULL){
				if(currentCache->checkTime != -1){
					//decrease check time"
					currentCache->checkTime--;
					//printf("\n check time = %d",currentCache->checkTime);
					//printf("\n currentCache->beaconFound = %d",currentCache->beaconFound);
					if(currentCache->checkTime == 0){
						if(currentCache->beaconFound == 1){
							//dont reset time ---------------------------");
							dontReset = 0;
						}else{
							pthread_mutex_unlock (&MessageCacheLock);						
							FindUOIDSocket(currentCache->UOID);
							pthread_mutex_lock (&MessageCacheLock);
							//close(sock);
							
							pthread_t reset;
							pthread_create(&reset, NULL, ResetAll, (void *)0);
							//reset time ---------------------------");
							dontReset = 1;
						}
					}
				}
				//printf("\n cache time --");
				currentCache->timeLeft--;
				currentCache = currentCache->next;
			}
		}
		pthread_mutex_unlock (&MessageCacheLock);
	
	}
}
//this thread s forked whenever one needs tpo send a keepalive mesage
void *SendKeepAlive(void *data){
	struct SocketList *node;
	node = (struct SocketList *)data;
	pthread_mutex_lock (&socketListLock);
	SocketList *socket = NULL;;
	socket = (struct SocketList *)node;
	MessageHeader header;
	header.messageType = 248; //code for KEEPALIVE
	header.TTL = 1;
	header.dataLength = 0;
	header.CreateHeader();
	int sendFlag = 0;
	int childSd;
	if(socket != NULL){
		sendFlag = 1;
		childSd = socket->sockFd;
		FDATA writeData;
		writeData.logType = 's';
		for(int index = 0;index<4;index ++){
			writeData.UOID[index] = header.header[17 + index];
		}
		//printf("\n before sprinf");
		sprintf(writeData.target,"%s %d",node->hostname,node->portNumber);
		//p	rintf("\n after sprinf");
		writeData.msgType = "KPAV";	
		writeData.dataLength = 0;	
		writeData.TTL = header.TTL;
	
		writeData.time = GetTimeOfDay();
		writeData.WriteToFile();
		
	}
	pthread_mutex_unlock (&socketListLock);
	if(sendFlag == 1){
		SendMessage(childSd,header.header,NULL,0);
	}
	//printf("\n----------socket = %d",childSd);
	//printf("\n node->hostname = %s",node->hostname);
	//printf("\n node->portNumber = %d",node->portNumber);

	//printf("\n before sending keepalive");
	//printf("\n Sending keepalive");
	
	//printf("\n after sending keepalive");
	pthread_exit(NULL);
	return (void *)0;
}
