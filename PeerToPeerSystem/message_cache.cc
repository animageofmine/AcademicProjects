#include "message_cache.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

extern pthread_mutex_t MessageCacheLock;
extern MessageCache *cacheHead;
extern int MsgLifetime;
extern int JoinTimeout;
int AddToCache(struct MessageCache *temp){
	//printf("\n before lock acquire");
	
	pthread_mutex_lock (&MessageCacheLock);
	struct MessageCache *current = cacheHead;
	//printf("\n UOID Add_Add");
	if(cacheHead == NULL){
		cacheHead = temp;
	}else{
		while(current->next !=  NULL){
			current = current->next;
		}
		current->next = temp;
	}
	pthread_mutex_unlock(&MessageCacheLock);
	return 0;
}

//find if uoid is present in the cache
int FindUOID(unsigned char *UOID,int sockFd,int my){
	
	pthread_mutex_lock(&MessageCacheLock);
	struct MessageCache *current = cacheHead;
	struct MessageCache *previous = NULL;
	if(cacheHead == NULL){
		//printf("\n in cache head");
		
		MessageCache *cacheNode = new MessageCache;
		cacheNode->UOID = new unsigned char[20];
		for(int index = 0;index < 20;index ++){
			cacheNode->UOID[index] = UOID[index];
		}
		cacheNode->timeLeft = MsgLifetime;
		cacheNode->my = my;
		cacheNode->sockFd = sockFd;
		cacheNode->checkTime = -1;
		cacheNode->beaconFound =0;
		cacheNode->next = NULL;
		cacheHead = cacheNode;
		pthread_mutex_unlock(&MessageCacheLock);
		//printf("\n uoid not found");
		return 0;
	}else{
		
		while(current != NULL){
		
			if(memcmp ( UOID, current->UOID, 20 ) == 0){
				//printf("\n UOID found");
				pthread_mutex_unlock(&MessageCacheLock);
				return 1;
			
			}
		//printf("\n *************List UOID = ");
			//for(int index = 0;index < 20;index++){
			//	printf("%02x",current->UOID[index]);
			//}
			//printf("**************");
			previous = current;
			current = current->next;
			
		}
	}
	
	MessageCache *cacheNode = new MessageCache;
	//printf("\n outside cache head");
	cacheNode->UOID = new unsigned char[20];
	for(int index = 0;index < 20;index ++){
		cacheNode->UOID[index] = UOID[index];
	}
	cacheNode->timeLeft = MsgLifetime;
	cacheNode->my = my;
	cacheNode->sockFd = sockFd;
	cacheNode->checkTime = -1;
	cacheNode->beaconFound = 0;
	cacheNode->next = NULL;
	previous->next = cacheNode;
	
	pthread_mutex_unlock(&MessageCacheLock);
	//printf("\n uoid not found");
	return 0;
}


//mard neighbor found if beacon node responds tp check message
int FoundNeighbor(unsigned char *UOID){
	pthread_mutex_lock(&MessageCacheLock);
	struct MessageCache *current = cacheHead;
	while(current != NULL){
		if(memcmp ( UOID, current->UOID, 20 ) == 0){
			if(current->my == 1){
				current->beaconFound = 1;
				pthread_mutex_unlock(&MessageCacheLock);
				return 1;
			}
		}
		current = current->next;
	}
	pthread_mutex_unlock(&MessageCacheLock);
	return 0;
	
}

//find if uoid belonds the that node or not
int FindMy(unsigned char *UOID){
	pthread_mutex_lock(&MessageCacheLock);
	struct MessageCache *current = cacheHead;
	while(current != NULL){
		if(memcmp ( UOID, current->UOID, 20 ) == 0){
			if(current->my == 1){
				pthread_mutex_unlock(&MessageCacheLock);
				return 1;
			}
		}
		current = current->next;
	}
	pthread_mutex_unlock(&MessageCacheLock);
	return 0;
}

//set time for the check message,after this time drop the check message
int SetTimeForCheck(unsigned char *UOID){
	pthread_mutex_lock(&MessageCacheLock);
	struct MessageCache *current = cacheHead;
	while(current != NULL){
		if(memcmp ( UOID, current->UOID, 20 ) == 0){
			current->checkTime = JoinTimeout;
			current->beaconFound = 0;
		}
		current = current->next;
	}
	pthread_mutex_unlock(&MessageCacheLock);
	return 0;
}


//find the socket assigned to the uoid
int FindUOIDSocket(unsigned char *UOID){
	pthread_mutex_lock(&MessageCacheLock);
	struct MessageCache *current = cacheHead;
	while(current != NULL){
		if(strcmp((const char *)current->UOID,(const char *)UOID) == 0){
			pthread_mutex_unlock(&MessageCacheLock);
			//printf("\n socket found in uoid");
			return current->sockFd;
		}
		current= current->next;
	}
	pthread_mutex_unlock(&MessageCacheLock);
	return 0;
}

//delete the message cache

void Cache_Delete(){
		pthread_mutex_lock (&MessageCacheLock);
		
		struct MessageCache *temp = NULL;
		if(cacheHead != NULL){
			if(cacheHead->my == 1){
				//printf("\n UOID head node deleted");
				temp = cacheHead;
				cacheHead = cacheHead->next;
				delete temp;
			}
			struct MessageCache *cacheNode = cacheHead;
			if(cacheNode != NULL){
				while(cacheNode->next != NULL){
					//printf("\n check 7 ");
					if(cacheNode->next->my == 1){
						//printf("\n check 1 ");
						//printf("\n UOID node deleted");
						temp = cacheNode->next;
						//printf("\n check 2 ");
						cacheNode->next = cacheNode->next->next;
						//printf("\n check 3 ");
						delete temp;
						//printf("\n check 4 ");
						
					}
					//printf("\n check 5 ");
					cacheNode = cacheNode->next;
					//printf("\n check 6 ");
					if(cacheNode == NULL){
						break;
					}
				}	
			}
			
		}
		pthread_mutex_unlock (&MessageCacheLock);
}
//status neighbors 3 er
