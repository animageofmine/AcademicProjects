#include "message_header.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int myPortNumber;
extern char *GetUOID(const unsigned char *,char *, char *,int);

extern char *myIpAddress;
extern int commandLinePort;
void MessageHeader :: ParseHeader(char *message){
	messageType = ((int)message[0] & 255);
	for(int index = 0;index < 20 ; index++){
		UOID[index] = message[index + 1];
	}
	TTL = ((int)message[21] & 255);
	memcpy(&dataLength,&message[23],4);
}

MessageHeader :: MessageHeader(){
	header = NULL;
	messageType = 0;
	UOID = new char[20];
	TTL = 0;
	dataLength = 0;
	header = new char[27];
	for(int index = 0;index<27;index++){
		header[index] = '0';
	}
}

void MessageHeader :: CreateHeader(){
	header[0] = char(messageType);
	//printf("\n message type = %d",messageType);
	//printf("\n header[0] = %d",(int)header[0]);


	char *node_instance_id,*object_type;
	char uoid_buf[20];
	//char *return_value;
	int i = 0,j = 0;
	i = j;
	//return_value=(char *)malloc(20*sizeof(char));
	node_instance_id=(char *)malloc(30*sizeof(char));
	object_type=(char *)malloc(10*sizeof(char));

	//strcpy(node_instance_id,"nunki.usc.edu:13176");
	//strcpy(object_type,"hello");
	sprintf (node_instance_id,"%s_%d", myIpAddress,myPortNumber);
	sprintf(object_type,"%d",messageType);


	UOID=GetUOID((const unsigned char *)node_instance_id,object_type,uoid_buf,20);

	
	

		//strcpy(UOID,uoid_buf);
	for(int index = 0;index <20;index ++){
		UOID[index] = uoid_buf[index];
		//headerIndex++;
	}
	
	int headerIndex = 1;
	for(int index = 0;index <20;index ++){
		header[headerIndex] = UOID[index];
		headerIndex++;
	}
	header[21] = char(TTL);
	header[22] = 'R';
	int tempDataLength = dataLength;
	tempDataLength = tempDataLength >> 24;
	tempDataLength = tempDataLength & 255;
	header[23] = (char)tempDataLength;
	tempDataLength = dataLength;
	tempDataLength = tempDataLength >> 16;
	tempDataLength = tempDataLength & 255;
	header[24] = (char)tempDataLength;
	tempDataLength = dataLength;
	tempDataLength = tempDataLength >> 8;
	tempDataLength = tempDataLength & 255;
	header[25] = (char)tempDataLength;
	tempDataLength = dataLength;
	tempDataLength = tempDataLength & 255;
	header[26] = (char)tempDataLength;
	free(node_instance_id);
	free(object_type);
	free(uoid_buf);
}


unsigned char *MessageHeader :: GetID(){
	unsigned char *temp = new unsigned char[4];
	for(int index = 0;index < 4;index ++){
		temp[index] = UOID[index + 16];
	}
	return temp;
}

