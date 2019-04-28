#ifndef _GETMD5__H_
#define _GETMD5__H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/md5.h>

int get_md5(char *input,char *output);
int one_get_md5(char *input,uint32_t out_of_4ring[4],unsigned char d[16]);
void full_get_md5(char *input,uint32_t ring[1024],int hashnumber);
void md5_hash_4ring_1k_to_hashk(const uint32_t m_A[1024],const uint32_t m_B[1024],const uint32_t p_A[1024], const uint32_t p_B[1024],const uint64_t k[16],int hashnumber,char* hashkout);
void md5_hash_1ring_1k_to_hashk(const uint32_t p[1024],const uint64_t k[16],int hashnumber,char* hashkout);


#endif
