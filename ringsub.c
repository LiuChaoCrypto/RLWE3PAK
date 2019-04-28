#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rlwe_kex.h"

#include "fft.h"

void ringsub(uint32_t outring[1024],const uint32_t m_A[1024],const uint32_t gamma[1024])
{

    uint32_t minus[1024];
    for(int i=0;i<1024;i++)
    {

        minus[i]=-gamma[i]-1;
    }
    FFT_add(outring,m_A,minus);
}
