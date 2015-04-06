#ifndef HELLO_H
#define HELLO_H

class Hello{
	public:
		short int portNumber;
		char *hostname;
		char *message;
		int port;
		Hello(void);
		void CreateMessage(int,int);
		void ParsePayload(char *,int);
};
#endif
