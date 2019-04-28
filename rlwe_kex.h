/* This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * See LICENSE for complete information.
 */

#ifndef _RLWE_KEX_H_
#define _RLWE_KEX_H_

#include <stdint.h>

#include "fft.h"


int ThreePAK_server_first(const uint32_t *a, uint32_t sf[1024], uint32_t sg[1024], uint32_t bA[1024],
        uint32_t bB[1024], uint32_t mA[1024], uint32_t mB[1024], const uint32_t gamma[1024],
        const uint32_t eta[1024], FFT_CTX *ctx);
int ThreePAK_clientB_second(const uint32_t *a, uint32_t sB[1024], uint32_t pB[1024], const uint32_t mB[1024],
        FFT_CTX *ctx,char *pw2, uint64_t wB[16], uint64_t thetaB[16], char *kBS);
int ThreePAK_server_second(const uint32_t p_A[1024], const uint32_t p_B[1024], const char *kAS,
        const char *kBS, const uint64_t wA[16], const uint64_t wB[16],
        const uint32_t sf[1024], const uint32_t sg[1024],const uint32_t bA[1024], const uint32_t bB[1024],
        uint32_t cA[1024], uint32_t cB[1024], char *kSA, char *kSB, FFT_CTX *ctx);
int ThreePAK_clientB_third(const uint32_t pA[1024],const uint32_t pB[1024], const uint32_t mA[1024],
        const uint32_t mB[1024], const uint32_t cB[1024], const char *kSB, const uint32_t sB[1024],
        const uint64_t thetaB[16],uint64_t w[16],char *hashk,char *hashkkk, uint64_t theta[16],FFT_CTX *ctx);
int ThreePAK_clientA_second(const uint32_t pA[1024], const uint32_t pB[1024], const uint32_t mA[1024],
        const uint32_t mB[1024], const uint32_t cA[1024],const char *kSA, const uint32_t sA[1024], const uint64_t thetaA[16],const char *hashk,
        const uint64_t w[16], char *hashkk, char*skA,FFT_CTX *ctx);
int ThreePAK_clientB_finish(const uint32_t mA[1024],const uint32_t mB[1024],
        const uint32_t pA[1024], const uint32_t pB[1024],
        const uint64_t theta[16], char *skB,char *hashkk,char *hashkkk);




#endif /* _RLWE_KEX_H_ */
