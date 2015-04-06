#include <iostream.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <math.h>
#include <algorithm>
#include<openssl/sha.h>

//INCLUDE ALL THE USER DEFINED HEADER FILES
#include "message_header.h"
#include "hello.h"
#include "file.h"
#include "socket_list.h"
#include "timer.h"
#include "iniparser.h"
#include "iniparser.c"
#include "linklist_beacons.h"
#include "message_cache.h"
#include <time.h>
#include<sys/ddi.h>
#include "neighbour_list.h"


using namespace std;


class Beacon{//stoes information about the beacons
		int portNumber;
		int location;
		char *hostname;
	public:
		Beacon();
		int GenerateLocation(void);
};

struct hostLocation{//stores information about the location of the host
	int portNumber;
	char *hostname;
	int flag;
	int sockFd;
	
};

//GLOBAL VARIABLES FOR INIPARSER
char ***ini_parser_keys,no_of_beacon_nodes;
int parse_ini();
int portno_of_ini_file;
char *filename;
int alarmInt = 0;

//NEIGHBOR LIST VARIABLES
char ***store_neighbors;
void init_neighbour(char *);

FILE *fp;
int tcpSock;

//enum type to assign numbers to the respective types of messages
enum MyEnumType {   STATUS_RSP = 171,STATUS = 172,DELETE = 188,STORE = 204,GET_RSP = 219,
					GET = 220,SEARCH_RSP = 235,SEARCH = 236,CHECK_RSP = 245,CHECK = 246,
					NOTIFY = 247,KEEPALIVE = 248, HELLO = 250,JOIN_RSP = 251,JOIN = 252};

int myPortNumber;
void StatusForwardMessage(char *,char *,int);

//catches interrupt when we press Cntrl^C
void catch_int(int sig_num)
{
	printf("\nservant:%d>",myPortNumber);
	//printf("\n Shutting Down\n");
	//PrintSocket();
	Cache_Delete();
	//fclose(fp);
	signal(SIGINT, catch_int);
	

}
char * CurrentTime(void);
void *InputThread(void *);
struct SocketList *socketHead;
struct MessageCache *cacheHead;
struct Neighbour *neighbourHead;
int dontReset;
void NonBeacon(void);
char *myIpAddress;
int commandLinePort;
char *GetUOID(const unsigned char *,char *, char *,int);
//void GetUOID(char*,char*,unsigned char*,int);
pthread_mutex_t fileLock;
pthread_mutex_t socketListLock;
pthread_mutex_t MessageCacheLock;
pthread_mutex_t neighbourListLock;
pthread_mutex_t countLock;
int SendMessage(int,char *,char *,int);
double startTime;
double GetTimeOfDay();
int totalNeighbours;
int node[10];
//for(int index = 0;index< 10;index ++){
//	node[index] = -1;
//}
int links[50];
int linkIndex;
int nodeIndex;

int shutFlag = 0;
pthread_t inputThread;

//function called when alarm times goes 0
void catch_alarm_int(int sig_num){
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
				int flag = 0;
				while(1){
					
					
					//printf("\n UOID = '");
					//for(int index = 0;index <20;index ++){
					//	printf("%02x",UOID[index]);
					//}
					//printf("'");

					FDATA *writeData;//writes data to LOG FILE
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
					writeData->WriteToFile();
					//printf("\n sending notify");
					SendMessage(sock,header->header,"1",1);
					delete writeData;
					delete header;
					if(flag == 1){
						break;
					}
				}
			}
			
			PrintSocket();

		   
		    if(alarmInt == 1){
		    printf("\n Node Shutting. Please Send Requests Later");
		    pthread_cancel(inputThread);
		    shutFlag = 1;
		    close(tcpSock);
		    //set the alarm flag
		    //intFlag = 1;
		    fclose(fp);
		    exit(1);
		    }
		    //alarmInt = 0;

}


void *catch_alarm(void *delayArg){
    int *delay = (int *)delayArg;
    //signal(SIGALRM, catch_alarm_int);
    sleep(*delay);
    alarmInt = 1;
    catch_alarm_int(0);
    pthread_exit(NULL);
    return (void *)0;
}


//some more declarations of functions and variables
void* ResetAll(void*);
int join;
void *ReceiveThread(void *);
char myHostname[128];
void *BeaconNode(void *);//function called when the node is beacon node
void *NonBeaconReconnect(void *);//function called when the node is non-beacon
void SendHello(int,struct hostLocation *,int);//sends hello 
struct timeval tv;
void ParseHello(int,char *,int,struct hostLocation *,int);
int ReceiveData(int,struct hostLocation *,int,int);//receives all kinds of data depending on its type
void BindTcpSocket();//function to bind socket with address
char *GetHostIpAddress(char*);//gets host IP Address
int numberOfBeacons;
//hostLocation *beacon[4];
pthread_t timer;
int goDown = 0;
hostLocation *beacon[10];
char *HomeDir,*LogFilename;
int TTL = 30;
unsigned int Location;
int initFilePresent = 0;
int beaconNode= 0;
int JoinTimeout = 15;
int InitNeighbours = 3;
int MsgLifetime = 30;
//int JoinTimeout;
int MinNeighbours = 2;
int connectCount=0;
int AutoShutdown = 900;
int NoCheck = 0;
int Retry = 30;
char *logFilePath;
int KeepAliveTimeout = 60;



