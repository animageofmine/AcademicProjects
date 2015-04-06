#include "hello.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Hello :: Hello(){
	portNumber = 0;
	hostname = new char[20];
	message= NULL;
}
//parse the hello message
void Hello :: ParsePayload(char *message,int length){
	//printf("\n length = %d",length);
	memcpy(&portNumber,&message[0],2);
	int index;
	for(index = 0;index < length - 2;index++){
		hostname[index] = message[index + 2];
	}
	hostname[index] = '\0';
}
//to create hello message
void Hello :: CreateMessage(int commandLinePort,int length){
	//printf("\n length = %d",length);
	message = new char[length];
	short int temp = 0;
	short int tempCommandLinePort = commandLinePort;
	temp = tempCommandLinePort & 65280;
	temp = temp >> 8;
	message[0] = (char)temp;
	temp = 0;
	temp = tempCommandLinePort & 255;
	message[1] = (char)temp;
	for(int index = 2;index < length;index++){
		message[index] = hostname[index - 2];
	}

}

