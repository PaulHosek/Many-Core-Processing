#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include "timer.h"
#include <unistd.h>
#include <getopt.h>

using namespace std;

/* Utility function, use to do error checking.

   Use this function like this:

   checkCudaCall(cudaMalloc((void **) &deviceRGB, imgS * sizeof(color_t)));

   And to check the result of a kernel invocation:

   checkCudaCall(cudaGetLastError());
*/
void die(const char *msg){
    if (errno != 0) 
        perror(msg);
    else
        fprintf(stderr, "error: %s\n", msg);
    exit(1);
}   

void generate_image(int num_rows, int num_cols, unsigned char * image){
    for (int i = 0; i < num_cols * num_rows; ++i)
    {
        image[i] = (unsigned char) (rand() % 256); //255 + 1 for num bins
    }
}

void read_image(const char * image_path, int num_rows, int num_cols, unsigned char * image){
	char format[3];
    FILE *f;
    unsigned imgw, imgh, maxv, v;
    size_t i;

	printf("Reading PGM data from %s...\n", image_path);

	if (!(f = fopen(image_path, "r"))) die("fopen");

	fscanf(f, "%2s", format);
    if (format[0] != 'P' || format[1] != '2') die("only ASCII PGM input is supported");
    
    if (fscanf(f, "%u", &imgw) != 1 ||
        fscanf(f, "%u", &imgh) != 1 ||
        fscanf(f, "%u", &maxv) != 1) die("invalid input");

    if (imgw != num_cols || imgh != num_rows) {
        fprintf(stderr, "input data size (%ux%u) does not match cylinder size (%zux%zu)\n",
                imgw, imgh, num_cols, num_rows);
        die("invalid input");
    }

    for (i = 0; i < num_cols * num_rows; ++i)
    {
        if (fscanf(f, "%u", &v) != 1) die("invalid data");
        image[i] = (unsigned char) (((int)v * 255) / maxv); //255 for num bins
    }
    fclose(f);
}

static void checkCudaCall(cudaError_t result) {
    if (result != cudaSuccess) {
        cerr << "cuda error: " << cudaGetErrorString(result) << endl;
        exit(1);
    }
}



__global__ void histogramKernel(unsigned char* image, long img_size, unsigned int* histogram, int hist_size) {
    // shared memory to store local histogram of warp
    __shared__ unsigned int smem_hist[256];
    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int lane = threadIdx.x & (32-1); // tid in warp
//    int warp_id = threadIdx.x >> 5; // warp index, = /32+ round down
    unsigned int local_hist[256] = {0};


    // process 32 elements
    // each thread processes every max(tid)s pixel. if there are 12 threads, every 12th pixel will be processed
    // Note, that if values in image have pattern that matches this division, collisions are maximal
    for (int i = tid; i < img_size; i += blockDim.x * gridDim.x) {
        unsigned char value = image[i];
        atomicAdd(&local_hist[value], 1);
    }
    // aggregate the local histograms using warp-level atomic operations in shared memory
    for (int i = lane; i < 256; i += warpSize) {
        smem_hist[i] = 0;
    }
    // iterate over bins (0-255)
    for (int i = lane; i < 256; i += warpSize) {
        unsigned int sum = 0;
        // iterate over threads in warp
        for (int j = 0; j < warpSize; j++) {
            int index = i+j*256;
//            printf("(%d)+(%d)*256 =(%d)\n", i,j,i+j*256);
            if (index < 256){
                sum += local_hist[index];
            }


//            sum += local_hist[i + j * 256]; // FIXME: here is the illegal access
        }
        atomicAdd(&smem_hist[i], sum);
    }
    __syncthreads();
    // copy the aggregated histogram to global memory
    if (lane == 0) {
        for (int i = 0; i < 256; i++) {
            atomicAdd(&histogram[i], smem_hist[i]);
        }
    }
}


