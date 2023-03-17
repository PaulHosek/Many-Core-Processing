#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include "timer.h"
#include <unistd.h>
#include <getopt.h>
using namespace std;


// same as sequential, single thread, only overhead
__global__ void histogramKernel(unsigned char* image, long img_size, unsigned int* histogram, int hist_size) {
    unsigned int tid = threadIdx.x + blockDim.x * blockIdx.x;
    if (tid == 1){
        for (int i=0; i<img_size; i++) {
            histogram[image[i]]++;
        }
    }

}