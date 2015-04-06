#include <pthread.h>
#ifndef SOCKETLIST_H
#define SOCKETLIST_H

struct SocketList{
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
};



#endif
void PrintSocket(void);
void ReceivedKeepAlive(int);
struct SocketList *FindSocket(int);
void AddSocket(int,char *,int);
void SocketList_Add(struct SocketList *);
void SocketList_Delete(struct SocketList *);
void UpdateSocket(int,char *,int);
void Pthread_Update(int,pthread_t);
void ForwardMessage(char *,char *,int,int);
void UpdateTimer(int,int);
void SendCheck(void);
void ForwardCheck(char *,int);