void histogramCuda(unsigned char* image, long img_size, unsigned int* histogram, int hist_size) {
    int threadBlockSize = 512;

    // allocate the vectors on the GPU
    unsigned char* deviceImage = NULL;
    checkCudaCall(cudaMalloc((void **) &deviceImage, img_size * sizeof(unsigned char)));
    if (deviceImage == NULL) {
        cout << "could not allocate memory!" << endl;
        return;
    }
    unsigned int* deviceHisto = NULL;
    checkCudaCall(cudaMalloc((void **) &deviceHisto, hist_size * sizeof(unsigned int)));
    if (deviceHisto == NULL) {
        checkCudaCall(cudaFree(deviceImage));
        cout << "could not allocate memory!" << endl;
        return;
    }

    timer kernelTime1 = timer("kernelTime1");
    timer memoryTime = timer("memoryTime");

    // copy the original vectors to the GPU
    memoryTime.start();
    checkCudaCall(cudaMemcpy(deviceImage, image, img_size*sizeof(unsigned char), cudaMemcpyHostToDevice));
    memoryTime.stop();

    // Paul:
    // launch differently, change threadblock-size such we dont wast threads if the image size is not a multiple of 512
    int numBlocks = (img_size + threadBlockSize - 1) / threadBlockSize;

    // execute kernel
    kernelTime1.start();
    histogramKernel<<<numBlocks, threadBlockSize>>>(deviceImage, img_size, deviceHisto, hist_size);

//    histogramKernel<<<img_size/threadBlockSize, threadBlockSize>>>(deviceImage, img_size, deviceHisto, hist_size);
    cudaDeviceSynchronize();
    kernelTime1.stop();

    // check whether the kernel invocation was successful
    checkCudaCall(cudaGetLastError());

    // copy result back
    memoryTime.start();
    checkCudaCall(cudaMemcpy(histogram, deviceHisto, hist_size * sizeof(unsigned int), cudaMemcpyDeviceToHost));
    memoryTime.stop();

    checkCudaCall(cudaFree(deviceImage));
    checkCudaCall(cudaFree(deviceHisto));

    cout << "histogram (kernel): \t\t" << kernelTime1  << endl;
    cout << "histogram (memory): \t\t" << memoryTime << endl;
}

void histogramSeq(unsigned char* image, long img_size, unsigned int* histogram, int hist_size) {
  int i; 

  timer sequentialTime = timer("Sequential");
  
  for (i=0; i<hist_size; i++){
      histogram[i]=0;
  }

  sequentialTime.start();
  for (i=0; i<img_size; i++) {
	histogram[image[i]]++;
  }
  sequentialTime.stop();
  
  cout << "histogram (sequential): \t\t" << sequentialTime << endl;

}

int main(int argc, char* argv[]) {
    int c;
    int seed = 42;
    const char *image_path = 0;
    image_path ="../../images/pat1_100x150.pgm";
    int gen_image = 0;
    int debug = 0;

    int num_rows = 150;
    int num_cols = 100;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "s:i:rp:n:m:g")) != -1) {
        switch(c) {
            case 's':
                seed = atoi(optarg);
                break;
            case 'i':
            	image_path = optarg;
            	break;
            case 'r':
            	gen_image = 1;
            	break;
            case 'n':
            	num_rows = strtol(optarg, 0, 10);
            	break;
            case 'm':
				num_cols = strtol(optarg, 0, 10);
				break;
			case 'g':
				debug = 1;
				break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }

    int hist_size = 256;
    long img_size = num_rows*num_cols;

    unsigned char *image = (unsigned char *)malloc(img_size * sizeof(unsigned char)); 
    unsigned int *histogramS = (unsigned int *)malloc(hist_size * sizeof(unsigned int));     
    unsigned int *histogram = (unsigned int *)malloc(hist_size * sizeof(unsigned int));

    /* Seed such that we can always reproduce the same random vector */
    if (gen_image){
    	srand(seed);
    	generate_image(num_rows, num_cols, image);
    }else{
    	read_image(image_path,num_rows, num_cols, image);
    }

    histogramSeq(image, img_size, histogramS, hist_size);
    histogramCuda(image, img_size, histogram, hist_size);
    
    // verify the resuls
    for(int i=0; i<hist_size; i++) {
	  if (histogram[i]!=histogramS[i]) {
            cout << "error in results! Bin " << i << " is "<< histogram[i] << ", but should be " << histogramS[i] << endl; 
            exit(1);
        }
    }
    cout << "results OK!" << endl;
     
    free(image);
    free(histogram);
    free(histogramS);         
    
    return 0;
}
