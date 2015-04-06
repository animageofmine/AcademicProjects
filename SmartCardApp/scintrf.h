#ifndef _SCINTRF_H_
#define _SCINTRF_H_

typedef void *SmartcardState;

typedef SmartcardState (SC_Init_Func)(void);
typedef void (SC_Cleanup_Func)(SmartcardState);
typedef int (SC_GetSignatureSize_Func)(SmartcardState);
typedef int (SC_Sign_Func)(SmartcardState,
        unsigned char sha1_buf[SHA_DIGEST_LENGTH], unsigned char *);
typedef int (SC_3DesKeyGen_Func)(SmartcardState,
        unsigned char sha1_buf[SHA_DIGEST_LENGTH], unsigned char iv[8],
        unsigned char key1[8], unsigned char key2[8], unsigned char key3[8]);
typedef int (SC_PublicSizeQuery_Func)(SmartcardState,
        int *p_public_exponent_size_return, int *p_public_modulus_size_return);
typedef int (SC_PublicQuery_Func)(SmartcardState,
        unsigned char *public_exponent_return,
        unsigned char *public_modulus_return);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern SmartcardState SC_init(void);

extern void SC_cleanup(SmartcardState pSS);

extern int SC_get_signature_size(SmartcardState pSS);

extern int SC_sign(
		SmartcardState pSS,
		unsigned char sha1_buf[SHA_DIGEST_LENGTH],
		unsigned char *buf_return);

extern int SC_3des_key_gen(
		SmartcardState pSS,
		unsigned char sha1_buf[SHA_DIGEST_LENGTH],
		unsigned char iv[8],
		unsigned char key1[8],
		unsigned char key2[8],
		unsigned char key3[8]);

extern int SC_public_size_query(
		SmartcardState pSS,
		int *p_public_exponent_size_return,
		int *p_public_modulus_size_return);

extern int SC_public_query(
		SmartcardState pSS,
		unsigned char *public_exponent_return,
		unsigned char *public_modulus_return);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /*_SCINTRF_H_*/

