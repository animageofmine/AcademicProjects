/* THIS HEADER FILES DECLARES THE HEADER CALLED STORE_BEACONS WHICH STORES THE BEACONS OF THE WHOLE
NETWORK */

#ifndef _LINKLISTBEACONS_H_
#define _LINKLISTBEACONS_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct store_beacons
{
		char *port;
		char *hostname;
		struct store_beacons *next;
};

#endif

/* FUCTION DECLARATIONS FOR LINK LIST OF BEACONS*/
void insert_beacon(char *,char *);
void display_beacons();
char *return_hostname_of_beacon(int);
char *return_portno_of_beacon(int);
