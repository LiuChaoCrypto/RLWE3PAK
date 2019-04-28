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
#include <string.h>
#include <stdlib.h>

#include "rlwe_kex.h"

#include "fft.h"
#include "rlwe_a.h"
#include "rlwe_rand.h"
#include <openssl/md5.h>
#include "ringsub.h"
#include "Hash_pw_A_1.h"
#include "Hash_pw_B_1.h"
#include "Hash_pw_B_2.h"
#include "Hash_pw_B_3.h"
#include "Hash_pw_A_2.h"
#include "Hash_pw_A_3.h"


#define CHECK_OK(op, val) \
	{ \
		int tmp_ret; \
		tmp_ret = (op); \
		if (tmp_ret != (val)) { \
			fprintf(stderr, "Error (return code %d) at %s:%d\n", tmp_ret, __FILE__, __LINE__); \
			return -1; \
		} \
	}


int main() {

	uint32_t *a = rlwe_a;
    uint32_t *gamma_1=gamma1;
    uint32_t *eta_1=eta1;
    char pw_input_B[10]="BHello";
    char pw_input_A[10]="AHello";
    FFT_CTX ctx;
	if (!FFT_CTX_init(&ctx)) {
		printf("Memory allocation error.");
		return -1;
	}




	uint32_t ThreePAK_sf[1024];
	uint32_t ThreePAK_sg[1024];
	uint32_t ThreePAK_bA[1024];
	uint32_t ThreePAK_bB[1024];
	uint32_t ThreePAK_mA[1024];
	uint32_t ThreePAK_mB[1024];


	uint32_t ThreePAK_pB[1024];
	uint32_t ThreePAK_sB[1024];
    char ThreePAK_kBS[36];
	uint64_t ThreePAK_wB[16];

	uint32_t ThreePAK_pA[1024];
	uint32_t ThreePAK_sA[1024];
    char ThreePAK_kAS[36];
	uint64_t ThreePAK_wA[16];

    uint32_t ThreePAK_cA[1024];
	uint32_t ThreePAK_cB[1024];

    char ThreePAK_kSA[36];
    char ThreePAK_kSB[36];


	uint64_t ThreePAK_w[16];

    char ThreePAK_hashk[36];
    char ThreePAK_skB[36];


    char ThreePAK_hashkk[36];
    char ThreePAK_hashkkk[36];
    char ThreePAK_skA[36];

	uint64_t ThreePAK_thetaB[16];
	uint64_t ThreePAK_thetaA[16];
	uint64_t ThreePAK_theta[16];



    CHECK_OK(ThreePAK_server_first(a,ThreePAK_sf,ThreePAK_sg,ThreePAK_bA,
                ThreePAK_bB,ThreePAK_mA,ThreePAK_mB,gamma_1,eta_1,&ctx),1);

    CHECK_OK(ThreePAK_clientB_second(a,ThreePAK_sB,ThreePAK_pB,ThreePAK_mB,
                &ctx,pw_input_B,ThreePAK_wB,ThreePAK_thetaB,ThreePAK_kBS),1);
    CHECK_OK(ThreePAK_clientB_second(a,ThreePAK_sA,ThreePAK_pA,ThreePAK_mA,
                &ctx,pw_input_A,ThreePAK_wA,ThreePAK_thetaA,ThreePAK_kAS),1);

    CHECK_OK(ThreePAK_server_second(ThreePAK_pA,ThreePAK_pB,ThreePAK_kAS,
                ThreePAK_kBS,ThreePAK_wA,ThreePAK_wB,
                ThreePAK_sf,ThreePAK_sg,ThreePAK_bA,ThreePAK_bB,
                ThreePAK_cA,ThreePAK_cB,ThreePAK_kSA,ThreePAK_kSB,&ctx),1);

    CHECK_OK(ThreePAK_clientB_third(ThreePAK_pA,ThreePAK_pB,ThreePAK_mA,
                ThreePAK_mB,ThreePAK_cB,ThreePAK_kSB,ThreePAK_sB,
                ThreePAK_thetaB,ThreePAK_w,ThreePAK_hashk,ThreePAK_hashkkk,ThreePAK_theta,&ctx),1);

    CHECK_OK(ThreePAK_clientA_second(ThreePAK_pA,ThreePAK_pB,ThreePAK_mA,
                ThreePAK_mB,ThreePAK_cA,ThreePAK_kSA,ThreePAK_sA,
                ThreePAK_thetaA,ThreePAK_hashk,ThreePAK_w,ThreePAK_hashkk,ThreePAK_skA,&ctx),1);


    CHECK_OK(ThreePAK_clientB_finish(ThreePAK_mA,ThreePAK_mB,ThreePAK_pA,
                ThreePAK_pB,ThreePAK_theta,ThreePAK_skB,ThreePAK_hashkk,ThreePAK_hashkkk),1);








      printf("Client A password:%s",pw_input_A);
      printf("\nClient B password:%s",pw_input_B);
      printf("\nClient A final hash:%s",ThreePAK_skA);
      printf("\nClient B final hash:%s\n",ThreePAK_skB);
      int keys_match3=1;


	for (int i = 0; i < 32; i++) {
		keys_match3 &= (ThreePAK_skA[i] == ThreePAK_skB[i]);
	}
	if (keys_match3) {
		printf("New3PAK Keys match.\n");
	} else {
		printf("New3PAK Keys don't match! :(\n");
		FFT_CTX_free(&ctx);
		return -1;
	}


    printf("Success!");

	FFT_CTX_clear(&ctx);
	FFT_CTX_free(&ctx);

	return 0;

}
