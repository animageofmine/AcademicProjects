#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

#include <netinet/in.h>

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/rand.h>

#include "defs.h"
#include "scintrf.h"
#include "secret.c"

/* ------------------- SC_init() ------------------- */

SmartcardState SC_init(void)
{
    unsigned char *sha1_buf=NULL;

    /*
     * Here's some weird code.  You should replace this with
     * something useful if you'd like.
     */
    sha1_buf = (unsigned char *)malloc(SHA_DIGEST_LENGTH);
    if (sha1_buf != NULL) {
        memset(sha1_buf, 0, SHA_DIGEST_LENGTH);
        SHA1(HW4_rsa_private_key, HW4_rsa_private_key_size, sha1_buf);
    }
    return (SmartcardState)sha1_buf;
}

/* ------------------- SC_cleanup() ------------------- */

void SC_cleanup(SmartcardState pSS)
{
    unsigned char *sha1_buf=(unsigned char *)pSS;

    /*
     * Here's some weird code.  You should replace this with
     * something useful if you'd like.
     */
    if (sha1_buf != NULL) {
        free(sha1_buf);
    }
}

/* ------------------- SC_get_signature_size() ------------------- */

int SC_get_signature_size(SmartcardState pSS)
{
    unsigned char *state_buf=(unsigned char *)pSS;
    unsigned char sha1_buf[SHA_DIGEST_LENGTH];

    {
        /*
         * Here's some weird code.  You should replace this with
         * something useful if you'd like.
         */
        if (state_buf == NULL) return (-1);

        memset(sha1_buf, 0, SHA_DIGEST_LENGTH);
        SHA1(HW4_rsa_private_key, HW4_rsa_private_key_size, sha1_buf);

        if (memcmp(sha1_buf, state_buf, SHA_DIGEST_LENGTH) != 0) {
            /* this shouldn't happen */
            return (-1);
        }
    }
    return HW4_rsa_private_key_size;
}

/*-----------------SC_sign--------------------------------*/

int SC_sign(SmartcardState pSS,unsigned char sha1_buf[SHA_DIGEST_LENGTH],unsigned char *buf_return)
{
//printf("in smartcard\n");
/*************************************
SET d,e and n values from secret.c
*************************************/
int i,j,count;
RSA *rsa;
char tmp[3],buff[512],buff1[512];

   if((rsa=RSA_new())==NULL)
        {
                fprintf(stderr,"{key_new: RSA NEW failed}\n");
                exit(1);
        }

        rsa->n=BN_new();
        rsa->e=BN_new();
        rsa->d=BN_new();


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
/*printf("\ncount=%d",count);
//BN_copy(rsa->d,b1);
printf("the private key is:\n:");
for(i=0;i<256;i++)
printf("%c",buff[i]);
printf("\n");*/



j=0;
for(i=0;i<256;i+=2)
{
sprintf(tmp,"%.02x",HW4_rsa_public_modulus[j]);
tmp[2]='\0';
buff[i]=tmp[0];
buff[i+1]=tmp[1];
j++;
}
/*printf("public modulus\n");
for(i=0;i<256;i++)
printf("%c",buff[i]);
printf("\n");*/
count=BN_hex2bn(&rsa->n,buff);
//printf("\ncount=%d",count);

j=0;
for(i=0;i<6;i+=2)
{
sprintf(tmp,"%.02x",HW4_rsa_public_exponent[j]);
tmp[2]='\0';
buff1[i]=tmp[0];
buff1[i+1]=tmp[1];
j++;
}
/*printf("public exponent\n");
for(i=0;i<6;i++)
printf("%c",buff1[i]);
printf("\n");*/
count=BN_hex2bn(&rsa->e,buff1);
/*printf("\ncount=%d",count);
//printf("count=%d\n",count);
printf("sha1_buf=\n");
for(i=0;i<20;i++)
printf("%02x",sha1_buf[i]);
printf("\n");*/
/************************************
SIGN THE DIGEST OF THE FILE
************************************/
        RSA_private_encrypt(20,sha1_buf,(unsigned char *)buf_return,rsa,RSA_PKCS1_PADDING);
//      printf("\nTHE SIGN IS \n");
//       for(i=0;i<128;i++)
 //       printf("%02x",(unsigned char)buf_return[i]);
//        printf("%02x",buf_return[i]);
        return 1;
}


