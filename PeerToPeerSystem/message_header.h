#ifndef MESSAGE_HEADER
#define MESSAGE_HEADER




class MessageHeader{
	public:
		MessageHeader(void);
		int messageType;
		char *UOID;
		int TTL;
		int dataLength;
		char *header;
		void CreateHeader(void);
		void ParseHeader(char *);
		unsigned char *GetID();
};
#endif
