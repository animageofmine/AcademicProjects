#include "neighbour_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <fstream>
extern Neighbour *neighbourHead;
extern pthread_mutex_t neighbourListLock;
extern int InitNeighbours;
extern char *HomeDir;
extern void catch_alarm_int(int);
extern int alarmInt;



//add neighbor to the list when receive response to join messages
int AddNeighbour(int portNumber,char *hostname,long int distance){
	//printf("\n in add neighbour");
	struct Neighbour *temp = new Neighbour;
	if(temp == NULL){
		printf("\n Malloc Failed");
		exit(0);
	}
	temp->hostname = new char[20];
	if(temp->hostname == NULL){
		printf("\n Malloc Failed");
		exit(0);
	}
	//printf("\n hostname = %s",hostname);
	sprintf(temp->hostname,"%s",hostname);
	temp->portNumber = portNumber;
	temp->distance = distance;
	temp->next = NULL;
	//printf("\n before neighbour lock");
	pthread_mutex_lock (&neighbourListLock);
	
	struct Neighbour *current = neighbourHead;
	if(neighbourHead == NULL){
		neighbourHead = temp;
	}else{
		//check the distance between the neighhbors
		if(neighbourHead->distance > distance){
			struct Neighbour *temp1;
			temp1 = neighbourHead; 
			neighbourHead = temp;
			neighbourHead->next = temp1;
			//printf("\n distance less than");
			pthread_mutex_unlock (&neighbourListLock);
			return 1;
		}
		while(current->next !=  NULL){
			if(current->distance < distance && current->next->distance > distance){
				//printf("\n distance in between");
				struct Neighbour *temp1;
				temp1 = current->next;
				current->next = temp;
				current->next->next = temp1;
				pthread_mutex_unlock (&neighbourListLock);
				return 1;
			}
			current = current->next;
		}
		current->next = temp;
	}
	pthread_mutex_unlock(&neighbourListLock);
	//printf("\n after neighbour lock");
	return 0;
}

//write the neighbors to the ini file in response to the join message
void PrintNeighbour(){
	pthread_mutex_lock(&neighbourListLock);
	char *directory = new char[50];
   	memcpy(&directory[0],&HomeDir[1],strlen(HomeDir)-1);
   	directory[strlen(HomeDir)-1]='\0';
   	strcat(directory,"/init_neighbor_list");
   	char *pathName;
   	int x;
   	FILE *fp2;
   	int count = 0;
	int closeFlag = 0; 	
	struct Neighbour *tempCurrent = neighbourHead;
	int neighbor = 0;
	while(tempCurrent != NULL){
		neighbor++;
		tempCurrent = tempCurrent->next;
	}
 	struct Neighbour *current = neighbourHead;
 	if(current != NULL){
  		if ((fp2 = fopen(directory, "w")) == NULL) {
   		    printf("\nCouldn't open datafile for reading\n");
   	    	exit(1);	
 	  	}else{		
 	  		closeFlag = 1;
  	  		//printf("\n Open Successfull");
 	  	}	
	}	
	
	if(neighbor < InitNeighbours){	
 		alarmInt = 1;
 		catch_alarm_int(0);
 		exit(0);
	}
	

	if(neighbor >= InitNeighbours){
		while(current != NULL){
			if(count == InitNeighbours){
				break;
			}
			pathName = new char[50];
		   	sprintf(pathName,"%s:%d",current->hostname,current->portNumber);
	   	x=strlen(pathName);
			   	pathName[x]='\0';
		   	fputs(pathName,fp2);  // writing to the file we opened
		   	fputs("\n",fp2);
			count++;
			current = current->next;
			delete pathName;
		}
	}
	if(closeFlag == 1){
		fclose(fp2);
	}	
	pthread_mutex_unlock(&neighbourListLock);
	
}	
