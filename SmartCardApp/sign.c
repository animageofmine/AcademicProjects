#include "header.h"
#include "defs.h"
#include "scintrf.h"


int sign(char *sopath,char *filename)
{
	RSA *rsa;	
	unsigned char to[512]="";
        SHA_CTX md;
        FILE *fp;
        char buffer[10];
        unsigned char digest[20];
        int count,i;

	if((rsa=RSA_new())==NULL)
	{
		fprintf(stderr,"{key_new: RSA NEW failed}\n");
		exit(1);
	}

	rsa->n=BN_new();
	rsa->e=BN_new();
	rsa->d=BN_new();


/***********************************
COUNT SHA1 OF THE FILE
***********************************/
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
        SHA1_Final(digest,&md);

/*        for(i=0;i<20;i++)
        {
                printf("%02x",digest[i]);
        }*/


/*************************************
SET d,e and n values from secret.c
*************************************/
/*
j=0;
for(i=0;i<256;i+=2)
{
sprintf(tmp,"%.02x",HW4_rsa_private_key[j]);
tmp[2]='\0';
buff[i]=tmp[0];
buff[i+1]=tmp[1];
j++;
}
count=BN_hex2bn(&rsa->d,buff);
//BN_copy(rsa->d,b1);


j=0;
for(i=0;i<256;i+=2)
{
sprintf(tmp,"%.02x",HW4_rsa_public_modulus[j]);
tmp[2]='\0';
buff[i]=tmp[0];
buff[i+1]=tmp[1];
j++;
}
count=BN_hex2bn(&rsa->n,buff);

j=0;
for(i=0;i<6;i+=2)
{
sprintf(tmp,"%.02x",HW4_rsa_public_exponent[j]);
tmp[2]='\0';
buff1[i]=tmp[0];
buff1[i+1]=tmp[1];
j++;
}
count=BN_hex2bn(&rsa->e,buff1);
//printf("count=%d\n",count);
*/


/************************************
SIGN THE DIGEST OF THE FILE 
************************************/
	
//	SC_sign(NULL,digest,(unsigned char *)to);

	SmartcardState pss=(SmartcardState)NULL;
void *handle=NULL;
//void *vp =NULL;
SC_Init_Func *pfn_init=NULL;
SC_Cleanup_Func *pfn_cleanup=NULL;
SC_GetSignatureSize_Func *pfn_getsigsize=NULL;
SC_Sign_Func *pfn_sc_sign=NULL;

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

pfn_sc_sign = (SC_Sign_Func*)dlsym(handle,"SC_sign");
if(pfn_init==NULL || pfn_cleanup==NULL || pfn_getsigsize==NULL || pfn_sc_sign==NULL)
{
fprintf(stderr,"(cannot find smartcard functions in this library)\n");
exit(1);
}
pss = (pfn_init)();

//printf("sig size is %d\n", (pfn_getsigsize)(pss));
//int sz=(pfn_getsigsize)(pss);

(pfn_sc_sign) (pss,digest,(unsigned char*)to);

(pfn_cleanup)(pss);
dlclose(handle);


//	printf("\n");
	for(i=0;i<128;i++)
	printf("%02x",to[i]);
	printf("\n");
	return 1;

}
	

