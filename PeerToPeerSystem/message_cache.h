#include <pthread.h>
#ifndef MESSAGECACHE_H
#define MESSAGECACHE_H

struct MessageCache{
	unsigned char *UOID;
	int sockFd;
	int my;
	int timeLeft;
	int checkTime;
	int beaconFound;
	struct MessageCache *next;
	
};


#endif

int AddToCache(struct MessageCache *);
int FindUOID(unsigned char *,int,int);
int FindUOIDSocket(unsigned char*);
int FindMy(unsigned char *);
void Cache_Delete(void);
int SetTimeForCheck(unsigned char *);
int FoundNeighbor(unsigned char *);
