#ifndef NEIGHBOUR
#define NEIGHBOUR
struct Neighbour{
	int portNumber;
	char *hostname;
	int distance;
	struct Neighbour *next;
};
#endif

int AddNeighbour(int,char *,long int);
void PrintNeighbour();
