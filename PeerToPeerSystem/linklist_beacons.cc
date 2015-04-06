//INCLUDE THE REQUIRED HEADER FILES TO CREATE THE LINK LIST
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "linklist_beacons.h"

struct store_beacons *first,*last;

//FUNCTION FOR INSERTING THE BEACON IN THE LINKLIST
void insert_beacon(char *host_name,char *port_no)
{
	//printf("in insert\n");
	struct store_beacons *new1=new struct store_beacons;
	struct store_beacons *p=first;
	if(new1==NULL)//IF THE LINK LIST OF BEACONS IS EMPTY
		printf("not enough memory for a new node\n");
	new1->hostname=new char[20];
	new1->port=new char[20];
	strcpy(new1->hostname,host_name);
	strcpy(new1->port,port_no);
	new1->next=NULL;
	if(first==NULL)//first element in link list
	{
			first=new1;
	}
	else//IF THE LINK LIST OF BEACONS IS NOT EMPTY
	{
		//printf("link list is not empty\n");
	//	printf("first element is, port= %s, hostname=%s\n",first->port,first->hostname);
		while(p->next!=NULL)
		{

			p=p->next;
		}
		p->next=new1;
	}
}

//FUNCTION TO DISPLAY THE BEACONS IN THE BEACONS LINK LIST
void display_beacons()
{
	//printf("in display\n");
	struct store_beacons *new1;
	new1=(first);
	//printf("first node in the link list is, port = %s, hostname=%s\n",new1->port,new1->hostname);
	if(new1!=NULL)
	{
		while(new1->next!=NULL)
		{
			//printf("host=%s, port=%s\n",new1->hostname,new1->port);
			new1=new1->next;
		}
		//printf("host=%s, port=%s\n",new1->hostname,new1->port);
	}
	else
	{
		printf("The link list is empty\n");
	}
	//printf("\n");
}


//THIS FUNCTION RETURNS THE HOSTNAME ACCORDING TO THE BEACON NUMBER REQUESTED FROM SV_NODE
char * return_hostname_of_beacon(int beacon_number)
{
	int count=0;
	struct store_beacons *temp;
	temp=first;
	if(temp!=NULL)
		{
			while(count!=beacon_number)
			{
				count++;
				temp=temp->next;
			}
			return temp->hostname;
		}
	else
		{
			printf("The link list of beacons is empty\n");
		}
	return (char *)0;
}

//THIS FUNCTION RETURNS THE PORT NUMBER ACCORDING TO THE BEACON NUMBER REQUESTED FROM SV_NODE PROGRAM
char * return_portno_of_beacon(int beacon_number)
{
	int count=0;
	struct store_beacons *temp;
	temp=first;
	if(temp!=NULL)
		{
			while(count!=beacon_number)
			{
				count++;
				temp=temp->next;
			}
			return temp->port;
		}
	else
		{
			printf("The link list of beacons is empty\n");
		}
		return (char *)0;
}
