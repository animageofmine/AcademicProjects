#ifndef FILE_H
#define FILE_H

class FDATA{
	public:
	char logType;
	double time;
	char *target;
	char *msgType;
	int dataLength;
	int TTL;
	unsigned char *UOID;
	char *data;
	char *oneLine;
	char *errorMsg;
	FDATA(void);
	void WriteToFile(void);
	void WriteErrorToFile();
};
#endif
