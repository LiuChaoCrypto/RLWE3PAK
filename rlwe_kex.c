/* This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * See LICENSE for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rlwe_kex.h"

#include "fft.h"
#include "rlwe.h"
#include "rlwe_rand.h"
#include "getmd5.h"
#include "ringsub.h"
//-------------------------------test code---------------------------------------------------
//--------------------------------------------------
//---------------------------------------------------

int ThreePAK_server_first(const uint32_t *a, uint32_t sf[1024], uint32_t sg[1024], uint32_t bA[1024],
        uint32_t bB[1024], uint32_t mA[1024], uint32_t mB[1024], const uint32_t gamma[1024],
        const uint32_t eta[1024], FFT_CTX *ctx)
{
	int ret;
	uint32_t ef[1024],eg[1024];
	RAND_CTX rand_ctx;
	ret = RAND_CHOICE_init(&rand_ctx);
	if (!ret) {
		return ret;
	}
#if CONSTANT_TIME
	rlwe_sample_ct(sf, &rand_ctx);
	rlwe_sample_ct(ef, &rand_ctx);
	rlwe_sample_ct(sg, &rand_ctx);
	rlwe_sample_ct(eg, &rand_ctx);
#else
	rlwe_sample(sf, &rand_ctx);
	rlwe_sample(sg, &rand_ctx);
	rlwe_sample(ef, &rand_ctx);
	rlwe_sample(eg, &rand_ctx);
#endif
	rlwe_key_gen(bA, a, sf, ef, ctx);
	rlwe_key_gen(bB, a, sg, eg, ctx);

    ringsub(mA,bA,gamma);
    ringsub(mB,bB,eta);

	rlwe_memset_volatile(ef, 0, 1024 * sizeof(uint32_t));
	rlwe_memset_volatile(eg, 0, 1024 * sizeof(uint32_t));
	RAND_CHOICE_cleanup(&rand_ctx);
	return ret;
}


int ThreePAK_clientB_second(const uint32_t *a, uint32_t sB[1024], uint32_t pB[1024],const uint32_t mB[1024],
        FFT_CTX *ctx,char *pw2, uint64_t wB[16], uint64_t thetaB[16], char *kBS)
{
    uint32_t gamma[1024],bB[1024],v1[1024];
    full_get_md5(pw2,gamma,1);

    FFT_add(bB,mB,gamma);
	int ret;
	uint32_t eB[1024],eBB[1024];
	RAND_CTX rand_ctx;
	ret = RAND_CHOICE_init(&rand_ctx);
	if (!ret) {
		return ret;
	}
#if CONSTANT_TIME
	rlwe_sample_ct(sB, &rand_ctx);
	rlwe_sample_ct(eB, &rand_ctx);
	rlwe_sample_ct(eBB, &rand_ctx);
#else
	rlwe_sample(sB, &rand_ctx);
	rlwe_sample(eB, &rand_ctx);
	rlwe_sample(eBB, &rand_ctx);
#endif
	rlwe_key_gen(pB, a, sB, eB, ctx);

	rlwe_key_gen(v1, bB, sB, eBB, ctx);

#if CONSTANT_TIME
	rlwe_crossround2_ct(wB, v1, &rand_ctx);
	rlwe_round2_ct(thetaB, v1);
#else
	rlwe_crossround2(wB, v1, &rand_ctx);
	rlwe_round2(thetaB, v1);
#endif

    md5_hash_1ring_1k_to_hashk(bB,thetaB,2,kBS);

	rlwe_memset_volatile(eB, 0, 1024 * sizeof(uint32_t));
	rlwe_memset_volatile(eBB, 0, 1024 * sizeof(uint32_t));
	rlwe_memset_volatile(v1, 0, 1024 * sizeof(uint32_t));
	RAND_CHOICE_cleanup(&rand_ctx);
	return ret;
}



int ThreePAK_server_second(const uint32_t pA[1024], const uint32_t pB[1024], const char *kAS,
        const char *kBS, const uint64_t wA[16], const uint64_t wB[16],
        const uint32_t sf[1024], const uint32_t sg[1024],const uint32_t bA[1024], const uint32_t bB[1024],
        uint32_t cA[1024], uint32_t cB[1024], char *kSA, char *kSB, FFT_CTX *ctx)
{

    char kAS_temp[36],kBS_temp[36];

    uint64_t thetaA_temp[16],thetaB_temp[16];
    uint32_t mulA[1024],mulB[1024],sS[1024],e1[1024],e2[1024];
	FFT_mul(mulA, pA, sf, ctx);
#if CONSTANT_TIME
	rlwe_rec_ct(thetaA_temp, mulA, wA);
#else
	rlwe_rec(thetaA_temp, mulA, wA);
#endif
	rlwe_memset_volatile(mulA, 0, 1024 * sizeof(uint32_t));
    md5_hash_1ring_1k_to_hashk(bA,thetaA_temp,2,kAS_temp);
	int keys_match=1;
    for (int i = 0; i < 32; i++) {
		keys_match &= (kAS_temp[i] == kAS[i]);
	}
    if (keys_match) {
//		printf("Client A hash match\n");
	} else {
		printf("In Server A's hash kAS don't match the input hash! :(\n");
		return -1;
    }

	FFT_mul(mulB, pB, sg, ctx);
#if CONSTANT_TIME
	rlwe_rec_ct(thetaB_temp, mulB, wB);
#else
	rlwe_rec(thetaB_temp, mulB, wB);
#endif
	rlwe_memset_volatile(mulB, 0, 1024 * sizeof(uint32_t));
    md5_hash_1ring_1k_to_hashk(bB,thetaB_temp,2,kBS_temp);
	keys_match=1;
    for (int i = 0; i < 32; i++) {
		keys_match &= (kBS_temp[i] == kBS[i]);
	}
    if (keys_match) {
//		printf("Client A hash match\n");
	} else {
		printf("In Server B's hash kBS don't match the input hash! :(\n");
		return -1;
    }

	int ret;
	RAND_CTX rand_ctx;
	ret = RAND_CHOICE_init(&rand_ctx);
	if (!ret) {
		return ret;
	}
#if CONSTANT_TIME
	rlwe_sample_ct(sS, &rand_ctx);
	rlwe_sample_ct(e1, &rand_ctx);
	rlwe_sample_ct(e2, &rand_ctx);
#else
	rlwe_sample(sS, &rand_ctx);
	rlwe_sample(e1, &rand_ctx);
	rlwe_sample(e2, &rand_ctx);
#endif
	rlwe_key_gen(cB, pA, sS, e1, ctx);
	rlwe_key_gen(cA, pB, sS, e2, ctx);

	rlwe_memset_volatile(sS, 0, 1024 * sizeof(uint32_t));
	rlwe_memset_volatile(e1, 0, 1024 * sizeof(uint32_t));
	rlwe_memset_volatile(e2, 0, 1024 * sizeof(uint32_t));


    md5_hash_1ring_1k_to_hashk(pB,thetaA_temp,2,kSA);
    md5_hash_1ring_1k_to_hashk(pA,thetaB_temp,2,kSB);
	return 1;
}

int ThreePAK_clientB_third(const uint32_t pA[1024],const uint32_t pB[1024], const uint32_t mA[1024],
        const uint32_t mB[1024], const uint32_t cB[1024], const char *kSB, const uint32_t sB[1024],
        const uint64_t thetaB[16],uint64_t w[16],char *hashk,char *hashkkk, uint64_t theta[16],FFT_CTX *ctx)
{


    char kSB_temp[36];
    md5_hash_1ring_1k_to_hashk(pA,thetaB,2,kSB_temp);
	int keys_match=1;
    for (int i = 0; i < 32; i++) {
		keys_match &= (kSB_temp[i] == kSB[i]);
	}
    if (keys_match) {
//		printf("Client A hash match\n");
	} else {
		printf("In Client B, S's hash kSB don't match the input hash! :(\n");
		return -1;
    }
    int ret;
	uint32_t vB[1024];
	uint32_t eBBB[1024];
	RAND_CTX rand_ctx;
	ret = RAND_CHOICE_init(&rand_ctx);
	if (!ret) {
		return ret;
	}
#if CONSTANT_TIME
	rlwe_sample_ct(eBBB, &rand_ctx);
#else
	rlwe_sample(eBBB, &rand_ctx);
#endif
	rlwe_key_gen(vB, cB, sB, eBBB, ctx);
#if CONSTANT_TIME
	rlwe_crossround2_ct(w, vB, &rand_ctx);
	rlwe_round2_ct(theta, vB);
#else
	rlwe_crossround2(w, vB, &rand_ctx);
	rlwe_round2(theta, vB);
#endif
	rlwe_memset_volatile(vB, 0, 1024 * sizeof(uint32_t));
	rlwe_memset_volatile(eBBB, 0, 1024 * sizeof(uint32_t));



    md5_hash_4ring_1k_to_hashk(mA,mB,pA,pB,theta,3,hashk);
    md5_hash_4ring_1k_to_hashk(mA,mB,pA,pB,theta,4,hashkkk);
    return ret;
}

int ThreePAK_clientA_second(const uint32_t pA[1024], const uint32_t pB[1024], const uint32_t mA[1024],
        const uint32_t mB[1024], const uint32_t cA[1024],const char *kSA, const uint32_t sA[1024], const uint64_t thetaA[16],const char *hashk,
        const uint64_t w[16], char *hashkk, char*skA,FFT_CTX *ctx)
{

    char kSA_temp[36];
    md5_hash_1ring_1k_to_hashk(pB,thetaA,2,kSA_temp);
	int keys_match=1;
    for (int i = 0; i < 32; i++) {
		keys_match &= (kSA_temp[i] == kSA[i]);
	}
    if (keys_match) {
//		printf("Client A hash match\n");
	} else {
		printf("In Client A, S's hash kSA don't match the input hash! :(\n");
		return -1;
    }


    uint64_t theta[16];
    uint32_t vA[1024];
    FFT_mul(vA, cA, sA, ctx);
#if CONSTANT_TIME
	rlwe_rec_ct(theta, vA, w);
#else
	rlwe_rec(theta, vA, w);
#endif


	rlwe_memset_volatile(vA, 0, 1024 * sizeof(uint32_t));

    char hashk_temp[36];
    md5_hash_4ring_1k_to_hashk(mA,mB,pA,pB,theta,3,hashk_temp);

    keys_match=1;
    for (int i = 0; i < 32; i++) {
		keys_match &= (hashk_temp[i] == hashk[i]);
	}

    if (keys_match) {
//		printf("Client A hash match\n");
	} else {
		printf("Client A's hash don't match the CLient B input hash! :(\n");
		return -1;
    }

    md5_hash_4ring_1k_to_hashk(mA,mB,pA,pB,theta,4,hashkk);
    md5_hash_4ring_1k_to_hashk(mA,mB,pA,pB,theta,5,skA);
	return 1;
}

int ThreePAK_clientB_finish(const uint32_t mA[1024],const uint32_t mB[1024],
        const uint32_t pA[1024], const uint32_t pB[1024],
        const uint64_t theta[16], char *skB,char *hashkk,char *hashkkk)
{

	int keys_match=1;
    for (int i = 0; i < 32; i++) {
		keys_match &= (hashkk[i] == hashkkk[i]);
	}

    if (keys_match) {
//		printf("Client B hash match\n");
	} else {
		printf("Client B's hash don't match the input hash! :(\n");
		return -1;
    }

    md5_hash_4ring_1k_to_hashk(mA,mB,pA,pB,theta,5,skB);
    return 1;
}

