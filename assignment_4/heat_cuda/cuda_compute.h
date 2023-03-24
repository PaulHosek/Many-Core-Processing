#ifndef CUDA_COMPUTE_H
#define CUDA_COMPUTE_H

#include "output.h"
#include "input.h"

#ifdef __cplusplus
extern "C" 
#endif 

void cuda_do_compute(const struct parameters* p, struct results *r);

#endif
