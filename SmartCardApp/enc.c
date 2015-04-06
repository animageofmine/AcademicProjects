#include "header.h"
#include "scintrf.h"
#include "defs.h"

int enc(char *sopath,char *filename)
{
	//printf("you are in the enc program\n");
        SHA_CTX md;
	int eight=8;
	DES_key_schedule schedule1;
	DES_key_schedule schedule2;
	DES_key_schedule schedule3;
        FILE *fp;
        unsigned char buffer[10],DES_output[8];
	char DES_input[8],ch;
        unsigned char digest[20],file_digest[20];
        int count,i,len_of_pass,verify=1,no_of_bytes;
	unsigned char random_bits1[20],random_bits2[20],DES_key1[8],DES_key2[8],DES_key3[8],IV[8];
	char *buf,*buff;
	

	if(RAND_bytes(random_bits1,20)==0)
	{
		fprintf(stderr,"Error obtaining 20 bytes of random bits from OPENSSL\n");
		exit(1);
	}
	if(RAND_bytes(random_bits2,20)==0)
	{
		fprintf(stderr,"Error obtaining 20 bytes of random bits from OPENSSL\n");
		exit(1);
	}
/**************************************
COMPUTE SHA1 OF FILE
**************************************/

        if((fp=fopen(filename,"rb"))==NULL)
        {
                fprintf(stderr,"(input file %s does not exist)\n",filename);
                exit(1);
        }
        SHA1_Init(&md);
        while((count=fread(&buffer,1,10,fp)))
        {
                SHA1_Update(&md,buffer,count);
        }
        SHA1_Final(file_digest,&md);

/*        for(i=0;i<20;i++)
        {
                printf("%02x",file_digest[i]);
        }*/

/**************************************
ASK FOR PASSWORD FOR DES
**************************************/
buf=(char *)malloc(4096*sizeof(char));
buff=(char *)malloc(4096*sizeof(char));
des_read_pw(buf,buff,4096,"Enter passphrase for DES encryption: ",verify);

//printf("buff=%s",buf);
//printf("buff2=%s\n",buff);
len_of_pass=strlen(buf);
//printf("len_of_pass=%d\n",len_of_pass);

/**************************************
GENERATE SHA1 OF THE PASSWORD
**************************************/

	SHA1_Init(&md);

	//free(&md);
	//SHA1_Update(&md,"yesnomaybe",10);
	SHA1_Update(&md,buf,strlen(buf));
	SHA1_Final(digest,&md);

	//printf("The SHA1 of password is\n");
        for(i=0;i<20;i++)
        {
                //printf("%02x",digest[i]);
        }

        //printf("\n");
	

/*******************************************
COMPUTE SHA1 OF X AND RANDOM BITS(RB1) 1
*******************************************/
/*	SHA1_Init(&md);
	SHA1_Update(&md,digest,20);
	SHA1_Update(&md,HW4_random_bits_1,20);
	SHA1_Final(X_RB1,&md);
	for(i=0;i<20;i++)
	{
		//printf("%02x",X_RB1[i]);
	}
	//printf("\n");*/

/******************************************
COMPUTE SHA1 OF RB1(Y) AND RANDOM BITS 2(RB2)
******************************************/
/*	SHA1_Init(&md);
        SHA1_Update(&md,X_RB1,20);
        SHA1_Update(&md,HW4_random_bits_2,20);
        SHA1_Final(X_RB2,&md);
        for(i=0;i<20;i++)
        {
                //printf("%02x",X_RB2[i]);
        }
        //printf("\n");*/

/***************************************
COMPUTER IV AND 3 DES KEYS
***************************************/
//	SC_3des_key_gen(NULL,digest,IV,DES_key1,DES_key2,DES_key3);


	SmartcardState pss=(SmartcardState)NULL;
void *handle=NULL;
//void *vp =NULL;
SC_Init_Func *pfn_init=NULL;
SC_Cleanup_Func *pfn_cleanup=NULL;
SC_GetSignatureSize_Func *pfn_getsigsize=NULL;
SC_3DesKeyGen_Func *pfn_sc_enc=NULL;

handle = dlopen(sopath, RTLD_NOW|RTLD_GLOBAL);
if (handle==NULL)
{
//fprintf(stderr, "%s\n",sopath);
fprintf(stderr, "%s\n",
dlerror());
fprintf(stderr,"dlopen() failed\n");
exit(1);
}
pfn_init = (SC_Init_Func*)dlsym(handle,"SC_init");


pfn_cleanup = (SC_Cleanup_Func*)dlsym(handle,"SC_cleanup");


pfn_getsigsize = (SC_GetSignatureSize_Func*)dlsym(handle,"SC_get_signature_size");

pfn_sc_enc = (SC_3DesKeyGen_Func *)dlsym(handle,"SC_3des_key_gen");

if(pfn_init==NULL || pfn_cleanup==NULL || pfn_getsigsize==NULL || pfn_sc_enc==NULL)
{
fprintf(stderr,"(cannot find smartcard functions in this library)\n");
exit(1);
}


pss = (pfn_init)();
//printf("sig size is %d\n", (pfn_getsigsize)(pss));

(pfn_sc_enc) (pss,digest,IV,DES_key1,DES_key2,DES_key3);

(pfn_cleanup)(pss);
dlclose(handle);

/*	for(i=0;i<8;i++)
	IV[i]=X_RB1[i];
	for(i=8,j=0;i<16;i++,j++)
	DES_key1[j]=X_RB1[i];
	for(i=0;i<8;i++)
	DES_key2[i]=X_RB2[i];
	for(i=8,j=0;i<16;i++,j++)
	DES_key3[j]=X_RB2[i];
	printf("IV=");
	for(i=0;i<8;i++)
	printf("%02x",IV[i]);
	printf("\nDES KEY 1=");
	for(i=0;i<8;i++)
	printf("%02x",DES_key1[i]);
	printf("\nDES key 2=");
	for(i=0;i<8;i++)
	printf("%02x",DES_key2[i]);
	printf("\nDES key 3=");
	for(i=0;i<8;i++)
	printf("%02x",DES_key3[i]);
	printf("\n");*/


	
	
	DES_set_odd_parity((DES_cblock *)DES_key1);
	DES_set_odd_parity((DES_cblock *)DES_key2);
	DES_set_odd_parity((DES_cblock *)DES_key3);
	
	DES_key_sched((DES_cblock *)DES_key1,&schedule1);
	DES_set_key((DES_cblock *)DES_key1,&schedule1); 

        DES_key_sched((DES_cblock *)DES_key2,&schedule2);
        DES_set_key((DES_cblock *)DES_key2,&schedule2);

        DES_key_sched((DES_cblock *)DES_key3,&schedule3);
        DES_set_key((DES_cblock *)DES_key3,&schedule3);

/******************************************
READ FILE 8 BYTES AT A TIME AND ENCRYPT IT
******************************************/

	if((fp=fopen(filename,"rb"))==NULL)
	{
		fprintf(stderr,"{input file %s does not exist}\n",filename);
		exit(1);
	}
	i=0;
	while((count=fread(&ch,1,1,fp)))
	{
	i++;
	}
//	printf("no. of bytes=%d\n",i);
	no_of_bytes=i%8;
	fclose(fp);	
	printf("HW4");
	if(no_of_bytes>0)
	printf("%c",no_of_bytes);
	else
	printf("%c",eight);
        if((fp=fopen(filename,"rb"))==NULL)
        {
                fprintf(stderr,"{input file %s does not exist}\n",filename);
                exit(1);
        }
	while((count=fread(&DES_input,1,8,fp)))
	{
		if(count!=8)
		{
		for(i=count;i<8;i++)
		DES_input[i]='\0';
		DES_ede3_cbc_encrypt(DES_input,DES_output,8,&schedule1,&schedule2,&schedule3,(DES_cblock*)IV,1);
		for(i=0;i<8;i++)
		printf("%c",DES_output[i]);
	//	printf("%s",DES_output);
		}
		else
		{
	//	printf("%d\n",count);	
		DES_ede3_cbc_encrypt(DES_input,DES_output,8,&schedule1,&schedule2,&schedule3,(DES_cblock*)IV,1);
		for(i=0;i<8;i++)
		printf("%c",DES_output[i]);
//    		printf("%s",DES_output);
		}
	}
	for(i=0;i<20;i++)
	printf("%c",file_digest[i]);
		

	return 1;


}
