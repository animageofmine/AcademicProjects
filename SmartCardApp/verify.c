#include "header.h"
#include "scintrf.h"
#include "defs.h"

int verify(char *sopath,char *filename)
{
	RSA *rsa;	
	char tmp[3],*filebuffer,ch;
	unsigned char *modulus,*exponent;
	unsigned char temp,temp1;
	//unsigned char *modulus1,*exponent1;
	unsigned char modulus1[512],exponent1[512];
        FILE *fp;
        unsigned char *to,filebuffer1[128];
        int count,i,j,k;
	int size_of_exponent,size_of_modulus;

	if((rsa=RSA_new())==NULL)
	{
		fprintf(stderr,"{key_new: RSA NEW failed}\n");
		exit(1);
	}

	rsa->n=BN_new();
	rsa->e=BN_new();
	rsa->d=BN_new();
	
	for(i=0;i<128;i++)
	filebuffer1[i]&=0x00;
	

/*************************************
READ THE FILE & SAVE IN THE BUFFER
*************************************/
if((fp=fopen(filename,"rb"))==NULL)
{
	fprintf(stderr,"{cannot open file:verify}\n");
	exit(1);
}
count=0;
while(fread(&ch,1,1,fp))
{
	if(ch!=EOF && ch!=0x0a)	
	{
	if((ch>0x40 && ch<0x47) || (ch>0X60 && ch<0X67) || (ch>0X2F && ch<0X3a))
	{
	}
	else
	{
	fprintf(stderr,"%s is not generated by the sign command)\n",filename);
	exit(1);
	}
	}
	count++;
}
//printf("no. of characters in file =%d\n",count);
fclose(fp);
if((fp=fopen(filename,"rb"))==NULL)
{
	fprintf(stderr,"{cannot open file:verify}\n");
	exit(1);
}
filebuffer=(char *)malloc((count-1)*sizeof(char));
count=0;
while(fread(&ch,1,1,fp))
{
filebuffer[count]=ch;
count++;
}
//printf("%s\n",filebuffer);
k=0;
for(i=0;i<256;i=i+2)
{
	temp=temp&0x00;
//	printf("f=%02x ",(int)filebuffer[i]%16);
	if(filebuffer[i]>=0x61 && filebuffer[i]<=0x66)
	temp=(int)filebuffer[i]-87;
	else
	temp=(int)filebuffer[i]-48;
//	printf("t0=%02x ",temp);
	temp=temp<<4;
	temp1=temp1&0x00;
//	printf("t=%02x ",temp);
	if(filebuffer[i+1]>=0x61 && filebuffer[i+1]<=0x66)
	temp1=(int)filebuffer[i+1]-87;
	else
	temp1=(int)filebuffer[i+1]-48;
	temp1=temp1 & 0x0f;	
//	printf("t1=%02x\n",temp1);
	temp=temp | temp1;
	filebuffer1[k]=temp;
	k++;
}
/*printf("length of filebuffer is %d\n",strlen(filebuffer1));
for(i=0;i<128;i++)
printf("%02x",filebuffer1[i]);*/
	
	
	
		

//printf("filebuffer is %s\n",filebuffer);	


/*************************************
SET d,e and n values from secret.c
*************************************/

/*j=0;
for(i=0;i<256;i+=2)
{
sprintf(tmp,"%.02x",HW4_rsa_private_key[j]);
tmp[2]='\0';
buff[i]=tmp[0];
buff[i+1]=tmp[1];
j++;
}
count=BN_hex2bn(&rsa->d,buff);
//printf("count=%d\n",count);
for(i=0;i<128;i++)
printf("%c",buff[i]);
printf("\n");
//printf("return private key size=%d\n",count);
//BN_copy(rsa->d,b1);
//SC_public_query(NULL,buff1,buff);*/


/*for(i=0;i<128;i++)
printf("%c",buff[i]);
printf("\n");
for(i=0;i<6;i++)
printf("%c",buff1[i]);
printf("strlen(buff1)=%d\n",strlen(buff1));

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
//printf("return public modulus size=%d\n",count);

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
printf("count=%d\n",count);
//printf("return public exponent size=%d\n",count);
count=BN_hex2bn(&rsa->n,buff);
printf("count=%d\n",count);
count=BN_hex2bn(&rsa->e,buff1);
printf("count=%d\n",count);*/

//SC_public_size_query(NULL,&size_of_exponent,&size_of_modulus);



SmartcardState pss=(SmartcardState)NULL;
void *handle=NULL;
//void *vp =NULL;
SC_Init_Func *pfn_init=NULL;
SC_Cleanup_Func *pfn_cleanup=NULL;
SC_GetSignatureSize_Func *pfn_getsigsize=NULL;
SC_PublicSizeQuery_Func *pfn_sc_public_size_query=NULL;

handle = dlopen(sopath, RTLD_NOW|RTLD_GLOBAL);
if (handle==NULL)
{
//fprintf(stderr, "%s\n",sopath);
fprintf(stderr, "%s\n",
dlerror());
fprintf(stderr,"dlopen() fail\n");
exit(1);
}
pfn_init = (SC_Init_Func*)dlsym(handle,"SC_init");


pfn_cleanup = (SC_Cleanup_Func*)dlsym(handle,"SC_cleanup");


pfn_getsigsize = (SC_GetSignatureSize_Func*)dlsym(handle,"SC_get_signature_size");

pfn_sc_public_size_query = (SC_PublicSizeQuery_Func *)dlsym(handle,"SC_public_size_query");

if(pfn_init==NULL || pfn_cleanup==NULL || pfn_getsigsize==NULL || pfn_sc_public_size_query==NULL)
{
fprintf(stderr,"(cannot find smartcard functions in this library)\n");
exit(1);
}


pss = (pfn_init)();
//printf("sig size is %d\n", (pfn_getsigsize)(pss));
//int sz=(pfn_getsigsize)(pss);

(pfn_sc_public_size_query) (pss,&size_of_exponent,&size_of_modulus);


//printf("size of exponent=%d   size of modulus=%d\n",size_of_exponent,size_of_modulus);

exponent=(char *)malloc((size_of_exponent)*sizeof(char));
modulus=(char *)malloc((size_of_modulus)*sizeof(char));

//SC_public_query(NULL,exponent,modulus);



SC_PublicQuery_Func *pfn_public_query=NULL;

pfn_public_query= (SC_PublicQuery_Func *)dlsym(handle,"SC_public_query");

pss = (pfn_init)();
//printf("sig size is %d\n", (pfn_getsigsize)(pss));

(pfn_public_query) (pss,exponent,modulus);

(pfn_cleanup)(pss);
dlclose(handle);


/*for(i=0;i<size_of_exponent;i++)
{
	printf("%02x ",exponent[i]);
}
for(i=0;i<size_of_modulus;i++)
{
	printf("%02x ",modulus[i]);
}*/

//exponent1=(unsigned char *)malloc((size_of_exponent*2)*sizeof(unsigned char));
//modulus1=(unsigned char *)malloc((size_of_modulus*2)*sizeof(unsigned char));

	j=0;
	for(i=0;i<(size_of_modulus*2);i+=2)
        {
        sprintf(tmp,"%.02x",modulus[j]);
	modulus1[i]=tmp[0];
	modulus1[i+1]=tmp[1];
        tmp[2]='\0';
//	printf("modulus1=%c\n",modulus1[i]);
//	printf("tmpm=%s\n",tmp);
        j++;
        }
	modulus1[i]='\0';

        j=0;
        for(i=0;i<(size_of_exponent*2);i+=2)
        {
        sprintf(tmp,"%.02x",exponent[j]);
	exponent1[i]=tmp[0];
	exponent1[i+1]=tmp[1];	
        tmp[2]='\0';
//	printf("exponent=%c\n",exponent1[i]);
//	printf("tmpe=%s\n",tmp);
        j++;
	}
	exponent1[i]='\0';

//printf("\nexponent=%s\n",exponent1);
//printf("\nmodulus=%s\n",modulus1);

count=BN_hex2bn(&rsa->n,modulus1);
//printf("count=%d",count);

count=BN_hex2bn(&rsa->e,exponent1);
//printf("count=%d",count);



/************************************
VERIFY THE FILENAME OF THE FILE 
************************************/


	to=(char *)malloc(20*sizeof(char));
	i=RSA_public_decrypt(128,filebuffer1,to,rsa,RSA_PKCS1_PADDING);	

	for(i=0;i<20;i++)
	printf("%02x",to[i]);
	printf("\n");
	return 1;
}
	