//MAIN STARTS HERE
int main(int argc , char *argv[]){
	int commandLineReset = 1;
	filename=new char[20];
	if(argc == 2){
		commandLineReset = 0;
		strcpy(filename,argv[1]);
		//printf("\n filename = '%s'",filename);
		if(argv[1][0] == '-'){
			printf("\n Malformed Command Line Agruements. Please Enter again"); 
			exit(0);
		}
	}
	
	if(argc == 3){
		//remove(
		//printf("\n 3 argeuments");
		remove("beacons_stored");
		strcpy(filename,argv[2]);
	}
	signal(SIGPIPE, SIG_IGN);
	HomeDir = new char[50];
	LogFilename = new char[50];
	strcpy(LogFilename,"servant.log");
	no_of_beacon_nodes=parse_ini();//parse INI FILE HERE
	
	//printf("\n aftre");//cout<<"My Port no: "<<portno_of_ini_file;
	
	//init_neighbour("init_neighbor_list");

	//int shutdownDelay = 1030;
	
	
	//STORE ALL THE VALUES OF INI FILE INTO THE GLOBAL VARIABLES
	for(int xyz=0;xyz<17;xyz++)
	{
		if(strcmp("TTL",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
				TTL = 30;
			}else{
				TTL = atoi(ini_parser_keys[xyz][1]);
			}
			//printf("\n TTL = %d",TTL);
		}
		if(strcmp("KeepAliveTimeout",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				KeepAliveTimeout = atoi(ini_parser_keys[xyz][1]);
			}
			//printf("\n KeepAliveTimeout= %d",KeepAliveTimeout);
			
		}

		if(strcmp("AutoShutdown",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				AutoShutdown = atoi(ini_parser_keys[xyz][1]);
			}
			//printf("\n AutoShutdown= %d",AutoShutdown);
			
		}
		if(strcmp("NoCheck",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				NoCheck = atoi(ini_parser_keys[xyz][1]);
			}

			//printf("\n NoCheck= %d",NoCheck);
		}

		if(strcmp("Location",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				Location = atoi(ini_parser_keys[xyz][1]);
			}
			//printf("\n Location= %d",Location);
			
		}
		if(strcmp("HomeDir",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				strcpy(HomeDir,ini_parser_keys[xyz][1]);
			}

			
			//printf("\n home dir = %s",HomeDir);
		}
		if(strcmp("LogFilename",ini_parser_keys[xyz][0])==0){
			
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
				strcpy(LogFilename,"servant.log");
			}else{
				strcpy(LogFilename,ini_parser_keys[xyz][0]);
			}
		}          
		if(strcmp("JoinTimeout",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				JoinTimeout = atoi(ini_parser_keys[xyz][1]);
			}

			//printf("\n JoinTimeout= %d",JoinTimeout);
		}
		if(strcmp("InitNeighbors",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				InitNeighbours = atoi(ini_parser_keys[xyz][1]);
			}
			//printf("\n JInitNeighbours= %d",InitNeighbours);
			
		}
		if(strcmp("MinNeighbors",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				MinNeighbours = atoi(ini_parser_keys[xyz][1]);
			}
			//printf("\n MinNeighbours= %d",MinNeighbours);
			
		}
		if(strcmp("MsgLifetime",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				MsgLifetime = atoi(ini_parser_keys[xyz][1]);
			}

			//printf("\n MsgLifetime= %d",MsgLifetime);
		}
		if(strcmp("Retry",ini_parser_keys[xyz][0])==0){
			if(strcmp(ini_parser_keys[xyz][1],"null")==0){
			}else{
				Retry = atoi(ini_parser_keys[xyz][1]);
			}
			//printf("\n Retry= %d",Retry);
			
		}
	}
	
	pthread_t shutThread;
	//create a pthread to process alarm
	pthread_create(&shutThread, NULL, catch_alarm, (void *)&AutoShutdown);
	
	
	//printf("\n msg lifetime = %d",MsgLifetime);
	//printf("\n Location = %d",Location);
	logFilePath = new char[100];
	//sprintf(logFilePath,"%s/%s",HomeDir,LogFilename);
	//strcat(logFilePath,HomeDir);
	//strcat(logFilePath,LogFileName);
	//exit(0);
   	memcpy(&logFilePath[0],&HomeDir[1],strlen(HomeDir)-1);
   	if(commandLineReset == 1){
   		char *tempString = new char[50];
   		strcpy(tempString,logFilePath);
   		strcat(tempString,"/init_neighbor_list");
   		delete tempString;
   		//remove(logFilePath
   	}
   	
  	
   	//logFilePath[strlen(HomeDir)-1]='\0';
   	//printf("\n log fiel name = %d",LogFilename);
   	strcat(logFilePath,"/");
 	if(argc == 3){
		char *temp = new char[50];
		strcpy(temp,logFilePath);
		strcat(temp,"init_neighbor_list");
		remove(temp);
		//printf("\n temp = %s", temp);
		delete temp;	
	}

   	strcat(logFilePath,LogFilename);
	//printf("\n log file path = %s",logFilePath);
	socketHead = NULL;
	signal(SIGINT, catch_int);
	startTime = GetTimeOfDay();
	
	//INITIALIZE MUTEX FOR DIFFERENT KINDS OF LOCKS USED IN PROGRAM
	pthread_mutex_init(&countLock, NULL);
	pthread_mutex_init(&fileLock, NULL);
	pthread_mutex_init(&socketListLock,NULL);
	pthread_mutex_init(&MessageCacheLock,NULL);
	pthread_mutex_init(&neighbourListLock,NULL);
	pthread_mutex_lock (&fileLock);

 	if(argc == 3){
		remove(logFilePath);
	}

   	if ((fp = fopen(logFilePath, "w+")) == NULL) {
   	    printf("\nCouldn't open datafile for reading\n");
   	    exit(1);
   	}else{
 		fclose(fp);
 		//printf("\n log file name = %s",logFilePath);
   		//printf("\n open successfull");
   	}
   	
	//STORE ALL THE BEACON INFORMATION INTO BEACON STRUCTURE
    pthread_mutex_unlock(&fileLock);
	for(int index = 0;index<no_of_beacon_nodes;index++){
		beacon[index] = new hostLocation;
		beacon[index]->portNumber=atoi(return_portno_of_beacon(index));
		beacon[index]->hostname=new char[20];
		beacon[index]->hostname=return_hostname_of_beacon(index);
		beacon[index]->flag=0;
		beacon[index]->sockFd = 0;
	}




	gethostname(myHostname, sizeof(myHostname));
	myIpAddress = GetHostIpAddress(myHostname);
	myPortNumber = portno_of_ini_file;
	struct hostLocation *commandNode = new hostLocation;
	commandNode->portNumber = myPortNumber;
	commandNode->hostname = new char[20];
	sprintf(commandNode->hostname,"%s",myHostname);
	commandNode->flag=0;
	commandNode->sockFd = 0;
	
	//pthread_create(&inputThread, NULL,InputThread , (void *)commandNode);
	pthread_t *threads[3];
	int i=0;
	//cout<<endl;
	
	//DETECT WHETHER THE NODE IS BEACON OR NON BEACON
	for(int index = 0; index < no_of_beacon_nodes;index++){
		if(portno_of_ini_file == beacon[index]->portNumber){
			beaconNode = 1;//IT MEANS THE NODE IS BEACON
			join = 1;
			//printf("\n beacon");
			//exit(0);
		}
	}
	if(beaconNode == 0){
		//printf("\n non beacon");
	}

	if(beaconNode ==1){//IF IT IS A BEACON NODE...
		for(int index=0;index < no_of_beacon_nodes;index++){
			//printf("\t\t portno = %d\n",beacon[index]->portNumber);
		 	if(portno_of_ini_file > beacon[index]->portNumber){
		 		//cout<<"My port no is larger than "<<beacon[index]->portNumber<<endl;
		 		threads[i] = new pthread_t;
		 		pthread_create(threads[i], NULL, BeaconNode, (void *)beacon[index]);
				i++;
			}
		}
		pthread_create(&timer, NULL, Timer, (void *)0);
	}
	FILE *initFile;
	char *directory = new char[50];
   	memcpy(&directory[0],&HomeDir[1],strlen(HomeDir)-1);
   	directory[strlen(HomeDir)-1]='\0';
   	strcat(directory,"/init_neighbor_list");
	int dontResetAgain = 0;
	int connectReturn = 0;
	while(1){
		int fileChecked = 0;
		if(beaconNode == 0){//IF IT IS NOT A BEACON NODE..
	   		if ((initFile = fopen(directory, "r")) == NULL) {
   			   // printf("\nCouldn't open datafile for reading\n");
   	   		}else{
   	   			dontResetAgain = 0;
				totalNeighbours = 0;
				initFilePresent = 1;
				//printf("\n open successful");
				//printf("\n before ");
				init_neighbour(directory);
				//printf("\n after 1");
				if(totalNeighbours < MinNeighbours){
					dontResetAgain = 1;
					//pthread_mutex_unlock (&countLock);
					if(fileChecked != 1){
						fileChecked = 1;
						printf("\n Not Enough Neighbours");
						pthread_t reset;
						pthread_create(&reset, NULL, ResetAll, (void *)0);
					}else{
						remove(directory);
						alarmInt = 1;
						catch_alarm_int(1);
						exit(0);
					}
				}else{//IF IT IS NOT A BEACON NODE
					int beaconTcpSock;
					//hostLocation *dest;
					hostLocation *dest[totalNeighbours]; 
					int connected = 0;
					for(int i=0;i< totalNeighbours;i ++){
						//printf("\n hostname = %s",store_neighbors[i][0]);
						//printf("\n port number = %s",store_neighbors[i][1]);
						char *ipAddress = GetHostIpAddress(store_neighbors[i][0]);
						struct sockaddr_in serverAddr;
						serverAddr.sin_family = AF_INET;         // host byte order
						serverAddr.sin_port = htons(atoi(store_neighbors[i][1]));     // short, network byte order
						serverAddr.sin_addr.s_addr = inet_addr(ipAddress);
					    //node_id = new char[25];
						
						memset(&(serverAddr.sin_zero), '\0', 8); // zero the rest of the struct
						//int connectReturn = 0;
						beaconTcpSock = socket(PF_INET, SOCK_STREAM, 0);

						if(beaconTcpSock == -1){
						printf("\n Error creating socket\n");
							exit(0);
						}
						connectReturn = connect(beaconTcpSock,(struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
						if(connectReturn != -1){
							connectCount++;
							dest[connected] = new hostLocation;
							dest[connected]->portNumber=atoi(store_neighbors[i][1]);
							dest[connected]->hostname=new char[20];
							dest[connected]->hostname=store_neighbors[i][0];
							dest[connected]->flag=0;
							dest[connected]->sockFd = beaconTcpSock;
							connected++;
						}

						
						//pthread_create(thread, NULL, NonBeaconReconnect, (void *)dest);
						delete ipAddress;
					}
					
					
					if(connectCount >= MinNeighbours){
						for(int i=0;i< connected;i ++){
							pthread_t *thread;
							thread = new pthread_t;
							pthread_create(thread, NULL, NonBeaconReconnect, (void *)dest[i]);
						}
					}else{
						if(fileChecked != 1){
							fileChecked = 1;
							printf("\n alarm int");
							//catch_alarm_int(0);
							//printf("\n Not Enough Neighbours");
							pthread_t reset;
							pthread_create(&reset, NULL, ResetAll, (void *)0);
						}else{
							remove(directory);
							alarmInt = 1;
							catch_alarm_int(0);
							exit(0);
						}
					}
					
				}			
			}
		}
		if(beaconNode == 0 && initFilePresent == 0){
			fileChecked  = 1;
			NonBeacon();
			//sleep(JoinTimeout - 1);
		}
		if(initFilePresent == 1 && dontResetAgain == 0){
			pthread_create(&timer, NULL, Timer, (void *)0);
			//usleep(20000);
			pthread_mutex_lock (&countLock);
			if(connectCount < MinNeighbours){
				pthread_mutex_unlock (&countLock);
				goDown = 1;
				//alarmInt = 1;
				//printf("\n alarm int");
				//catch_alarm_int(0);
				//printf("\n Not Enough Neighbours");
				//pthread_t reset;
				//pthread_create(&reset, NULL, ResetAll, (void *)0);
			}
			pthread_mutex_unlock (&countLock);
		}
		//IF THE NODE IS A BEACON NODE AND INIT FILE IS ALREADY PRESENT
		if(beaconNode == 1 || initFilePresent == 1){
			pthread_create(&inputThread, NULL,InputThread , (void *)commandNode);
			initFilePresent = 1;
			tcpSock = socket(PF_INET, SOCK_STREAM, 0);
			if(tcpSock == -1){
				printf("\n Error Creating Socket. Please try again");
				exit(0);
			}
			int reuseAddr = 1;
			if(setsockopt(tcpSock,SOL_SOCKET,SO_REUSEADDR,(void*)(&reuseAddr),sizeof(int)) == -1) {
			     printf("\n Reuse of Socket address failed");
			     exit(0);
		    }
		    //printf("\n before bind");
			BindTcpSocket();
	   		//printf("\n before bind");
			int listenReturn = listen(tcpSock,5);
			if(listenReturn == -1){
				//printf("\n Listen failed\n");
				exit(0);
			}
			struct sockaddr_in clientAddr;
			socklen_t namelen;
			namelen = (sizeof(struct sockaddr_in));
			int childSd;

			//printf("\n In acccept");
			//while loop to accept the connections witht he clients
			while(1){
			   	childSd = accept(tcpSock, (struct sockaddr *) &clientAddr,&namelen);
				if(childSd>0){
					//printf("\n in accep loop");
					//printf("\n before add socket");
					AddSocket(childSd,NULL,0);
				    //printf("\n after add socket");
				    hostLocation *nodeData = new hostLocation;
				    nodeData->hostname = new char[20];
				    if(nodeData->hostname == NULL){
				    	//printf("\n Malloc Failed");
			    		exit(0);
				    }
				    nodeData->portNumber = 0;
				    nodeData->sockFd = childSd;
					pthread_t *receive = new pthread_t;
					pthread_create(receive, NULL, ReceiveThread, (void *)nodeData);
				}
			}
		}
	}
}	



void *ReceiveThread(void *node){
	hostLocation *nodeLocation;
	nodeLocation = (struct hostLocation *)node;

	int type = ReceiveData(nodeLocation->sockFd,nodeLocation,0,1);
					
	UpdateSocket(nodeLocation->sockFd,nodeLocation->hostname,nodeLocation->portNumber);
	
	if(type == HELLO){//IF MESSAGE TYPE IS HELLO
		
		UpdateTimer(nodeLocation->sockFd,1);
		SendHello(nodeLocation->sockFd,nodeLocation,1);
		pthread_t *thread;
		thread = new pthread_t;
		//printf("\n connection established");
		nodeLocation->flag = 1;
		//nodeLocation->sockFd = 	;
		pthread_create(thread, NULL, BeaconNode, (void *)nodeLocation);
	}
	return (void *)0;
}

void NonBeacon(){//IF THE THE NODE IS A NON-BEACON NODE
	dontReset = 0;
	join = 0;
	int sock,connectReturn;
	struct sockaddr_in *beaconAddr = NULL;
	//printf("\n line 355");
	//printf("\n no_of_beacon_nodes = %d",no_of_beacon_nodes);
	int receiveReturn = 0;
	for(int index = 0;index<no_of_beacon_nodes;index++){
		sock = socket(PF_INET, SOCK_STREAM, 0);
		if(sock == -1){
			printf("\n Error Creating Socket.");
			exit(0);
		}
		beaconAddr = new sockaddr_in;
		char *ipAddress;
		ipAddress = GetHostIpAddress(beacon[index]->hostname);
		beaconAddr->sin_addr.s_addr = inet_addr(ipAddress);
	   	beaconAddr->sin_family = AF_INET;         // host byte order
	   	beaconAddr->sin_port = htons(beacon[index]->portNumber);     // short, network byte order
	   	beaconAddr->sin_addr.s_addr = inet_addr(ipAddress);
	    memset(&(beaconAddr->sin_zero), '\0', 8); // zero the rest of the struct
		connectReturn = 0;
		
	    tv.tv_sec = JoinTimeout;
	    tv.tv_usec = 0;

		connectReturn = connect(sock,(struct sockaddr *)beaconAddr, sizeof(struct sockaddr));
		if(connectReturn != -1){
			//printf("\n Connection Successful in non beacon");
			MessageHeader header;
			header.messageType = 252; //code for KEEPALIVE
			header.TTL = TTL;
			header.dataLength = 2 + 4 + strlen(myHostname);
			header.CreateHeader();
			//printf("\n header = %s",header.header);
			char *payload = new char[2 + 4+ sizeof(myHostname) + 1];
			//printf("\n portnumber  = %d",myPortNumber);
			//printf("\n hostname  = %s",myHostname);
			short int tempPort = (short int)myPortNumber;
			memcpy(&payload[0],&Location,4);
			memcpy(&payload[4],&tempPort,2);
			//printf("\n send to port number  = %d",beacon[index]->portNumber);
			memcpy(&payload[6],&myHostname[0],strlen(myHostname));
			//printf("\n payload = %s",payload);
			AddSocket(sock,myHostname,myPortNumber);
			int payloadLength = 4 + 2 + strlen(myHostname);
			//printf("\n sending join request in non beacon");


			FDATA *writeData = new FDATA;//WRITE TO THE LOG FILE
			writeData->logType = 's';
			for(int index1 = 0;index1<4;index1 ++){
				writeData->UOID[index1] = header.header[17 + index1];
			}
			sprintf(writeData->target,"%s %d",beacon[index]->hostname,beacon[index]->portNumber);
			
			writeData->msgType = "JNRQ";
			writeData->dataLength = header.dataLength ;
			writeData->TTL = header.TTL;
			sprintf(writeData->data,"%d %s",myPortNumber,myHostname);
			//delete tempHost;
			writeData->time = GetTimeOfDay();
			writeData->WriteToFile();



			SendMessage(sock,header.header,payload,payloadLength);//SEND MESSAGE FOR RESPECTIVE MESSAGE TYPE
			//printf("\n payload = %s",payload);
			//int bytesReceived = 0;
			double currentTime;
			hostLocation *nodeData = new hostLocation;
			nodeData->hostname = new char[20];
			sprintf(nodeData->hostname,"%s",beacon[index]->hostname);
			nodeData->portNumber = beacon[index]->portNumber;
			int checkTime;
			startTime = GetTimeOfDay();
			double tempStart = startTime;
			while(1){
			    //msgReceived = new char[200];
			   // printf("\n in receive data");
			    //select(sock+1, &readfds, NULL, NULL, &tv);
			   // printf("\n after receiva data");
				receiveReturn = ReceiveData(sock,nodeData,1,0);
				if(receiveReturn == HELLO){
				}
				if(receiveReturn == 10){
					break;
				}
				currentTime = GetTimeOfDay();
				checkTime = (int)((currentTime - tempStart)/(double)1000);
				
				//printf("\n check time = %d",checkTime);
				//printf("\n tv.tv_sec = %d",tv.tv_sec);
				tv.tv_sec = tv.tv_sec - (int)((currentTime - tempStart)/(double)1000);
				//delete msgReceived;
				//printf("\n tv.tv_sec = %d",tv.tv_sec);
				tempStart = currentTime;
				if(tv.tv_sec>0){
					
				}
				if(tv.tv_sec<0){
					break;
				}
			}
			

		}else{
			//printf("\n connect failed");
			if(index == no_of_beacon_nodes - 1){
				//pthread_t reset;
				//pthread_create(&reset, NULL, ResetAll, (void *)0);
				
				//printf("\n No beacons available");
				//pthread_exit(NULL);
				//exit(0);
			}
			close(sock);
		}
		if(tv.tv_sec<0){
			break;
		}
		
		if(receiveReturn == 10){
			//printf("\n exiting non beacon");
			//fclose(fp);
			break;
		}
		
	}
	struct SocketList *delNode = FindSocket(sock);
	if(delNode != NULL){
		//printf("\n 11");
		SocketList_Delete(delNode);
	}
	close(sock);
	PrintNeighbour();
	//printf("after print neighbor\n");
	//exit(1);
}

//IF THE NODE IS NON-BEACON, IT CONNECTS TO THE NEIGHBOR AFTER THE JOIN MESSAGE
void *NonBeaconReconnect(void *hostDetails){
	join = 1;
	hostLocation *nodeLocation;
	nodeLocation = (struct hostLocation *)hostDetails;
	//int beaconTcpSock = nodeLocation->sockFd;




	hostLocation *nodeData = NULL;
	/*char *ipAddress = GetHostIpAddress(nodeLocation->hostname);
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;         // host byte order
	serverAddr.sin_port = htons(nodeLocation->portNumber);     // short, network byte order
	serverAddr.sin_addr.s_addr = inet_addr(ipAddress);
	    //node_id = new char[25];
	int connectFlag = 0;
	memset(&(serverAddr.sin_zero), '\0', 8); // zero the rest of the struct
	int connectReturn = 0;
	beaconTcpSock = socket(PF_INET, SOCK_STREAM, 0);

	if(beaconTcpSock == -1){
		printf("\n Error creating socket\n");
		exit(0);
	}
	connectReturn = connect(beaconTcpSock,(struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
	if(connectReturn != -1){
		connectCount++;
	}*/
	//usleep(200000);
	//if(goDown == 1){
		//pthread_exit(NULL);
	//}
	//int connectFlag = 0;
	//if(connectReturn != -1){
		//printf("\n non beacon reconnect");
		//AddSocket(beaconTcpSock);
		nodeData = new hostLocation;
		nodeData->hostname = new char[20];
		sprintf(nodeData->hostname,"%s",nodeLocation->hostname);
		nodeData->hostname[strlen(nodeLocation->hostname)] = '\0';
		nodeData->portNumber = nodeLocation->portNumber;
		//printf("\n add");
		AddSocket(nodeLocation->sockFd,nodeData->hostname,nodeData->portNumber);
		UpdateTimer(nodeLocation->sockFd,1);
		//printf("\n after add");
		SendHello(nodeLocation->sockFd,nodeData,0);
		//delete nodeData;
		//nodeData = new hostLocation;
		//nodeData->hostname = new char[20];

		ReceiveData(nodeLocation->sockFd,nodeData,1,0);
		//printf("\n before");
		//pthread_mutex_lock (&countLock);
		//connectCount++;
		//pthread_mutex_unlock (&countLock);
		//delete nodeData;
		
	//}else{
		//printf("\n Could Not Connect");
		//connectFlag = 1;
		//close(hostLocation->sockFd);
		//exit(0);
	//}
	

		pthread_t self = pthread_self();
		//printf("\n address of self = %d",&self);
		Pthread_Update(nodeLocation->sockFd,self);
		//PrintSocket();
		int receiveReturn;
		while(1){
			receiveReturn = ReceiveData(nodeLocation->sockFd,nodeLocation,0,0);
			if(receiveReturn == 10){
				//printf("\n exiting non beacon");
				fclose(fp);
				break;
			}
			
		}
	
	close(nodeLocation->sockFd);
	//printf("\n exiting in reconnect");
	pthread_exit(NULL);
	return (void *)0;
}

//IF THE NODE IS A BEACON NODE, THEN IT CALL THIS FUNCTION
void *BeaconNode(void *hostDetails){
	join = 1;
	
	//PROCEDURE FOR SOCKET CONNECTION
	//printf("\n beacon ");
	hostLocation *nodeLocation;
	nodeLocation = (struct hostLocation *)hostDetails;
	int beaconTcpSock = nodeLocation->sockFd;
	hostLocation *nodeData = NULL;
	//signal(SIGINT, catch_int);
	if(nodeLocation->flag == 0){
		//printf("\n -----Initiate Connection--------");
		struct sockaddr_in serverAddr;
		//connect to the server port
	   	serverAddr.sin_family = AF_INET;         // host byte order
	   	serverAddr.sin_port = htons(nodeLocation->portNumber);     // short, network byte order
	   	serverAddr.sin_addr.s_addr = inet_addr(myIpAddress);
	    //node_id = new char[25];

	    memset(&(serverAddr.sin_zero), '\0', 8); // zero the rest of the struct
		int connectReturn = 0;
		while(1){
			beaconTcpSock = socket(PF_INET, SOCK_STREAM, 0);

			if(beaconTcpSock == -1){
				printf("\n Error creating socket\n");
				exit(0);
			}
			connectReturn = connect(beaconTcpSock,(struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
			if(connectReturn != -1){
				//printf("\n beacon node hello");
				//AddSocket(beaconTcpSock);
				nodeData = new hostLocation;
				nodeData->hostname = new char[20];
				strcpy(nodeData->hostname,nodeLocation->hostname);
				//nodeData->hostname = "nunki.usc.edu";
				nodeData->portNumber = nodeLocation->portNumber;
				//printf("\n add");
				AddSocket(beaconTcpSock,nodeData->hostname,nodeData->portNumber);//ADD SOCKET TO THE SOCKET LIST
				UpdateTimer(beaconTcpSock,1);
				//printf("\n after add");
				SendHello(beaconTcpSock,nodeData,0);
				delete nodeData;
				nodeData = new hostLocation;
				nodeData->hostname = new char[20];

				ReceiveData(beaconTcpSock,nodeData,1,0);//WAIT TO RECEIVE DATA FROM THE OTHER NODE HAVING BEACONTCPSOCK SOCKET
				//delete nodeData;
				break;
			}else{
				//printf("\n sleeping");
				sleep(Retry);
				close(beaconTcpSock);
			}
		}
		nodeLocation = nodeData;
	}
	pthread_t self = pthread_self();
	//printf("\n address of self = %d",&self);
	Pthread_Update(beaconTcpSock,self);
	//PrintSocket();
	int receiveReturn = 0;
	while(1){
		receiveReturn = ReceiveData(beaconTcpSock,nodeLocation,0,0);
		if(receiveReturn == 10){
			//printf("\n exiting  beacon");
			fclose(fp);
			break;
		}


	}
	//printf("\n exiting");
	pthread_exit(NULL);
	return (void *)0;
}

//FUNCTION CALLED WHEN MESSAGE IS TO BE SENT TO ANOTHER NODE, DEPENDING ON MESSAGE TYPE
int SendMessage(int sock,char *header,char *payload,int payloadLength){
	char *fullMessage = NULL;
	if(payload == NULL){
		payloadLength = 0;
	}
	fullMessage = new char[27 + payloadLength + 1];
	if(fullMessage == NULL){
		printf("\n malloc failed");
		exit(0);
	}
	for(int index = 0;index < 27 ;index++){
		fullMessage[index] = header[index];
	}
	
	int length = 27;
	if(payload != NULL){
		length = 27 + payloadLength;
		for(int index = 27;index < length; index++){
			fullMessage[index] = payload[index - 27];
		}
	}
	SocketList *current = FindSocket(sock);//FIND CURRENT SOCKET
	if(current == NULL){
		
		//printf("\n socket not found in send BOOM");;
	}else{
		//printf("\n lock acquired in  send");
		//pthread_mutex_lock(&socketListLock);

		pthread_mutex_lock(&current->socketLock);
		//printf("\n after lock acquired in  send");
		send(sock,fullMessage,length,0);
		//printf("\n ************bytes sent = %d******************",bytesSent);
		pthread_mutex_unlock(&current->socketLock);
	}
	//pthread_mutex_unlock(&socketListLock);
	//printf("\n lock released in send");
	return 0;
}


//FUNCTION TO RECEIVE DATA ACCORDING TO ITS RESPECTIVE MESSAGE TYPE
int ReceiveData(int childSd,struct hostLocation *nodeData,int breakLoop, int helloType){
	//printf("\n receive data");
	char *msgReceived = NULL;
	msgReceived = new char[100];
	if(msgReceived == NULL){
		printf("\n Malloc Failed");
		exit(0);
	}
	SocketList *current = FindSocket(childSd);
	if(current == NULL){
		//printf("socket not found");
		return 10;
	}
	//printf("\n socket found = %d",current->sockFd);
	int bytesReceived;
	while(1){
			
			//FOR JOIN
			if(join == 0){
			    fd_set readfds;
			    FD_ZERO(&readfds);
			    FD_SET(childSd, &readfds);

				select(childSd+1, &readfds, NULL, NULL, &tv);
				if (FD_ISSET(childSd, &readfds)) {
					bytesReceived = recv(childSd,msgReceived,27,0);
					if(bytesReceived <= 0){
						return 10;
					}
				}else{
					return 10;
				}
			}
			if(join ==1 ){
				bytesReceived = recv(childSd,msgReceived,27,0);
			}
			//printf("\n****************bytesReceived = %d********************",bytesReceived);

			if(bytesReceived <= 0){
				//printf("\n errno = %d",errno);

				//printf("\n childSd = %d and port number = %d",childSd,current->portNumber);
				close(childSd);
				if(beaconNode == 1){
					
					//struct SocketList *current = FindSocket(childSd);
					/*for(int index = 0;index < 4;index++){
						//printf("\n beacon[index]->portNumber = %d",beacon[index]->portNumber);
						//printf("\n
						if(beacon[index]->portNumber < myPortNumber){
							pthread_t thread;
							hostLocation *node = new hostLocation;
							node->flag = 0;
							node->portNumber = nodeData->portNumber;
							node->hostname = NULL;
							node->sockFd = 0;
							sleep(2);
							pthread_create(&thread, NULL, BeaconNode, (void *)node);
						}
					}*/
				}
				SocketList_Delete(current);
				pthread_exit(NULL);
				return 0;
			}
			int type = ((int)msgReceived[0] & 255);
			//printf("\n type = %d",type);
			if(type == HELLO){//IF TYPE IS HELLO
				ParseHello(childSd,msgReceived,bytesReceived,nodeData,helloType);
				//FD_ZERO(&readfds);
				//printf("\n returning from hello");
				return HELLO;
			}

			if(type == KEEPALIVE){//IF TYPE IS KEEPALIVE
				MessageHeader parsedMsg;
				parsedMsg.ParseHeader(msgReceived);
				//FD_ZERO(&readfds);
				//printf("\n KEEPALIVE received");
				FDATA writeData;
				writeData.logType = 'r';
				for(int index = 0;index<4;index ++){
					writeData.UOID[index] = parsedMsg.UOID[16 + index];
				}
				//printf("\n check 1");
				sprintf(writeData.target,"%s %d",nodeData->hostname,nodeData->portNumber);
				writeData.data[0] = '\0';
				writeData.msgType = "KPAV";
				//printf("\n check 1");
				writeData.dataLength = 0;
				//printf("\n parsedMsg.TTL ------>= %d",parsedMsg.TTL);
				writeData.TTL = parsedMsg.TTL;
				writeData.time = GetTimeOfDay();
				writeData.WriteToFile();
				
				ReceivedKeepAlive(childSd);
				//printf("\n check 1");
			}
			if(type == NOTIFY){//IF TYPE OF MESSAGE IS NOTIFY
			
				//printf("\n notify received");
				MessageHeader parsedMsg;
				parsedMsg.ParseHeader(msgReceived);
				char *newPayload = new char[parsedMsg.dataLength];
				//int bytes = recv(childSd,newPayload,parsedMsg.dataLength,0);
				recv(childSd,newPayload,parsedMsg.dataLength,0);
				
				FDATA writeData;
				writeData.logType = 'r';
				for(int index = 0;index<4;index ++){
					writeData.UOID[index] = parsedMsg.UOID[16 + index];
				}
				sprintf(writeData.target,"%s %d",nodeData->hostname,nodeData->portNumber);
				writeData.data[0] = newPayload[0];
				writeData.msgType = "NTFY";
				writeData.dataLength = 1;
				//printf("\n parsedMsg.TTL ------>= %d",parsedMsg.TTL);
				writeData.TTL = parsedMsg.TTL;
				writeData.time = GetTimeOfDay();
				writeData.WriteToFile();
				
				if(beaconNode == 1){
					//struct SocketList *current = FindSocket(childSd);
					for(int index = 0;index < no_of_beacon_nodes;index++){
						//printf("\n beacon[index]->portNumber = %d",beacon[index]->portNumber);
						//printf("\n
						if(beacon[index]->portNumber < myPortNumber){
							pthread_t thread;
							hostLocation *node = new hostLocation;
							node->flag = 0;
							node->portNumber = nodeData->portNumber;
							node->hostname = NULL;
							node->sockFd = 0;
							sleep(2);
							pthread_create(&thread, NULL, BeaconNode, (void *)node);
						}
					}
				}
				//printf("\n 21");
				SocketList *current1 = FindSocket(childSd);
				
				SocketList_Delete(current1);
				//printf("\n 21");
				sleep(2);
				if(beaconNode == 0){
					SendCheck();
					
				}
				//printf("\n after 21");
				//sleep(2);
				//pthread_exit(NULL);
			}
			if(type == JOIN){//IF MESSAGE TYPE IS JOIN
				if(initFilePresent){
					unsigned char *UOID = new unsigned char[20];
					for(int index =0;index < 20;index++){
						UOID[index] = msgReceived[index + 1];
					}				
					//printf("\n 1");
					MessageHeader parsedMsg;
					FDATA *writeData = new FDATA;
					parsedMsg.ParseHeader(msgReceived);
					char *newPayload = new char[parsedMsg.dataLength];
					recv(childSd,newPayload,parsedMsg.dataLength,0);
					//printf("\n *************bytes = %d*************");	
					writeData->logType = 'r';
					for(int index = 0;index<4;index ++){
						writeData->UOID[index] = parsedMsg.UOID[16 + index];
					}
					//printf("\n 2");
					short int portNumber;
					int joinLocation = 0;
					memcpy(&joinLocation,&newPayload[0],4);
					memcpy(&portNumber,&newPayload[4],2);
					char *tempHost = new char[20];
					for(int index = 0;index < parsedMsg.dataLength - 6;index ++){
						tempHost[index] = newPayload[index + 6];
					}
					//printf("\n strlen of temphost = %d",strlen(tempHost));
					//tempHost[parsedMsg.dataLength - 2] = '\0';
					//printf("\n tmephost = %s",tempHost);
					if(nodeData->portNumber > 0){
						sprintf(writeData->target,"%s %d",nodeData->hostname,nodeData->portNumber);
					}
					if(nodeData->portNumber == 0){
						sprintf(nodeData->hostname,"%s",tempHost);
						nodeData->portNumber = portNumber;
						sprintf(writeData->target,"%s %d",tempHost,portNumber);
						UpdateSocket(childSd,tempHost,portNumber);
					}
					//printf("\n 3");
					//WRITE DATA TO LOG FILE
					writeData->msgType = "JNRQ";
					writeData->dataLength = parsedMsg.dataLength;
					writeData->TTL = parsedMsg.TTL;

					sprintf(writeData->data,"%s %d",tempHost,portNumber);
					//delete tempHost;
					writeData->time = GetTimeOfDay();
					writeData->WriteToFile();
					delete writeData;
					int joinReturn = FindUOID(UOID,childSd,0);
					//printf("\n join return = %d",joinReturn);
					if(parsedMsg.TTL > 0){
						if(joinReturn == 0){
							//printf("\n ------------------join received----------------");
							//printf("\n 4");
							////////////////Update forwaard function///////////////
							ForwardMessage(msgReceived,newPayload,bytesReceived,nodeData->portNumber);
							//printf("\n after join forward ");
							MessageHeader *header = new MessageHeader;
							if(header == NULL){
								printf("\n Malloc Failed");
								exit(0);
							}
							header->messageType = JOIN_RSP;
							header->TTL = TTL;
							header->dataLength = 20 + 2 + 4 + strlen(myHostname);
							header->CreateHeader();
							//printf("\n after header");
							short int tempPort = (short int)myPortNumber;
	
							char* payload = new char[100];
							memcpy(&payload[0],&UOID[0],20);
							//printf("\n memcpy");
							int distance = labs(Location - joinLocation);
							memcpy(&payload[20],&distance,4);
							memcpy(&payload[24],&tempPort,2);
							memcpy(&payload[26],&myHostname[0],strlen(myHostname));
							//printf("\n payload = %s",payload);
							//printf("\n sending message in join");
							//FindUOID(header->UOID,childSd,1);
							SendMessage(childSd,header->header,payload,header->dataLength);
							//printf("\n 5");
							writeData = new FDATA;
							writeData->logType = 's';
							for(int index = 0;index<4;index ++){
								writeData->UOID[index] = header->header[16 + index];
							}
							if(nodeData->portNumber > 0){
								sprintf(writeData->target,"%s %d",nodeData->hostname,nodeData->portNumber);
							}
								if(nodeData->portNumber == 0){
								sprintf(writeData->target,"%s %d",tempHost,portNumber);
							}
							writeData->msgType = "JNRS";
							writeData->dataLength = header->dataLength ;
							writeData->TTL = header->TTL;
							for(int index = 0;index<20;index++){
								
							}
							sprintf(writeData->data,"%02x%02x%02x%02x %d %d %s",UOID[16],UOID[17],UOID[18],UOID[19],distance,tempPort,myHostname);
							//delete tempHost;
							writeData->time = GetTimeOfDay();
							//printf("\n 6");
							writeData->WriteToFile();
							delete writeData;
							delete payload;
							delete header;
							
						}else{
							//printf("\n *************message dropped*******************");
						}
					}else{
						//printf("\n *************message dropped*******************");
					}
				
					//delete UOID;
					//return JOIN;
				}
			}	
			if(type == JOIN_RSP){// IF THE MESSAGE IS JOIN RESPONSE
				if(breakLoop == 0){
					//printf("\n*************** join response received***********");
					unsigned char *UOID = new unsigned char[20];
					MessageHeader *parsedMsg  = new MessageHeader;
					parsedMsg->ParseHeader(msgReceived);	
					char *newPayload = new char[parsedMsg->dataLength];
					recv(childSd,newPayload,parsedMsg->dataLength,0);
					//printf("\n*************bytes = %d ********************",bytes); 
					printf("\n 7");
					memcpy(&UOID[0],&newPayload[0],20);
					FindUOID(UOID,childSd,0);
					//if(joinReturn == 1){
					//	printf("\n --------UOID Found---------");
					//}

					int tempLength = parsedMsg->dataLength;
					FDATA *writeData = new FDATA;
					writeData->logType = 'r';
					for(int index = 0;index<4;index ++){
						writeData->UOID[index] = parsedMsg->UOID[16 + index];
					}
				//	printf("\n 7");
					sprintf(writeData->target,"%s %d",nodeData->hostname,nodeData->portNumber);
					writeData->msgType = "JNRS";
					writeData->dataLength = parsedMsg->dataLength;
					writeData->TTL = parsedMsg->TTL;
					short int tempTTL = (short int)parsedMsg->TTL - 1;
					int tempLocation;
					memcpy(&tempLocation,&newPayload[20],4);
					short int tempPort;
					memcpy(&tempPort,&newPayload[24],2);
					char *tempHost = new char[20];
					memcpy(&tempHost[0],&newPayload[26],parsedMsg->dataLength - 26);
					sprintf(writeData->data,"%02x%02x%02x%02x %d %d %s",UOID[16],UOID[17],UOID[18],UOID[19],tempLocation,tempPort,tempHost);
					writeData->time = GetTimeOfDay();
					writeData->WriteToFile();
					delete writeData;
					delete parsedMsg;
					//printf("\n 8");
					int forwardSocket = FindUOIDSocket(UOID);
					//printf("\n forwardSocket = %d",forwardSocket);
					if(forwardSocket == 0){
						//printf("\n UOID not found");
					}else{
						if(tempTTL > 0){
						char *tempHeader = new char[27];
						char *payload = new char[50];
						if(tempTTL > TTL){
							tempTTL = (short int)TTL;
						}
						tempTTL = tempTTL << 8;
						memcpy(&tempHeader[0],&msgReceived[0],27);
						memcpy(&tempHeader[21],&tempTTL,1);
						memcpy(&payload[0],&newPayload[0],tempLength);
						//printf("\n strlen payload = %d",strlen(payload));
						int payloadLength = tempLength;
						//printf("\n payload length = %d----------- and forward socket = %d",payloadLength,forwardSocket);
						srand(time(NULL));

					    //generate secret number: 
					 	int random = rand() % 100 + 1;
						usleep(random);
   						//printf("\n 9");
					   // printf("\n sending join response");
						SendMessage(forwardSocket,tempHeader,payload,payloadLength);
						//MessageHeader *header = new MessageHeader;
						//header->messageType = JOIN_RSP;
						FDATA *write = new FDATA;
						write->logType = 'f';
						for(int index = 0;index<4;index ++){
							write->UOID[index] = msgReceived[17 + index];
						}
						SocketList *forwardNode = FindSocket(forwardSocket); 
						//printf("\n 10");
						sprintf(write->target,"%s %d",forwardNode->hostname,forwardNode->portNumber);
						write->msgType = "JNRS";
						write->dataLength = tempLength;
						tempTTL = tempTTL >> 8;
						tempTTL = tempTTL & 255;
						write->TTL = tempTTL;
						sprintf(write->data,"%02x%02x%02x%02x %d %d %s",UOID[16],UOID[17],UOID[18],UOID[19],tempLocation,tempPort,tempHost);
						write->time = GetTimeOfDay();
						write->WriteToFile();
						delete writeData;
						//delete forwardNode;
						}
					}
				}
				if(breakLoop == 1 && beaconNode == 0){
					MessageHeader *joinHeader = new MessageHeader;
					if(joinHeader == NULL){
						printf("\n Malloc Failed");
						exit(0);
					}
					joinHeader->ParseHeader(msgReceived);
					//printf("\n message type = %d",joinHeader->messageType);
					//printf("\n TTL = %d",joinHeader->TTL);
					//printf("\n data length = %d",joinHeader->dataLength);
					char *newPayload = new char[joinHeader->dataLength];
					recv(childSd,newPayload,joinHeader->dataLength,0);
					//printf("\n **************bytes = %d*****************");
					FDATA *writeJoin = new FDATA;
					writeJoin->logType = 'r';
					writeJoin->msgType = "JNRS";
					writeJoin->TTL = joinHeader->TTL;
					writeJoin->dataLength = joinHeader->dataLength;
					long int tempLocation;
					memcpy(&tempLocation,&newPayload[20],4);
					short int tempPort;
					memcpy(&tempPort,&newPayload[24],2);
					//printf("\n check 3");
					char *tempHost = new char[20];
					if(tempHost == NULL){
						printf("\n Malloc Failed");
						exit(0);
					}
					for(int index =0;index < joinHeader->dataLength - 26;index++){
						tempHost[index] = newPayload[index + 26];
					}
					//memcpy(&tempHost[0],&msgReceived[53],bytesReceived-53);
					unsigned char *UOID = new unsigned char[20];	
					for(int index = 0;index <20;index++){
						UOID[index] = (unsigned char)newPayload[index];
					}
					
					sprintf(writeJoin->data,"%02x%02x%02x%02x %d %d %s",UOID[16],UOID[17],UOID[18],UOID[19],tempLocation,tempPort,tempHost);
					sprintf(writeJoin->target,"%s %d",nodeData->hostname,nodeData->portNumber);
					memcpy(&writeJoin->UOID[0],&msgReceived[17],4);
					//memcpy(&writeJoin->data[0],&msgReceived[27],bytesReceived - 27);
					writeJoin->time = GetTimeOfDay();
					writeJoin->WriteToFile();
					//long int distance = Location - tempLocation;
					if(tempHost == NULL){
						//printf("\n host null");
					}
					AddNeighbour(tempPort,tempHost,tempLocation);
				}
			}
			if(type == STATUS){//IF THE MESSAGE TYPE IS STATUS
			if(initFilePresent){
				//printf("\n *******************status received*******************");
				MessageHeader *parse = new MessageHeader;
				parse->ParseHeader(msgReceived);
				//printf("\n parse->type = %d",parse->messageType);
				//printf("\n parse->TTL = %d",parse->TTL);
				//printf("\n parse->dataLength = %d \n '",parse->dataLength);
				unsigned char *UOID = new unsigned char[20];
				memcpy(&UOID[0],&parse->UOID[0],20);
				//strcpy(UOID,(unsigned char*)parse->UOID);
				//for(int index = 0;index<20;index++){
				//	printf("%02x",UOID[index]);
				//}
				//printf("'");
				char *newPayload = new char[parse->dataLength];
				recv(childSd,newPayload,parse->dataLength,0);
				char data = newPayload[0];
				FDATA *writeToFile = new FDATA;
				writeToFile->logType = 'r';
				writeToFile->msgType = "STRQ";
				writeToFile->TTL = parse->TTL;
				writeToFile->dataLength = parse->dataLength;
				writeToFile->data[0] = data;
				writeToFile->data[1] = '\0';
				sprintf(writeToFile->target,"%s %d",nodeData->hostname,nodeData->portNumber);				
				memcpy(&writeToFile->UOID[0],&msgReceived[17],4);			
				writeToFile->time = GetTimeOfDay();
				writeToFile->WriteToFile();
				short int tempPort = 0;
				int neighborLength = 0;
				int statusType = 0;
				int UOIDReturn = FindUOID(UOID,childSd,0);
				//printf("\n UOID return = %d", UOIDReturn);
				if(UOIDReturn == 0){
					if(parse->TTL> 0){	
						if(data == '1'){
							char *myNeighbors = new char[200];
							pthread_mutex_lock(&socketListLock);
							SocketList *current = socketHead;
							int dataLength = 0;
							while(current != NULL){
								dataLength = 2 + strlen(current->hostname);
								//printf("\n current->hostname = '%s'",current->hostname);
								//printf("\n current->portNumber = %d",current->portNumber);
								//printf("\n datalength = %d",dataLength);
								tempPort = (short int)current->portNumber;
								
								//printf("\n tempPosr = %d",tempPort);
								memcpy(&myNeighbors[neighborLength],&dataLength,4);
								memcpy(&myNeighbors[neighborLength+4],&tempPort,2);
								memcpy(&myNeighbors[neighborLength+6],&current->hostname[0],strlen(current->hostname));
								neighborLength = neighborLength + 2 + 4 + strlen(current->hostname);
								//printf("\n myNeighbors = %s",myNeighbors);
								//printf("\n neighbors = '");
								//for(int index = 0;index < neighborLength;index++){
								//	printf(" %02x",myNeighbors[index]);
								//}
								//printf("'");
								//printf("\n neoighborLength = %d",neighborLength);
								current = current->next;
								statusType = 1;
							}
							int zero = 0;
							memcpy(&myNeighbors[neighborLength],&zero,4);
							pthread_mutex_unlock(&socketListLock);
							//strcat(myNeighbors,"0000");
							neighborLength = neighborLength + 4;
							//sprintf(myNeighbors,"%s0000",myNeighbors);
							
							//CALCULATE THE HEADER AND PAYLOAD LENGTH
							char *payload = new char[250];	
							memcpy(&payload[0],&UOID[0],20);	
							short int hostLength = 2 + strlen(myHostname);
							short int myPort = (short int)myPortNumber;
							//printf("\n my port number = %d",myPortNumber);
							memcpy(&payload[20],&hostLength,2);
							memcpy(&payload[22],&myPort,2);
							memcpy(&payload[24],&myHostname[0],strlen(myHostname));
							memcpy(&payload[24+strlen(myHostname)],&myNeighbors[0],neighborLength);
							//printf("\n");
							//for(int index = 0;index < 24 + (int)strlen(myHostname) + (int)neighborLength;index++){
							//	printf("%02x",(unsigned char)payload[index]);
							//}
							MessageHeader *header = new MessageHeader;
							header->messageType = 171;
							header->TTL = TTL;
							header->dataLength = 24 + strlen(myHostname) + neighborLength;
							header->CreateHeader();
							SendMessage(childSd,header->header,payload,header->dataLength);
							FDATA *write = new FDATA;//WRITE DATA TO LOG FILE
							write->logType = 's';
							write->msgType = "STRS";
							write->TTL =TTL;
							write->dataLength = header->dataLength;
							unsigned char *tempUOID = new unsigned char[20];
							memcpy(&tempUOID[0],&header->UOID[16],20);
							write->time = GetTimeOfDay();
							//sprintf((unsigned char *)write->UOID,"%02x%02x%02x%02x",tempUOID[16],tempUOID[17],tempUOID[18],tempUOID[19]);
							memcpy(&write->UOID[0],&header->UOID[16],4);
							//for(int index = 0;index<4;index ++){
							//	write->UOID[index] = header->UOID[16 + index];
							//}
							//for(int index = 0;index<4;index ++){
							sprintf(write->data,"%02x%02x%02x%02x",UOID[16],UOID[17],UOID[18],UOID[19]);
							//printf("\n header->UOID = %02x",header->UOID[16 + index]);
								//write->data[index] = UOID[16 + index];
							//}
							//write->data[4] = '\0';
							//memcpy(&write->UOID[0],&header->UOID[16],4);
							//write->UOID[4] = '\0';
							//memcpy(&write->data[0],&UOID[16],4);
							//write->data[4] = '\0';
							sprintf(write->target,"%s %d",nodeData->hostname,nodeData->portNumber);
							
							//printf("\n time  = %f",write->time);
							write->WriteToFile();
							if(parse->TTL - 1 >0){	
								StatusForwardMessage(msgReceived,newPayload,nodeData->portNumber);
							}
							delete payload;
							delete header;
							delete myNeighbors;
							delete write;
						}
					}
					delete newPayload;
					delete writeToFile;
					delete parse;
				}
			}
			}
			if(type == 171){
				//int myFlag = 0;
				//printf("\n status received");
				MessageHeader *parseMsg = new MessageHeader;
				parseMsg->ParseHeader(msgReceived);
				char *newPayload = new char[parseMsg->dataLength];
				recv(childSd,newPayload,parseMsg->dataLength,0);
				unsigned char *UOID = new unsigned char[20];
				memcpy(&UOID[0],&newPayload[0],20);
				FDATA *writeToFile = new FDATA;
				writeToFile->logType = 'r';
				writeToFile->msgType = "STRS";
				writeToFile->TTL = parseMsg->TTL;
				writeToFile->dataLength = parseMsg->dataLength;
				sprintf(writeToFile->data,"%02x%02x%02x%02x",UOID[16],UOID[17],UOID[18],UOID[19]);
				//memcpy(&writeToFile->data[0],&UOID[16],4);
				//writeToFile->data[1] = '\0';
				sprintf(writeToFile->target,"%s %d",nodeData->hostname,nodeData->portNumber);				
				memcpy(&writeToFile->UOID[0],&msgReceived[17],4);			
				writeToFile->time = GetTimeOfDay();
				//printf("\n file time = %f",writeToFile->time);
				writeToFile->WriteToFile();
				
				int forwardSocket = FindUOIDSocket(UOID);
				//printf("\n forward socket = %d",forwardSocket);
				if(forwardSocket > 0){
					SocketList * current = FindSocket(forwardSocket);
					//printf("\n current");
					if(current != NULL){
						int my = FindMy(UOID);
						//printf("\n my = %d",my);
						if(my == 0){
							//printf("\n forwarding status response");
							FDATA *writeToFile = new FDATA;
							writeToFile->logType = 'f';
							writeToFile->msgType = "STRS";
							if(parseMsg->TTL - 1 > TTL){
								parseMsg->TTL = TTL;
							}else{
								parseMsg->TTL--;
							}
							//WRITE TO LOG FILE
							writeToFile->TTL = parseMsg->TTL;
							writeToFile->dataLength = parseMsg->dataLength;
							sprintf(writeToFile->data,"%02x%02x%02x%02x",UOID[16],UOID[17],UOID[18],UOID[19]);
							//memcpy(&writeToFile->data[0],&UOID[16],4);
							//writeToFile->data[1] = '\0';
							sprintf(writeToFile->target,"%s %d",nodeData->hostname,nodeData->portNumber);				
							memcpy(&writeToFile->UOID[0],&msgReceived[17],4);			
							writeToFile->time = GetTimeOfDay();
							//printf("\n file time rsp = %f",writeToFile->time);
							writeToFile->WriteToFile();					
							short int tempTTL = (short int)parseMsg->TTL;
							tempTTL = tempTTL << 8;	
							memcpy(&msgReceived[21],&tempTTL,1);
							//int forwardSocket = FindUOIDSocket(UOID);
							SendMessage(forwardSocket,msgReceived,newPayload,parseMsg->dataLength);
						}
						
					}		
				}
				if(forwardSocket == 0){
					//do{
					short int hostLength;
					short int hostPort;
					char *hostName = new char[20];
					memcpy(&hostLength,&newPayload[20],2);
					memcpy(&hostPort,&newPayload[22],2);
					memcpy(&hostName[0],&newPayload[24],hostLength - 2);
					int found = 0;
					for(int index = 0;index < 10;index ++){
						if(node[index] ==hostPort){
							found = 1;
						}
					}
					if(found ==0){
						node[nodeIndex] = hostPort;
						nodeIndex++;
						//sprintf(line,"n -t * -s %d -c red -i black",hostPort);
					}
					//printf("\n Host Length = %d", hostLength);
					//printf("\n host port = %d",hostPort);
					//printf("\n host name = %s",hostName);
					int neighborIndex = 22 + hostLength;
					char *neighborHost = NULL;
					short int neighborPort = 0;
					int neighborLength = 0;
					while(1){
						memcpy(&neighborLength,&newPayload[neighborIndex],4);
						neighborIndex = neighborIndex + 4;
						if(neighborLength > 0){
										
							neighborHost = new char[20];
							memcpy(&neighborPort,&newPayload[neighborIndex],2);
							neighborIndex = neighborIndex + 2;
							memcpy(&neighborHost[0],&newPayload[neighborIndex],neighborLength - 2);
							neighborIndex = neighborIndex + neighborLength - 2;
							//printf("\n neighbor length = %d",neighborLength);
							//printf("\n neighborPort = %d",neighborPort);
							//printf("\n neighbor host = %s",neighborHost);
							delete neighborHost;
							found = 0;
							for(int index = 0;index <10;index++){
								if(node[index] == neighborPort){
									found = 1;
								}
							}
							if(found == 0){
								node[nodeIndex] = neighborPort;
								nodeIndex++;
								//sprintf(line,"n -t * -s %d -c red -i black",neighborPort);
							}
							//sprintf(oneLine,"l -t * -s %d -d %d -c blue",hostPort,neighborPort);
							links[linkIndex++] = hostPort;
							links[linkIndex++] = neighborPort;
						}
						
						if(neighborLength == 0){
							//printf("\n neighbor break");
							break;
						}
					}
				}
				
			}

			if(type == CHECK){//IF THE MESSAGE TYPE IS CHECK
				//printf("\n check""""""""""""""""""""""""""""""""");
				if(initFilePresent){
					//printf("\n check received");
					MessageHeader *parseMsg = new MessageHeader;
					parseMsg->ParseHeader(msgReceived);
					char *newPayload = new char[parseMsg->dataLength];
					recv(childSd,newPayload,parseMsg->dataLength,0);
					unsigned char *UOID = new unsigned char[20];
					memcpy(&UOID[0],&msgReceived[1],20);
					
					FDATA *writeToFile = new FDATA;
					writeToFile->logType = 'r';
					writeToFile->msgType = "CKRQ";
					writeToFile->TTL = parseMsg->TTL;
					writeToFile->dataLength = parseMsg->dataLength;

					//sprintf(writeToFile->data,"%02x%02x%02x%02x",UOID[16],UOID[17],UOID[18],UOID[19]);
					//for(int index = 0;index<4;index++){
					//	writeToFile->data[index] = UOID[index+16];
					//}
					//memcpy(&writeToFile->data[0],&UOID[16],4);
					//writeToFile->data[1] = '\0';
					sprintf(writeToFile->target,"%s %d",nodeData->hostname,nodeData->portNumber);				
					memcpy(&writeToFile->UOID[0],&msgReceived[17],4);			
					writeToFile->time = GetTimeOfDay();

					writeToFile->WriteToFile();		
					int UOIDReturn = FindUOID(UOID,childSd,0);
					//printf("\n UOID found = %d",UOIDReturn);
					if(UOIDReturn == 0){
						if(beaconNode == 0){
							if(parseMsg->TTL > 1){	
								ForwardCheck(msgReceived,childSd);
							}
						}
						if(beaconNode == 1){
								MessageHeader *header = new MessageHeader;
								header->TTL= TTL;
								header->dataLength = 20;
								header->messageType = CHECK_RSP;
								header->CreateHeader();
								FDATA *writeToFile = new FDATA;
								writeToFile->logType = 's';
								writeToFile->msgType = "CKRS";
								writeToFile->TTL = TTL;
								writeToFile->dataLength = header->dataLength;
								sprintf(writeToFile->data,"%02x%02x%02x%02x",UOID[16],UOID[17],UOID[18],UOID[19]);
								
								//for(int index = 0;index<4;index++){
								//	writeToFile->data[index] = UOID[index+16];
								//}
								//memcpy(&writeToFile->data[0],&UOID[16],4);
								//writeToFile->data[1] = '\0';
								sprintf(writeToFile->target,"%s %d",nodeData->hostname,nodeData->portNumber);				
								memcpy(&writeToFile->UOID[0],&header->UOID[16],4);			
								writeToFile->time = GetTimeOfDay();
								writeToFile->WriteToFile();		
								SendMessage(childSd,header->header,(char *)UOID,20);
						}
					}
				}
			}
			if(type == CHECK_RSP){// IF IT IS A CHECK RESPONSE
				//printf("\n check response received");
					
					MessageHeader *parseMsg = new MessageHeader;
					parseMsg->ParseHeader(msgReceived);
	
					char *newPayload = new char[parseMsg->dataLength];
					recv(childSd,newPayload,parseMsg->dataLength,0);
					unsigned char *UOID = new unsigned char[20];
					memcpy(&UOID[0],&newPayload[0],20);
					int uoidFound = FindUOID(UOID,childSd,0);
							FDATA *writeToFile = new FDATA;
							writeToFile->logType = 'r';
							writeToFile->msgType = "CKRS";
							writeToFile->TTL = parseMsg->TTL;
							writeToFile->dataLength = parseMsg->dataLength;
							sprintf(writeToFile->data,"%02x%02x%02x%02x",UOID[16],UOID[17],UOID[18],UOID[19]);
							sprintf(writeToFile->target,"%s %d",nodeData->hostname,nodeData->portNumber);				
							memcpy(&writeToFile->UOID[0],&parseMsg->UOID[16],4);			
							writeToFile->time = GetTimeOfDay();
								//double currentTime = GetTimeOfDay();
								//writeToFile->time = (currentTime - startTime)/(double)1000;
								writeToFile->WriteToFile();	

					if(uoidFound == 1){
						int my = FindMy(UOID);
						if(my == 1){
							//printf("\n mera uoid mil gaya");
							FoundNeighbor(UOID);
					
					
							
						}else{
							if(parseMsg->TTL > 1){							
								int forwardSocket = FindUOIDSocket(UOID);
								
								//short int tempTTL = parseMsg->TTL - 1;
								//tempTTL = tempTTL << 8;
								//memcpy(&msgReceived[21],&tempTTL,1);
								FDATA *writeToFile = new FDATA;
								writeToFile->logType = 'f';
								writeToFile->msgType = "CKRS";
								if(parseMsg->TTL - 1 > TTL){
									parseMsg->TTL = TTL;
								}else{
									parseMsg->TTL--;
								}
								
								//WRITE TO LOG FILE
								writeToFile->TTL = parseMsg->TTL;
								writeToFile->dataLength = parseMsg->dataLength;
								sprintf(writeToFile->data,"%02x%02x%02x%02x",UOID[16],UOID[17],UOID[18],UOID[19]);
								//for(int index = 0;index<4;index++){
								//	writeToFile->data[index] = UOID[index+16];
								//}
								//memcpy(&writeToFile->data[0],&UOID[16],4);
								//writeToFile->data[1] = '\0';
								sprintf(writeToFile->target,"%s %d",nodeData->hostname,nodeData->portNumber);				
								memcpy(&writeToFile->UOID[0],&parseMsg->UOID[16],4);			
								short int tempTTL = (short int)parseMsg->TTL;
								tempTTL = tempTTL << 8;	
								memcpy(&msgReceived[21],&tempTTL,1);

								writeToFile->time = GetTimeOfDay();
								//double currentTime = GetTimeOfDay();
								//writeToFile->time = (currentTime - startTime)/(double)1000;
								writeToFile->WriteToFile();	
								SendMessage(forwardSocket,msgReceived,newPayload,20);
							}	
						}
					}				
				
			}
			
			if(breakLoop == 1){
				break;
			}
	}
	//printf("\n after loop");
	//delete msgReceived;
	return 0;
}

//FUNCTION TO RESET ALL THE FILES, if -RESET OPTION IS GIVEN IN THE COMMAND LINE
void* ResetAll(void *none){
		//alarmInt = 1;
		if(alarmInt == 1){
			pthread_exit(NULL);
		}
		catch_alarm_int(1);
		//sleep(3);
		printf("\n deleting sockets");
		pthread_mutex_lock (&socketListLock);
		//SocketList *current = socketHead;
		//}
		socketHead = NULL;
		//printf("\n all locks deleted");
		pthread_mutex_unlock (&socketListLock);
		pthread_mutex_lock (&MessageCacheLock);
	

		//delete current1;
		
		cacheHead = NULL;
		pthread_mutex_unlock (&MessageCacheLock);

		pthread_mutex_lock (&neighbourListLock);
		neighbourHead = NULL;
		pthread_mutex_unlock (&neighbourListLock);
	
		char *directory = new char[50];
	   	memcpy(&directory[0],&HomeDir[1],strlen(HomeDir)-1);
	   	directory[strlen(HomeDir)-1]='\0';
	   	strcat(directory,"/init_neighbor_list");
		//printf("\n all locks deleted");
		remove(directory);
		initFilePresent = 0;
		join = 0;
		totalNeighbours = 0;
		NonBeacon();
		connectCount = 0;
		goDown = 0;
		FILE *initFile;
   		int dontResetAgain = 0;







		if(beaconNode == 0){//IF THE NODE IS A BEACON NODE
	   		if ((initFile = fopen(directory, "r")) == NULL) {
   			   // printf("\nCouldn't open datafile for reading\n");
   	   		}else{
   	   			dontResetAgain = 0;
				totalNeighbours = 0;
				initFilePresent = 1;
				//printf("\n open successful");
				//printf("\n before ");
				init_neighbour(directory);
				//printf("\n after 1");
				if(totalNeighbours < MinNeighbours){
					dontResetAgain = 1;
					//pthread_mutex_unlock (&countLock);
					printf("\n Not Enough Neighbours");
					pthread_t reset;
					pthread_create(&reset, NULL, ResetAll, (void *)0);
				
				}else{
					int beaconTcpSock;
					//hostLocation *dest;
					hostLocation *dest[totalNeighbours]; 
					int connectReturn;
					int connected = 0;
					for(int i=0;i< totalNeighbours;i ++){
						//printf("\n hostname = %s",store_neighbors[i][0]);
						//printf("\n port number = %s",store_neighbors[i][1]);
						char *ipAddress = GetHostIpAddress(store_neighbors[i][0]);
						struct sockaddr_in serverAddr;
						serverAddr.sin_family = AF_INET;         // host byte order
						serverAddr.sin_port = htons(atoi(store_neighbors[i][1]));     // short, network byte order
						serverAddr.sin_addr.s_addr = inet_addr(ipAddress);
					    //node_id = new char[25];
						
						memset(&(serverAddr.sin_zero), '\0', 8); // zero the rest of the struct
						//int connectReturn = 0;
						beaconTcpSock = socket(PF_INET, SOCK_STREAM, 0);

						if(beaconTcpSock == -1){
						printf("\n Error creating socket\n");
							exit(0);
						}
						connectReturn = connect(beaconTcpSock,(struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
						if(connectReturn != -1){
							connectCount++;
							dest[connected] = new hostLocation;
							dest[connected]->portNumber=atoi(store_neighbors[i][1]);
							dest[connected]->hostname=new char[20];
							dest[connected]->hostname=store_neighbors[i][0];
							dest[connected]->flag=0;
							dest[connected]->sockFd = beaconTcpSock;
							connected++;
						}

						
						//pthread_create(thread, NULL, NonBeaconReconnect, (void *)dest);
						delete ipAddress;
					}
					
					//CHECK IF THE NO. OF CONNECTIONS ARE GREATER THAN THE MIN. NO. OF NEIGHBORS
					if(connectCount >= MinNeighbours){
						for(int i=0;i< connected;i ++){
							pthread_t *thread;
							thread = new pthread_t;
							pthread_create(thread, NULL, NonBeaconReconnect, (void *)dest[i]);
						}
					}else{
						remove(directory);
						printf("\n alarm int");
						alarmInt = 1;
						catch_alarm_int(0);
						printf("\n Not Enough Neighbours");
						exit(0);
						//pthread_t reset;
						//pthread_create(&reset, NULL, ResetAll, (void *)0);
				
					}
					
				}			
			}
		}













   /*		if ((initFile = fopen(directory, "r")) == NULL) {
		    //printf("\nCouldn't open datafile for reading\n");
		    exit(0);
   		}else{
   			initFilePresent = 1;
   			init_neighbour(directory);
			if(totalNeighbours < MinNeighbours){
				printf("\n print exit");
				alarmInt = 1;
				catch_alarm_int(0);
			}else{	
				for(int i=0;i< totalNeighbours;i ++){
					printf("\n hostname = %s",store_neighbors[i][0]);
					printf("\n port number = %s",store_neighbors[i][1]);
					pthread_t *thread;
					thread = new pthread_t;
					hostLocation *dest;
					dest = new hostLocation;
					dest->portNumber=atoi(store_neighbors[i][1]);
					dest->hostname=new char[20];
					dest->hostname=store_neighbors[i][0];
					dest->flag=0;
					dest->sockFd = 0;
					pthread_create(thread, NULL, NonBeaconReconnect, (void *)dest);
				}
			}
		}*/
		//sleep(5);
		//pthread_t timer;
		//pthread_create(&timer, NULL, Timer, (void *)0);

		//if(dontResetAgain == 0){
			//printf("\n connect count = %d",connectCount);
			//pthread_mutex_lock (&countLock);
		//	if(connectCount < MinNeighbours){
				//pthread_mutex_unlock (&countLock);
				//printf("\n Not Enough Neighbours");
				//pthread_t reset;
				//pthread_create(&reset, NULL, ResetAll, (void *)0);
			
		//	}
			//pthread_mutex_unlock (&countLock);
		//}
		pthread_exit(NULL);
		return (void *)0;
	}
	
/*char * CurrentTime(){
	//printf("The time is \n");	
	time_t timezone;
	//int daylight;
	char buf[100],buf1[100];
	timezone=time(NULL);
	strcpy(buf1,ctime(&timezone));
	memcpy(&buf[0],&buf1[11],8);
	buf[9]='\0';
	//printf("The time is %s\n",buf);
	return buf;
}*/

//FUNCTION TO GET THE CURRENT TIME 
double GetTimeOfDay(){
	struct timeval tv;
	double inSec,inMsec;
	gettimeofday(&tv,NULL);
	inSec = (double)tv.tv_sec;
	inMsec = (double)(tv.tv_usec/1000000.00);
	return (inSec + inMsec);
}

//FOR STATUS MESSAGE, FUNCTION CALLED TO FORWARD MESSAGE TO THE NETWORK
void StatusForwardMessage(char *msgReceived,char *newPayload,int dontSendPort){
	
	MessageHeader *header = new MessageHeader;
	header->ParseHeader(msgReceived);
	//printf("\n parse->type = %d",header->messageType);
	//printf("\n parse->TTL = %d",header->TTL);
	//printf("\n parse->dataLength = %d \n '",header->dataLength);
	FDATA *write = new FDATA;
	write->logType = 'f';
	write->msgType = "STRQ";
	
	write->dataLength = header->dataLength;
	for(int index = 0;index<4;index ++){
		write->UOID[index] = header->UOID[16 + index];
	}
	short int tempTTL = (short int)header->TTL - 1;
	if(tempTTL > TTL){
		tempTTL = TTL;
	}
	write->TTL = tempTTL;//CHECK THE VALUE OF TTL
	tempTTL = tempTTL << 8;
	memcpy(&msgReceived[21],&tempTTL,1);
	write->data[0] = newPayload[0];
	write->data[1] = '\0';
	
	write->time = GetTimeOfDay();
	//printf("\n msg received = %s and datalength = %d---------------",msgReceived,strlen(msgReceived));
	//printf("\n payload = %s---------------",newPayload);
	pthread_mutex_lock(&socketListLock);
	SocketList *current = socketHead;
	int sendCount = 0;
	while(current != NULL){
		if(current->portNumber != dontSendPort){
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
			sprintf(write->target,"%s %d",sendPort->hostname,sendPort->portNumber);
			write->WriteToFile();
			SendMessage(sendPort->sockFd,msgReceived,newPayload,header->dataLength);
		}
		sendCount--;
		//current = current->next;
	}
	
}


void ParseHello(int childSd,char *message,int length,struct hostLocation *nodeData,int type){
	MessageHeader parsedMsg;
	Hello parsePayload;
	parsedMsg.ParseHeader(message);
	char *payload = new char[parsedMsg.dataLength];
	recv(childSd,payload,parsedMsg.dataLength,0);
	parsePayload.ParsePayload(payload,parsedMsg.dataLength);
	FDATA writeData;
	writeData.logType = 'r';
	for(int index = 0;index<4;index ++){
		writeData.UOID[index] = parsedMsg.UOID[16 + index];
	}
	nodeData->portNumber = parsePayload.portNumber;
	sprintf(nodeData->hostname,"%s",parsePayload.hostname);
	sprintf(writeData.target,"%s %d",parsePayload.hostname,parsePayload.portNumber);
	if(type == 1){
		writeData.msgType = "HLLO";
	}else{
		writeData.msgType = "HLLO";
	}
	writeData.dataLength = parsedMsg.dataLength;
	writeData.TTL = parsedMsg.TTL;
	sprintf(writeData.data,"%d %s",parsePayload.portNumber,parsePayload.hostname);
	writeData.time = GetTimeOfDay();
	writeData.WriteToFile();
	
}

//FUNCTION TO SEND THE HELLO MESSAGE
void SendHello(int childSd,struct hostLocation *nodeData,int type){
	//Sending HELLO response
	MessageHeader header;
	Hello hello;
	//printf("\n 1");
	FDATA writeData;
	header.messageType = HELLO;
	writeData.logType = 's';
	//printf("\n 2");
	if(type == 0){
		writeData.msgType = "HLLO";
	}else{
		writeData.msgType = "HLLO";
	}
	sprintf(writeData.target,"%s %d",nodeData->hostname,nodeData->portNumber);
	header.TTL = 1;
	memcpy((void *)&hello.portNumber,(void *)&portno_of_ini_file,2);
	strcpy(hello.hostname,myHostname);
	//printf("\n 3");
	header.dataLength = 2 + strlen(hello.hostname);
	writeData.dataLength = header.dataLength ;
	writeData.TTL = header.TTL;
	sprintf(writeData.data,"%d %s",nodeData->portNumber,nodeData->hostname);
	header.CreateHeader();
	//printf("\n 4");
	writeData.UOID = header.GetID();
	
	hello.CreateMessage(portno_of_ini_file,header.dataLength);
	//printf("\n sending hello");
	SendMessage(childSd,header.header,hello.message,header.dataLength);
	writeData.time = GetTimeOfDay();
	writeData.WriteToFile();
	//printf("\n 5");
}

//FUNCTION TO GET THE UOID FOR EACH AND EVERY MESSAGE
char *GetUOID(const unsigned char *node_inst_id,char *obj_type, char *uoid_buf,int uoid_buf_sz)
{
	/*cout<<node_inst_id<<endl;
	cout<<obj_type<<endl;
	cout<<uoid_buf_sz<<endl;*/
	//int i;
	
	static unsigned long seq_no=(unsigned long)1;
	char sha1_buf[20],str_buf[104];
	
	snprintf(str_buf,sizeof(str_buf),"%s_%s_%11d",node_inst_id,obj_type,(long)seq_no++);
	
	SHA1((const unsigned char *)str_buf,strlen(str_buf),(unsigned char *)sha1_buf);
	memset(uoid_buf,0,uoid_buf_sz);
	memcpy(uoid_buf,sha1_buf,min((int)uoid_buf_sz,(int)sizeof(sha1_buf)));

	/*printf("\n");
	for(i=0;i<20;i++)
		printf("%02x",uoid_buf[i]);
	printf("\n");
	//printf("uoid in getUOID fn=%s\n",uoid_buf);*/	
	return uoid_buf;

	
	
}


//FUNCTION CALLED TO BIND THE SOCKET WITH THE ADDRESS
void BindTcpSocket(){
	struct sockaddr_in tcp_addr;
	tcp_addr.sin_family = AF_INET; /* set up the server name */
	tcp_addr.sin_port = htons(portno_of_ini_file); /* use first available port number */
	char *ipAddress = NULL;
	ipAddress = GetHostIpAddress(myHostname);
	tcp_addr.sin_addr.s_addr = inet_addr(ipAddress);
	memset(&(tcp_addr.sin_zero), '\0', 8); // zero the rest of the struct
    int tcpBindReturn = bind(tcpSock, (struct sockaddr *)&tcp_addr, sizeof(struct sockaddr));
    if(tcpBindReturn == -1){
		printf("\n Error Binding to the Socket. Please try again.\n");
		close(tcpSock);
		alarmInt = 1;
		catch_alarm_int(1);
		exit(0);
	}
}

//THIS FUNCTION GETS THE IP ADDRESS OF THE HOST
char *GetHostIpAddress(char *hostname){
	struct hostent *hostName;
	hostName=gethostbyname(hostname);
    char *ipAddress = NULL;
    ipAddress = new char[20];
    ipAddress = inet_ntoa(*((struct in_addr *)hostName->h_addr));
	return ipAddress;
}

/*double GetTimeOfDay(){
	struct timeval tv;
	double inSec,inMsec;
	gettimeofday(&tv,NULL);
	inSec = (double)tv.tv_sec * 1000.00;
	inMsec = (double)(tv.tv_usec/1000.00);
	return (inSec + inMsec);
}*/


//THIS FUNCTION PARSES THE INI FILE AND GETS THE VALUES FROM IT
int parse_ini()
{

	char *portno,*hostname;
	dictionary *ini;
	FILE *fp1_beacons;
	char *section_name;
	char *port,*Location,*HomeDir,*LogFilename,*AutoShutdown,*TTL;
	char *MsgLifetime,*InitNeighbors,*JoinTimeout,*KeepAliveTimeout;
	char *MinNeighbors,*CacheProb,*StoreProb,*NeighborStoreProb,*CacheSize;
	char *PermSize,*Retry;
	char lin[1024];
	int keys_counter[17];//counter for all the keys
	int linno;//store the no. of lines in beacons_stored file
	char *keys[17]={"Port","Location","HomeDir","LogFilename","AutoShutdown","TTL","MsgLifetime","InitNeighbors","JoinTimeout",
								"KeepAliveTimeout","MinNeighbors","CacheProb","StoreProb","NeighborStoreProb","CacheSize","PermSize","Retry"};
	
	int i,j;
	static char *return_value_key;

	ini=iniparser_new(filename);
	//cout<<"after iniparser_new"<<endl;
	if(ini==NULL)
	{
		fprintf(stderr,"cannot parse inifile\n");
		exit(1);
	}


	//LOGIC FOR THREE DIMENSIONAL CHAR ARRAY IMPLEMENTATION
	ini_parser_keys=new char **[17];
			if(ini_parser_keys==NULL){cout<<"Could not allocate memory"<<endl;exit(1);}

			for(i=0;i<17;i++)
			{
			ini_parser_keys[i]=new char *[2];
		  			if(ini_parser_keys[i]==NULL){cout<<"Could not allocate memory"<<endl;exit(1);}
			}
			for(i=0;i<17;i++)
			{
				for(j=0;j<2;j++)
				{
					ini_parser_keys[i][j]=new char [100];
				}
			}

	//SET ALL THE VARIABLES TO NULL
	for(i=0;i<17;i++)
		{
				ini_parser_keys[i][1]="null";
		}


	//get the name of the section n in a dictionary
	section_name=iniparser_getsecname(ini,1);
	

	//Get value from the dictionary
	for(i=0;i<17;i++)
	keys_counter[i]=0;

	for(i=0;i<17;i++)
	{
		ini_parser_keys[i][0]=keys[i];
	}

	port=iniparser_getstring(ini,"init:Port","not found");
	if(port)
	{
		ini_parser_keys[0][1]=port;
		portno_of_ini_file=atoi(ini_parser_keys[0][1]);
	//	cout<<"Port = "<<port<<endl;
		keys_counter[0]++;
	}
	

	//Get Value of Location
	Location=iniparser_getstr(ini,"init:Location");

	if(Location)
	{
			ini_parser_keys[1][1]=Location;
			//cout<<"Location = "<<Location<<endl;
			keys_counter[1]++;
	}
	

	//Get Value of HomeDir
	HomeDir=iniparser_getstr(ini,"init:HomeDir");
	if(HomeDir)
		{
			ini_parser_keys[2][1]=HomeDir;
			//cout<<"HomeDir = "<<HomeDir<<endl;
			keys_counter[2]++;
		}
	

	//Get value of LogFilename
	LogFilename=iniparser_getstr(ini,"init:LogFilename");
	if(LogFilename)
		{
			ini_parser_keys[3][1]=LogFilename;
			//cout<<"LogFilename = "<<LogFilename<<endl;
			keys_counter[3]++;
		}
	

	//Get Value of AutoShutdown
	AutoShutdown=iniparser_getstr(ini,"init:AutoShutdown");
	if(AutoShutdown)
		{
			ini_parser_keys[4][1]=AutoShutdown;
			//cout<<"AutoShutdown = "<<AutoShutdown<<endl;
			keys_counter[4]++;
		}
	

	//Get Value of TTL
	TTL=iniparser_getstr(ini,"init:TTL");
	if(TTL)
		{
			ini_parser_keys[5][1]=TTL;
			//cout<<"TTL = "<<TTL<<endl;
			keys_counter[5]++;
		}
		

	//Get Value of MsgLifeTime
	MsgLifetime=iniparser_getstr(ini,"init:MsgLifetime");
	if(MsgLifetime)
		{
			ini_parser_keys[6][1]=MsgLifetime;
			//cout<<"MsgLifetime = "<<MsgLifetime<<endl;
			keys_counter[6]++;
		}
	
	//Get Value of InitNeighbors
	InitNeighbors=iniparser_getstr(ini,"init:InitNeighbors");
	if(InitNeighbors)
		{
			ini_parser_keys[7][1]=InitNeighbors;
			//cout<<"InitNeighbors = "<<InitNeighbors<<endl;
			keys_counter[7]++;
		}
	

	//Get Value of JoinTimeout
	JoinTimeout=iniparser_getstr(ini,"init:JoinTimeout");
	if(JoinTimeout)
		{
			ini_parser_keys[8][1]=JoinTimeout;
			//cout<<"JoinTimeout = "<<JoinTimeout<<endl;
			keys_counter[8]++;
		}
	
	//Get Value of KeepAliveTimeout
	KeepAliveTimeout=iniparser_getstr(ini,"init:KeepAliveTimeout");
	if(KeepAliveTimeout)
		{
			ini_parser_keys[9][1]=KeepAliveTimeout;
			//cout<<"KeepAliveTimeout = "<<KeepAliveTimeout<<endl;
			keys_counter[9]++;
		}
	
	//Get Value of MinNeighbors
	MinNeighbors=iniparser_getstr(ini,"init:MinNeighbors");
	if(MinNeighbors)
		{
			ini_parser_keys[10][1]=MinNeighbors;
			//cout<<"MinNeighbors = "<<MinNeighbors<<endl;
			keys_counter[10]++;
		}
	

	//Get Value of CacheProb
	CacheProb=iniparser_getstr(ini,"init:CacheProb");
	if(CacheProb)
		{
			ini_parser_keys[11][1]=CacheProb;
			//cout<<"CacheProb = "<<CacheProb<<endl;
			keys_counter[11]++;
		}
	
	//Get Value of StoreProb
	StoreProb=iniparser_getstr(ini,"init:StoreProb");
	if(StoreProb)
		{
			ini_parser_keys[12][1]=StoreProb;
			//cout<<"StoreProb = "<<StoreProb<<endl;
			keys_counter[12]++;
		}
	

	//Get Value of NeighborStoreProb
	NeighborStoreProb=iniparser_getstr(ini,"init:NeighborStoreProb");
	if(NeighborStoreProb)
		{
			ini_parser_keys[13][1]=NeighborStoreProb;
			//cout<<"NeighborStoreProb = "<<NeighborStoreProb<<endl;
			keys_counter[13]++;
		}
	
	//Get Value of NeighborStoreProb
  CacheSize=iniparser_getstr(ini,"init:CacheSize");
  if(CacheSize)
  	{
  		ini_parser_keys[14][1]=CacheSize;
  		//cout<<"CacheSize = "<<CacheSize<<endl;
  		keys_counter[14]++;
  	}
  

	//Get Value of PermSize
  PermSize=iniparser_getstr(ini,"init:PermSize");
  if(PermSize)
  	{
  		ini_parser_keys[15][1]=PermSize;
  		//cout<<"PermSize = "<<PermSize<<endl;
			keys_counter[15]++;
		}
	
	//Get Value for Retry for Beacons
	Retry=iniparser_getstr(ini,"beacons:Retry");
	if(Retry)
		{
			ini_parser_keys[16][1]=Retry;
			//cout<<"Retry = "<<Retry<<endl;
			keys_counter[16]++;
		}
	
	return_value_key=iniparser_beacon_nodes(ini,filename);
	if((fp1_beacons=fopen("beacons_stored","r"))==NULL)
		{
			cout<<"Cannot read file beacons_stored"<<endl;
			exit(1);
		}

	//READ THE BEACONS_STORED FILE HERE
	linno=0;
	while(fgets(lin,1025,fp1_beacons)!=NULL)
	{
		linno++;
		portno=new char[10];
	  	hostname=new char[20];
		if(sscanf (lin, "%[^:] : %[^;#]",hostname, portno) == 2)
			{
		
			insert_beacon(hostname,portno);
			}
			delete portno;
			delete hostname;

	}
	return linno;

}//ini_parser function ends

//THIS FUNCTION READS THE NEIGHBORS FROM THE INIT_NEIGHBOR_FILE AND STORES 
//THEM, SO THAT THEY CAN BE ACCESSED LATER ON IN THE PROGRAM
void init_neighbour(char *init_neighbor_file)
{
	char lin[1025];
	int i,j,no_of_lines;
	FILE *fp;
	dictionary *ini;
	
	if((fp=fopen(init_neighbor_file,"r"))==NULL)
		{
		printf("ERROR OPENING THE FILE: %s",init_neighbor_file);
		exit(1);
		}

	ini=iniparser_new(init_neighbor_file);

	no_of_lines=0;
	while(fgets(lin,1024,fp)!=NULL)
	{
		no_of_lines++;
	}
	
	if(ini==NULL)
	{
		fprintf(stderr,"cannot parse  init_neighbor_list\n");
		exit(1);
	}

	//ALLOCATE MEMORY FOR STORING NEIGHBORS
	store_neighbors=(char ***)malloc(no_of_lines*sizeof(char));
		if(store_neighbors==NULL){
			fprintf(stderr,"Could not allocate memory for storing neighbors\n");
			exit(1);}
			
		
	for(i=0;i<no_of_lines;i++)		
		{
		store_neighbors[i]=(char **)malloc(2*sizeof(char));
		if(store_neighbors[i]==NULL){
			fprintf(stderr,"Could not allocate memory for storing neighbors\n");
			exit(1);}
		}
	totalNeighbours = no_of_lines;	
	for(i=0;i<no_of_lines;i++)
		{
		for(j=0;j<2;j++)
			{
			store_neighbors[i][j]=(char *)malloc(100*sizeof(char));
			}
		}
	
	store_neighbors=init_neighbor_list(ini,init_neighbor_file);

	fclose(fp);
	
}//init_neighbor function ends
