#include "header.h"
long d0,d1,d2,d3;
int secret()
{
	int i,j,count;
	unsigned char *HW4_rsa_private_key,*HW4_rsa_public_exponent,*HW4_rsa_public_modulus;
	unsigned char HW4_random_bits_1[20],HW4_random_bits_2[20];
	void cb(int,int,void *);
	RSA *rsa;

	printf("#ifndef SHA_DIGEST_LENGTH\n");
	printf("#include <openssl/sha.h>\n");
	printf("#endif /* ~SHA_DIGEST_LENGTH */\n\n");
	

	fprintf(stderr,"generating parameters (1024 bits)\n");
	rsa=RSA_generate_key(1024,65537,cb,"GQ");



/*********************************************************
GENERATE RSA PRIVATE KEY
*********************************************************/
	count=0;	
	i=BN_num_bytes(rsa->d);
	printf("static unsigned char HW4_rsa_private_key[] = {\n  ");
	HW4_rsa_private_key=(unsigned char*)malloc(i*1);	
	BN_bn2bin(rsa->d,HW4_rsa_private_key);
	for(j=0;j<i;j++)
	{
	if(count==8)
	{
	printf("\n  ");
	count=0;
	}
	if(j==i-1)
	{
	printf("0x%02x\n",HW4_rsa_private_key[j]);
	break;
	}
	printf("0x%02x,",HW4_rsa_private_key[j]);
	count++;
	}
	printf("};\n");
	printf("static int HW4_rsa_private_key_size=128;\n\n");

/*****************************************************
GENERATE RSA PUBLIC EXPONENT
*****************************************************/
	count=0;	
        i=BN_num_bytes(rsa->e);
        HW4_rsa_public_exponent=(unsigned char*)malloc(i*1);
	printf("static unsigned char HW4_rsa_public_exponent[] = {\n  ");
        BN_bn2bin(rsa->e,HW4_rsa_public_exponent);
        for(j=0;j<i;j++)
	{
	if(count==8)
	{
	printf("\n  ");
	count=0;
	}
	if(j==i-1)
	{
        printf("0x%02x\n",HW4_rsa_public_exponent[j]);
	break;
	}
        printf("0x%02x,",HW4_rsa_public_exponent[j]);
	count++;
	}
        printf("};\n");
	printf("static int HW4_rsa_public_exponent_size=3;\n\n");

/**************************************************
GENERATE RSA PUBLIC MODULUS
**************************************************/
	count=0;	
        i=BN_num_bytes(rsa->n);
	printf("static unsigned char HW4_rsa_public_modulus[] = {\n  ");
        HW4_rsa_public_modulus=(unsigned char*)malloc(i*1);
        BN_bn2bin(rsa->n,HW4_rsa_public_modulus);
        for(j=0;j<i;j++)
	{
	if(count==8)
	{
	printf("\n  ");
	count=0;
	}
	if(j==i-1)
	{
        printf("0x%02x\n",HW4_rsa_public_modulus[j]);
	break;
	}
        printf("0x%02x,",HW4_rsa_public_modulus[j]);
	count++;
	}
        printf("};\n");
	printf("static int HW4_rsa_public_modulus_size=128;\n\n");



/************************************************
GENERATE RANDOM BITS 1
************************************************/
	count=0;
        if(RAND_bytes(HW4_random_bits_1,20)==0)
        {
                fprintf(stderr,"Error obtaining 20 bytes of random from OPENSSL");
        }
	printf("static unsigned char HW4_random_bits_1[SHA_DIGEST_LENGTH] = {\n  ");
        for(i=0;i<20;i++)
	{
	if(count==8)
	{
	printf("\n  ");
	count=0;
	}
	if(i==19)	
	{
        printf("0x%02x\n",HW4_random_bits_1[i]);
	break;
	}
        printf("0x%02x,",HW4_random_bits_1[i]);
	count++;
	}
        printf("};\n\n");


/**********************************************
GENERATE RANDOM BITS 2
**********************************************/
	count=0;
        if(RAND_bytes(HW4_random_bits_2,20)==0)
        {
                fprintf(stderr,"Error obtaining 20 bytes of random numbers from OPENSSL\n");
        }
	printf("static unsigned char HW4_random_bits_2[SHA_DIGEST_LENGTH] = {\n  ");
        for(i=0;i<20;i++)
	{
	if(count==8)
	{
	printf("\n  ");
	count=0;
	}
	if(i==19)
	{
        printf("0x%02x\n",HW4_random_bits_2[i]);
	break;
	}
        printf("0x%02x,",HW4_random_bits_2[i]);
	count++;
	}
        printf("};\n");

	return 1;
}

void cb(int n1,int n2,void *chr)	
{
	switch (n1) {
	case 0:
		d0++;
		fprintf(stderr,"Generating the i-th potential prime number\n");
		/*fprintf(stderr, "%s %d %d %lu\r\n", (char *)chr, n1, n2,
		    d0);*/
		break;
	case 1:
		d1++;
		fprintf(stderr,"number is tested for primality\n");
		/*fprintf(stderr, "%s\t\t%d %d %lu\r\n", (char *)chr, n1,
		    n2, d1);*/
		break;
	case 2:
		d2++;
		fprintf(stderr,"Random numbe generated prime is rejected\n");
		/*fprintf(stderr, "%s\t\t\t\t%d %d %lu\r\n", (char *)chr,
		    n1, n2, d2);*/
		break;
	case 3:
		d3++;
		if(n2==1)
		fprintf(stderr,"Random number q is found with p-1 relatively prime to e\n");
		else if(n2==0)
		fprintf(stderr,"Random number p is found with p-1 relatively prime to e\n");
		break;
	}
}
