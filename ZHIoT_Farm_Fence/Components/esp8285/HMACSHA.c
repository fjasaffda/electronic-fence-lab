
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HMACSHA.h"

void truncate
(
	char* d1, /* data to be truncated */
	char* d2, /* truncated data */
	int len /* length in bytes to keep */
)
{
	int i ;
	for (i = 0 ; i < len ; i++)
	{
		d2[i] = d1[i];
	}
}

void hmac_sha256
(
 const char* k, /* secret key */
 int lk, /* length of the key in bytes */
 const char* d, /* data */
 int ld, /* length of data in bytes */
 char* out, /* output buffer, at least "t" bytes */
 int* t
 )
{
	SHA256Context ictx, octx ;
	char isha[SHA256_HASH_SIZE], osha[SHA256_HASH_SIZE] ;
	char key[SHA256_HASH_SIZE] ;
	char buf[SHA_BLOCKSIZE] ;
	int i ;

	if (lk > SHA_BLOCKSIZE) {

		SHA256Context tctx ;

		SHA256Init(&tctx) ;
		SHA256Update(&tctx, k, lk) ;
		SHA256Final(&tctx, (uint8_t *)key) ;

		k = key ;
		lk = SHA256_HASH_SIZE ;
	}

	/**** Inner Digest ****/

	SHA256Init(&ictx) ;

	/* Pad the key for inner digest */
	for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x36 ;
	for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x36 ;

	SHA256Update(&ictx, buf, SHA_BLOCKSIZE) ;
	SHA256Update(&ictx, d, ld) ;

	SHA256Final(&ictx, (uint8_t *)isha) ;

	/**** Outter Digest ****/

	SHA256Init(&octx) ;

	/* Pad the key for outter digest */

	for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x5C ;
	for (i = lk ; i < SHA_BLOCKSIZE ; ++i) buf[i] = 0x5C ;

	SHA256Update(&octx, buf, SHA_BLOCKSIZE);
	SHA256Update(&octx, isha, SHA256_HASH_SIZE);
	SHA256Final(&octx, (uint8_t *)osha) ;

	/* truncate and print the results */
	*t = *t > SHA256_HASH_SIZE ? SHA256_HASH_SIZE : *t;
	truncate(osha, out, *t);
}





