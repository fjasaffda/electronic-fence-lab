#ifndef __HMACSHA_H
#define __HMACSHA_H

#include "sha256.h"

void hmac_sha256(const char* k,int lk,const char* d,int ld,char* out,int* t);

#endif
