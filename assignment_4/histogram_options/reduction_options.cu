//
// Created by Paul Hosek on 18.03.23.
//

#include "reduction_options.cuh"

// v1. simple reduction
if (threadIdx.x < 256) {
    atomicAdd(&histogram[threadIdx.x], local_hist[threadIdx.x]);
}


// v2. warp reduction
// each first warp in a thread block aggregates the values of the local hist into the global hist
// 256/32 = 8 -> 8 bins for each thread to add to the global array
if (threadIdx.x < warpSize) {
    for (int i = 0; i < 256; i += warpSize) {
        atomicAdd(&histogram[i + threadIdx.x], local_hist[i + threadIdx.x]);
    }
}

// v3. tree reduction
// reduce local hists into
// Perform tree-reduction on local histograms
for (int s = blockDim.x / 2; s > 0; s >>= 1) {
    if (threadIdx.x < s) {
        int i = threadIdx.x * 2;
        local_hist[i] += local_hist[i + s];
    }
    __syncthreads();
}
if (threadIDX.x < 256){ // copy first local_hist, no need for atomic here
    histogram[g_tid] = local_hist[g_tid]);
}

//if (threadIdx.x == 0) {
//    for (int i = 0; i < 256; i++) {
//        atomicAdd(&histogram[i], local_hist[i]);
//    }
