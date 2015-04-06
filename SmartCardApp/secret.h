#ifndef _HW4_SECRET_H_
#define _HW4_SECRET_H_

#ifndef SHA_DIGEST_LENGTH
#include <openssl/sha.h>
#endif /* ~SHA_DIGEST_LENGTH */

/*
 * 1,024-bit RSA private key
 * HW4_rsa_private_key_size should be 128
 */
static unsigned char HW4_rsa_private_key[];
static int HW4_rsa_private_key_size;

/*
 * RSA public exponent, must be numerically equal 65,537
 * This means that HW4_rsa_public_exponent[] should be { 0x01,0x00,0x01 }
 *         and HW4_rsa_public_exponent_size should be 3.
 */
static unsigned char HW4_rsa_public_exponent[];
static int HW4_rsa_public_exponent_size;

/*
 * 1,024-bit RSA public modulus
 * HW4_rsa_public_modulus_size should be 128
 */
static unsigned char HW4_rsa_public_modulus[];
static int HW4_rsa_public_modulus_size;

/*
 * two sets of 160-bit random bit patterns
 */
static unsigned char HW4_random_bits_1[SHA_DIGEST_LENGTH];
static unsigned char HW4_random_bits_2[SHA_DIGEST_LENGTH];

#endif /*_HW4_SECRET_H_*/
