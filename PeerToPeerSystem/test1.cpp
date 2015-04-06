#include "iniparser.h"
#include "iniparser.c"
#include "linklist_beacons.h"
//#include "linklist_beacons.cc"
#include<iostream>
using namespace std;

extern struct store_beacons *first;
int  main()
{
	first=NULL;
	printf("hello\n");
	char ***ptr;
	char *portno,*hostname;
	dictionary *ini;
	FILE *fp,*fp1;
	char *section_name;
	char *port,*Location,*HomeDir,*LogFilename,*AutoShutdown,*TTL;
	char *MsgLifetime,*InitNeighbors,*JoinTimeout,*KeepAliveTimeout;
	char *MinNeighbors,*CacheProb,*StoreProb,*NeighborStoreProb,*CacheSize;
	char *PermSize,*Retry,*where;
	char lin[1024];
	int keys_counter[17];//counter for all the keys
	int linno;//store the no. of lines in beacons_stored file
	char *keys[17]={"Port","Location","HomeDir","LogFilename","AutoShutdown","TTL","MsgLifetime","InitNeighbors","JoinTimout",
								"KeepAliveTimeout","MinNeighbors","CacheProb","StoreProb","NeighborStoreProb","CacheSize","PermSize","Retry"};
	char *keys1[17]={"init:Port","init:Location","init:HomeDir","init:LogFilename","init:AutoShutdown","init:TTL","init:MsgLifetime",
									"init:InitNeighbors","init:JoinTimout","init:KeepAliveTimeout","init:MinNeighbors","init:CacheProb",
									"init:StoreProb","init:NeighborStoreProb","init:CacheSize","init:PermSize","beacons:Retry"};
	int i,j,key_counter;
	static char *return_value_key;
	
	
	/*portno=new char[10];
	hostname=new char[20];*/
	
	
	fp=fopen("inifile.ini","r");	
	printf("after fileopen\n");
	if(fp==NULL)
	{
		cout<<"cannot open file for reading"<<endl;
		exit(1);
	}
	
	
	ini=iniparser_new("inifile_test.ini");
	cout<<"after iniparser_new"<<endl;
	if(ini==NULL)
	{
		fprintf(stderr,"cannot parse inifile\n");
		exit(1);
	}

	
	//LOGIC FOR THREE DIMENSIONAL CHAR ARRAY IMPLEMENTATION
	ptr=new char **[17];
			if(ptr==NULL){cout<<"Could not allocate memory"<<endl;exit(1);}
			
			for(i=0;i<17;i++)
			{
			ptr[i]=new char *[2];
		  			if(ptr[i]==NULL){cout<<"Could not allocate memory"<<endl;exit(1);}
			}
			for(i=0;i<17;i++)
			{
				for(j=0;j<2;j++)
				{
					ptr[i][j]=new char [100];
				}
			}
	
	
	
	//get the name of the section n in a dictionary
	printf("The no. of sections in INI file are %d\n",iniparser_getnsec(ini));
	section_name=iniparser_getsecname(ini,1);
	//iniparser_getsecname(ini,0);
	for(i=0;i<iniparser_getnsec(ini);i++)
		cout<<"Section_name="<<iniparser_getsecname(ini,i)<<endl;
		cout<<endl;
		

	//Get value from the dictionary
//	printf("The value for Port is %d",iniparser_getboolean(ini,"init:Port",-1));
	for(i=0;i<17;i++)
	keys_counter[i]=0;

//TRIAL
/*	for(i=0;i<17;i++)
	{
		return_value_key=iniparser_getstring(ini,keys1[i],"not found");
		if(strcmp(return_value_key,"not found")!=0)
			{
				if(keys_counter[i]>1)
					continue;
				else
					{
						keys_counter[i]++;
						cout<<keys[i]<<" = "<<return_value_key<<endl;
					}
			}
	}
	return_value_key=iniparser_beacon_nodes(ini,"inifile_test.ini");
	if((fp1=fopen("beacons_stored","r"))==NULL)
		{
			cout<<"Cannot read file beacons_stored"<<endl;
			exit(1);
		}
	
	//READ THE BEACONS_STORED FILE HERE
	linno=0;
	while(fgets(lin,1025,fp1)!=NULL)
	{
		linno++;
		cout<<lin;
	}*/
				
//	fclose(fp1);
	/*return_value_key=iniparser_beacon_nodes(ini,"inifile_test.ini");
	printf("return value from ini beacon nodes = %s\n",return_value_key);
	display1();*/

	for(i=0;i<17;i++)
	{
		ptr[i][0]=keys[i];
	}

	port=iniparser_getstring(ini,"init:Port","not found");
	if(port)
	{
		ptr[0][1]=port;
		cout<<"Port = "<<port<<endl;
		keys_counter[0]++;
	}
	else
			printf("Port Not found\n");


	//Get Value of Location
	Location=iniparser_getstr(ini,"init:Location");

	if(Location)
	{
			ptr[1][1]=Location;
			cout<<"Location = "<<Location<<endl;
			keys_counter[1]++;
	}
	else
		cout<<"Location not found"<<endl;
		
	//Get Value of HomeDir
	HomeDir=iniparser_getstr(ini,"init:HomeDir");
	if(HomeDir)
		{
			ptr[2][1]=HomeDir;
			cout<<"HomeDir = "<<HomeDir<<endl;
			keys_counter[2]++;
		}
	else
		cout<<"HomeDir not found"<<endl;
		
	//Get value of LogFilename
	LogFilename=iniparser_getstr(ini,"init:LogFilename");
	if(LogFilename)
		{
			ptr[3][1]=LogFilename;
			cout<<"LogFilename = "<<LogFilename<<endl;
			keys_counter[3]++;
		}
	else
		cout<<"LogFilename not found"<<endl;
	
	//Get Value of AutoShutdown
	AutoShutdown=iniparser_getstr(ini,"init:AutoShutdown");
	if(AutoShutdown)
		{
			ptr[4][1]=AutoShutdown;
			cout<<"AutoShutdown = "<<AutoShutdown<<endl;
			keys_counter[4]++;
		}
	else
		cout<<"Autoshutdown not found"<<endl;
		
	//Get Value of TTL
	TTL=iniparser_getstr(ini,"init:TTL");
	if(TTL)
		{
			ptr[5][1]=TTL;
			cout<<"TTL = "<<TTL<<endl;
			keys_counter[5]++;
		}
		else
			cout<<"TTL not found"<<endl;
			
	//Get Value of MsgLifeTime
	MsgLifetime=iniparser_getstr(ini,"init:MsgLifetime");
	if(MsgLifetime)
		{
			ptr[6][1]=MsgLifetime;
			cout<<"MsgLifetime = "<<MsgLifetime<<endl;
			keys_counter[6]++;
		}
	else
		cout<<"MsgLifetime not found"<<endl;
	
	//Get Value of InitNeighbors
	InitNeighbors=iniparser_getstr(ini,"init:InitNeighbors");
	if(InitNeighbors)
		{
			ptr[7][1]=InitNeighbors;
			cout<<"InitNeighbors = "<<InitNeighbors<<endl;
			keys_counter[7]++;
		}
	else
		cout<<"InitNeighbors not found"<<endl;
	
	//Get Value of JoinTimeout
	JoinTimeout=iniparser_getstr(ini,"init:JoinTimeout");
	if(JoinTimeout)
		{	
			ptr[8][1]=JoinTimeout;
			cout<<"JoinTimeout = "<<JoinTimeout<<endl;
			keys_counter[8]++;
		}
	else
		cout<<"JoinTimeout not Found"<<endl;
	
	//Get Value of KeepAliveTimeout
	KeepAliveTimeout=iniparser_getstr(ini,"init:KeepAliveTimeout");
	if(KeepAliveTimeout)
		{
			ptr[9][1]=KeepAliveTimeout;
			cout<<"KeepAliveTimeout = "<<KeepAliveTimeout<<endl;
			keys_counter[9]++;
		}
	else
		cout<<"KeepAliveTimeout not Found"<<endl;
		
	//Get Value of MinNeighbors
	MinNeighbors=iniparser_getstr(ini,"init:MinNeighbors");
	if(MinNeighbors)
		{
			ptr[10][1]=MinNeighbors;
			cout<<"MinNeighbors = "<<MinNeighbors<<endl;
			keys_counter[10]++;
		}
	else
		cout<<"MinNeighbors not Found"<<endl;
	
	//Get Value of CacheProb
	CacheProb=iniparser_getstr(ini,"init:CacheProb");
	if(CacheProb)
		{
			ptr[11][1]=CacheProb;
			cout<<"CacheProb = "<<CacheProb<<endl;
			keys_counter[11]++;
		}
	else
		cout<<"CacheProb Not Found"<<endl;
		
	//Get Value of StoreProb
	StoreProb=iniparser_getstr(ini,"init:StoreProb");
	if(StoreProb)
		{
			ptr[12][1]=StoreProb;
			cout<<"StoreProb = "<<StoreProb<<endl;
			keys_counter[12]++;
		}
	else
		cout<<"StoreProb Not Found"<<endl;
	
	//Get Value of NeighborStoreProb
	NeighborStoreProb=iniparser_getstr(ini,"init:NeighborStoreProb");
	if(NeighborStoreProb)
		{
			ptr[13][1]=NeighborStoreProb;
			cout<<"NeighborStoreProb = "<<NeighborStoreProb<<endl;
			keys_counter[13]++;
		}
	else
		cout<<"NeighborStoreProb Not Found"<<endl;
	
	//Get Value of NeighborStoreProb
  CacheSize=iniparser_getstr(ini,"init:CacheSize");
  if(CacheSize)
  	{
  		ptr[14][1]=CacheSize;
  		cout<<"CacheSize = "<<CacheSize<<endl;
  		keys_counter[14]++;
  	}
  else
  	cout<<"CacheSize not found"<<endl;
  	
	//Get Value of PermSize
  PermSize=iniparser_getstr(ini,"init:PermSize");
  if(PermSize)
  	{
  		ptr[15][1]=PermSize;
  		cout<<"PermSize = "<<PermSize<<endl;
			keys_counter[15]++;
		}
	else
		cout<<"PermSize Not Found"<<endl;
	
	//Get Value for Retry for Beacons
	Retry=iniparser_getstr(ini,"beacons:Retry");
	if(Retry)
		{
			ptr[16][1]=Retry;
			cout<<"Retry = "<<Retry<<endl;
			keys_counter[16]++;
		}
	else
		cout<<"Retry Not Found"<<endl;
	
	for(i=0;i<17;i++)
		{
			printf("%s=%s\n",ptr[i][0],ptr[i][1]);
//		if(keys_counter[i]==0 || keys_counter[i]>1)
			//cout<<keys[i]<<endl;
		}
		return_value_key=iniparser_beacon_nodes(ini,"inifile_test.ini");
	if((fp1=fopen("beacons_stored","r"))==NULL)
		{
			cout<<"Cannot read file beacons_stored"<<endl;
			exit(1);
		}
	
	//READ THE BEACONS_STORED FILE HERE
	linno=0;
	while(fgets(lin,1025,fp1)!=NULL)
	{
		linno++;
		cout<<lin;
		
		/*The "lin" contains the beacons(host and the port no)
		  Now we split the host and the port no. and store it in a global data structure*/
		  
		//insert_beacon(lin,lin);
		portno=new char[10];
	  hostname=new char[20];
		if(sscanf (lin, "%[^:] : %[^;#]",hostname, portno) == 2)
			{
			printf("\t\tportno=%s, hostname=%s\n",portno,hostname);
			
			insert_beacon(hostname,portno);
			/*insert_beacon("nunki.usc.edu","12345");
			insert_beacon("nunki1.usc.edu","23456");
			insert_beacon("nunki2.usc.edu","34567");*/
			}
			delete portno;
			delete hostname;
		
	}
	printf("The beacons are\n");
	display_beacons();
	printf("\n");	
	return 1;
}