/***********************************************************************************
IMPLEMENT SC_3des_key_gen
***********************************************************************************/

int SC_3des_key_gen(SmartcardState pSS,unsigned char sha1_buf[SHA_DIGEST_LENGTH],unsigned char iv[8],unsigned char key1[8],unsigned char key2[8],unsigned char key3[8])
{
	SHA_CTX md;
	int i,j;
	unsigned char X_RB1[20],X_RB2[20];
/******************************************
COMPUTE SHA1 OF X AND RANDOM BITS(RB1) 1
*******************************************/
        SHA1_Init(&md);
        SHA1_Update(&md,sha1_buf,20);
        SHA1_Update(&md,HW4_random_bits_1,20);
        SHA1_Final(X_RB1,&md);
        for(i=0;i<20;i++)
        {
                //printf("%02x",X_RB1[i]);
        }
        //printf("\n");

/******************************************
COMPUTE SHA1 OF RB1(Y) AND RANDOM BITS 2(RB2)
******************************************/
        SHA1_Init(&md);
        SHA1_Update(&md,X_RB1,20);
        SHA1_Update(&md,HW4_random_bits_2,20);
        SHA1_Final(X_RB2,&md);
        for(i=0;i<20;i++)
        {
                //printf("%02x",X_RB2[i]);
        }
        //printf("\n");

/***************************************
COMPUTER IV AND 3 DES KEYS
***************************************/
        for(i=0;i<8;i++)
        iv[i]=X_RB1[i];
        for(i=8,j=0;i<16;i++,j++)
        key1[j]=X_RB1[i];
        for(i=0;i<8;i++)
        key2[i]=X_RB2[i];
        for(i=8,j=0;i<16;i++,j++)
        key3[j]=X_RB2[i];
	
	return 1;
}

/*******************************************
IMPLEMENT SC_public_size_query
*******************************************/
int SC_public_size_query(SmartcardState pSS,int *p_public_exponent_size_return,int *p_public_modulus_size_return)
{
*p_public_exponent_size_return=HW4_rsa_public_exponent_size;
*p_public_modulus_size_return=HW4_rsa_public_modulus_size;
return 1;
}
	
int SC_public_query(SmartcardState pSS, unsigned char *public_exponent_return, unsigned char *public_modulus_return)
{

	int i;

	for(i=0;i<HW4_rsa_public_modulus_size;i++)
	{
		public_modulus_return[i]=HW4_rsa_public_modulus[i];
	}
	for(i=0;i<HW4_rsa_public_exponent_size;i++)
	{
		public_exponent_return[i]=HW4_rsa_public_exponent[i];
	}

	/*for(i=0;i<HW4_rsa_public_modulus_size;i+=2)
	{
	sprintf(tmp,"%.02x",HW4_rsa_public_modulus[j]);
	tmp[2]='\0';
	public_modulus_return[i]=tmp[0];
	public_modulus_return[i+1]=tmp[1];
	j++;
	}
	//ifor(i=0;i<128;i++)
	//{
	//	printf("%c",public_modulus_return[i]);
	//}

	j=0;
	for(i=0;i<HW4_rsa_public_exponent_size;i+=2)
	{
	sprintf(tmp,"%.02x",HW4_rsa_public_exponent[j]);
	tmp[2]='\0';
	public_exponent_return[i]=tmp[0];
	public_exponent_return[i+1]=tmp[1];
	j++;
	}*/
	return 1;
}


