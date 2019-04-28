#ifndef _RINGSUB_H
#define _RINGSUB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rlwe_kex.h"

#include "fft.h"
void ringsub(uint32_t outring[1024],const uint32_t m_A[1024],const uint32_t gamma[1024]);

#endif
