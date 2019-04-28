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
#include <inttypes.h>
#include <sys/time.h>

#include "rlwe_kex.h"

#include "fft.h"
#include "rlwe.h"
#include "rlwe_a.h"
#include <openssl/md5.h>
#include "ringsub.h"
#include "Hash_pw_A_1.h"
#include "Hash_pw_B_1.h"
#include "Hash_pw_B_2.h"
#include "Hash_pw_B_3.h"
#include "Hash_pw_A_2.h"
#include "Hash_pw_A_3.h"


#define ITERATIONS 10000

#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void) {
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}
#elif defined(__x86_64__)

static __inline__ unsigned long long rdtsc(void) {
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (unsigned long long)lo) | ( ((unsigned long long)hi) << 32 );
}
#endif

#define START_TIMER \
	gettimeofday(&timeval_start, NULL); \
	cycles_start = rdtsc();
#define END_TIMER \
	cycles_end = rdtsc(); \
	gettimeofday(&timeval_end, NULL);
#define PRINT_TIMER_AVG(op_name, it) \
	printf("%-30s %15d %15d %15" PRIu64 "\n", (op_name), (it), \
		(uint32_t) ((timeval_end.tv_usec+1000000*timeval_end.tv_sec) - (timeval_start.tv_usec+1000000*timeval_start.tv_sec)) / (it), \
		(cycles_end - cycles_start) / (it));
#define TIME_OPERATION(op, op_name, it) \
	START_TIMER \
	for (int i = 0; i < (it); i++) { \
		(op); \
	} \
	END_TIMER \
	PRINT_TIMER_AVG(op_name, it)

int main() {

	uint64_t cycles_start, cycles_end;
	struct timeval timeval_start, timeval_end;

	uint32_t s[1024];
	uint32_t e[1024];
	uint32_t b[1024];
	uint64_t k[16];
	uint64_t c[16];
    uint32_t *a=rlwe_a;
    uint32_t *gamma_1=gamma1;
    uint32_t *eta_1=eta1;

    char pw_input_A[10]="AHello";
    char pw_input_B[10]="BHello";



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




	FFT_CTX ctx;
	if (!FFT_CTX_init(&ctx)) {
		printf("Memory allocation error.");
		return -1;
	}

	RAND_CTX rand_ctx;
	if (!RAND_CHOICE_init(&rand_ctx)) {
		printf("Randomness allocation error.");
		return -1;
	}

	printf("%-30s %15s %15s %15s\n", "Operation", "Iterations", "usec (avg)", "cycles (avg)");
	printf("------------------------------------------------------------------------------\n");

#ifdef CONSTANT_TIME
	TIME_OPERATION(rlwe_sample_ct(s, &rand_ctx), "sample_ct", ITERATIONS / 50)
	TIME_OPERATION(FFT_mul(b, rlwe_a, s, &ctx), "FFT_mul", ITERATIONS / 50)
	rlwe_sample_ct(e, &rand_ctx);
	TIME_OPERATION(FFT_add(b, b, e), "FFT_add", ITERATIONS)
	TIME_OPERATION(rlwe_crossround2_ct(c, b, &rand_ctx), "crossround2_ct", ITERATIONS / 10)
	TIME_OPERATION(rlwe_round2_ct(k, b), "round2_ct", ITERATIONS / 10)
	TIME_OPERATION(rlwe_rec_ct(k, b, c), "rec_ct", ITERATIONS)
#else
	TIME_OPERATION(rlwe_sample(s, &rand_ctx), "sample", ITERATIONS / 50)
	TIME_OPERATION(FFT_mul(b, rlwe_a, s, &ctx), "FFT_mul", ITERATIONS / 50)
	rlwe_sample(e, &rand_ctx);
	TIME_OPERATION(FFT_add(b, b, e), "FFT_add", ITERATIONS)
	TIME_OPERATION(rlwe_crossround2(c, b, &rand_ctx), "crossround2", ITERATIONS / 10)
	TIME_OPERATION(rlwe_round2(k, b), "round2", ITERATIONS / 10)
	TIME_OPERATION(rlwe_rec(k, b, c), "rec", ITERATIONS)
#endif

    TIME_OPERATION(ThreePAK_server_first(a,ThreePAK_sf,ThreePAK_sg,ThreePAK_bA,
                ThreePAK_bB,ThreePAK_mA,ThreePAK_mB,gamma_1,eta_1,&ctx),"TPAK_server_first",ITERATIONS/50);
    TIME_OPERATION(ThreePAK_clientB_second(a,ThreePAK_sB,ThreePAK_pB,ThreePAK_mB,
                &ctx,pw_input_B,ThreePAK_wB,ThreePAK_thetaB,ThreePAK_kBS),"TPAK_clientB_second",ITERATIONS/50);
    TIME_OPERATION(ThreePAK_clientB_second(a,ThreePAK_sA,ThreePAK_pA,ThreePAK_mA,
                &ctx,pw_input_A,ThreePAK_wA,ThreePAK_thetaA,ThreePAK_kAS),"TPAK_clientA_first",ITERATIONS/50);
    TIME_OPERATION(ThreePAK_server_second(ThreePAK_pA,ThreePAK_pB,ThreePAK_kAS,
                ThreePAK_kBS,ThreePAK_wA,ThreePAK_wB,
                ThreePAK_sf,ThreePAK_sg,ThreePAK_bA,ThreePAK_bB,
                ThreePAK_cA,ThreePAK_cB,ThreePAK_kSA,ThreePAK_kSB,&ctx),"TPAK_server_second",ITERATIONS/50);
    TIME_OPERATION(ThreePAK_clientB_third(ThreePAK_pA,ThreePAK_pB,ThreePAK_mA,
                ThreePAK_mB,ThreePAK_cB,ThreePAK_kSB,ThreePAK_sB,
                ThreePAK_thetaB,ThreePAK_w,ThreePAK_hashk,ThreePAK_hashkkk,ThreePAK_theta,&ctx),"TPAK_clienB_thired",ITERATIONS/50);
    TIME_OPERATION(ThreePAK_clientA_second(ThreePAK_pA,ThreePAK_pB,ThreePAK_mA,
                ThreePAK_mB,ThreePAK_cA,ThreePAK_kSA,ThreePAK_sA,
                ThreePAK_thetaA,ThreePAK_hashk,ThreePAK_w,ThreePAK_hashkk,ThreePAK_skA,&ctx),"TPAK_clientA_second",ITERATIONS/50);
    TIME_OPERATION(ThreePAK_clientB_finish(ThreePAK_mA,ThreePAK_mB,ThreePAK_pA,
                ThreePAK_pB,ThreePAK_theta,ThreePAK_skB,ThreePAK_hashkk,ThreePAK_hashkkk),"TPAK_clientB_finish",ITERATIONS/50);


    FFT_CTX_clear(&ctx);
	FFT_CTX_free(&ctx);

	RAND_CHOICE_cleanup(&rand_ctx);

	return 0;

}
