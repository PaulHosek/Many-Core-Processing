#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cuda.h>
#include <float.h>

extern "C"
{
    #include "input.h"
    #include "output.h"
}
#include "cuda_compute.h"

#define BLOCK_SIZE_X 32 
#define BLOCK_SIZE_Y 32 
#define BLOCK_SIZE 1024 
#define WARP_SIZE 32

#define C_CDIR (0.25 * sqrt(2.0) / (sqrt(2.0) + 1.0))
#define C_CDIAG (0.25 / (sqrt(2.0) + 1.0))

__global__ void update_temperatures(double * temperature_old, double * temperature_new, double * conductivity, const int M, const int N)
{
    double old_temperature, new_temperature, c_c, rest_c_c;
    int inside_grid = 0;
    const int tidx = threadIdx.x;
    const int tidy = threadIdx.y;

    __shared__ double temperatures[BLOCK_SIZE_Y][BLOCK_SIZE_X][2];

    const int row = blockIdx.y * (blockDim.y - 2) + threadIdx.y;
    int col = blockIdx.x * (blockDim.x -2) + threadIdx.x - 1;
    inside_grid = (col >= 0) && (col < M) && (row < N + 1) && row;
    col += (col < 0) * M - (col == M) * M;
    const int index = row * M + col;

    if ((row < N + 2) && (col < M)) temperatures[tidy][tidx] = temperature_old[index];
    __syncthreads();

    if ((tidx) && (tidx < BLOCK_SIZE_X-1) && (tidy) && (tidy < BLOCK_SIZE_Y-1) && inside_grid)
    {
        //if (((tidx == BLOCK_SIZE_X-2) && (tidy == BLOCK_SIZE_Y-2)) || ((row == N) && (col == M-1))) printf("row=%d, col=%d, index=%d\n",row,col,index);
        old_temperature = temperature_old[index];
        c_c = conductivity[index - M];
        rest_c_c = 1 - c_c;

        new_temperature = old_temperature * c_c;
        new_temperature += (temperatures[tidy][tidx-1] + temperatures[tidy][tidx+1] +
                temperatures[tidy-1][tidx] + temperatures[tidy+1][tidx]) * rest_c_c * C_CDIR;
        new_temperature += (temperatures[tidy-1][tidx-1] + temperatures[tidy+1][tidx-1] +
                temperatures[tidy-1][tidx+1] + temperatures[tidy+1][tidx+1]) * rest_c_c * C_CDIAG;
        temperature_new[index] = new_temperature;
    }
}

__global__ void get_maxdiff_per_thread(double * temperature_old, double * temperature_new, double * maxdiff, const int M, const int N)
{
    const int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index < M*N) 
    {
        maxdiff[index] = abs(temperature_old[index+M] - temperature_new[index+M]);
    }
}

__global__ void get_maxdiff_per_block(double * maxdiff, int offset, int grid_size)
{
    int index = (blockIdx.x * blockDim.x + threadIdx.x) * offset;
    const int in_grid = index < grid_size;

    double local_maxdiff, diff;

    __shared__ double s_max_diff[BLOCK_SIZE];

    s_max_diff[threadIdx.x] = maxdiff[in_grid * index];

    __syncthreads();

    for (unsigned int s = BLOCK_SIZE/2; s>=WARP_SIZE; s/=2) 
    {
        if (threadIdx.x < s)
        { 
            local_maxdiff = s_max_diff[threadIdx.x];

            if ((diff = s_max_diff[threadIdx.x + s]) > local_maxdiff) 
            {
                s_max_diff[threadIdx.x] = diff;
            }
        }
        __syncthreads();
    }

    if (threadIdx.x < WARP_SIZE)
    {
        local_maxdiff = s_max_diff[threadIdx.x];
        for (unsigned int s = WARP_SIZE/2; s > 0; s /= 2)
        {
            if ((diff = __shfl_down_sync(0xffffffff, local_maxdiff, s)) > local_maxdiff) 
            {
                local_maxdiff = diff;
            }
        }
    }

    if (!threadIdx.x)
    {
        maxdiff[index] = local_maxdiff;
    } 
}

__global__ void initialize_statistics(double * temperatures, double * min, double * max, double * sum, const int M, const int N)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    double temperature;

    if (index < M*N) 
    {
        temperature = temperatures[index + M];
        min[index] = temperature;
        max[index] = temperature;
        sum[index] = temperature;
    }
}

__global__ void get_statistics_per_block(double * min, double * max, double * sum, int offset, int grid_size)
{
    int index = (blockIdx.x * blockDim.x + threadIdx.x) * offset;
    double local_sum, local_min, local_max, new_min, new_max;
    const int in_grid = index < grid_size;

    __shared__ double s_sum[BLOCK_SIZE];
    __shared__ double s_min[BLOCK_SIZE];
    __shared__ double s_max[BLOCK_SIZE];

    s_min[threadIdx.x] = min[in_grid * index];
    s_max[threadIdx.x] = max[in_grid * index];
    s_sum[threadIdx.x] = in_grid * sum[in_grid * index];

    __syncthreads();

    for (unsigned int s = BLOCK_SIZE/2; s>=WARP_SIZE; s/=2) 
    {
        if (threadIdx.x < s)
        {
            local_min = s_min[threadIdx.x];
            local_max = s_max[threadIdx.x];

            if ((new_min = s_min[threadIdx.x + s]) < local_min) 
            {
                s_min[threadIdx.x] = new_min;
            }
            if ((new_max = s_max[threadIdx.x + s]) > local_max) 
            {
                s_max[threadIdx.x] = new_max;
            }
            s_sum[threadIdx.x] += s_sum[threadIdx.x + s];
        }
        __syncthreads();
    }

    if (threadIdx.x < WARP_SIZE)
    {
        local_sum = s_sum[threadIdx.x];
        local_min = s_min[threadIdx.x];
        local_max = s_max[threadIdx.x];
        for (unsigned int s = WARP_SIZE/2; s > 0; s /= 2)
        {
            if ((new_min = __shfl_down_sync(0xffffffff, local_min, s)) < local_min) 
            {
                local_min = new_min;
            }
            if ((new_max = __shfl_down_sync(0xffffffff, local_max, s)) > local_max) 
            {
                local_max = new_max;
            }
            local_sum += __shfl_down_sync(0xffffffff, local_sum, s);
        }
    }

    if (!threadIdx.x)
    {
        sum[index] = local_sum;
        min[index] = local_min;
        max[index] = local_max;
    } 
}

extern "C"
__host__ void print_grid(double const * grid, int m, int n)
{
    for(int i=0; i < n; i++)
    {
        for(int j=0; j < m; j++)
        {
            printf("%.2f ", grid[i*m+j]);
        }
        printf("\n\n");
    }
}

extern "C"
__host__ void initialize(const struct parameters* p, double* temperature_old, double* temperature_new, double* conductivity)
{
    int MN = p->N * p-> M; 
    int M = p->M;
    int index;
    // Halo rows
    for (index = 0; index < M; index++)
    {
        temperature_old[index] = p->tinit[index];
        temperature_new[index] = p->tinit[index];
        temperature_old[MN + M + index] = p->tinit[MN - M + index];
        temperature_new[MN + M + index] = p->tinit[MN - M + index];
    }
    // Fill the temperature values into the grid cells
    for (index = 0; index < MN; index++)
    {
        temperature_old[M + index] = p->tinit[index];
        conductivity[index] = p->conductivity[index];
    }
}

extern "C"
__host__ void cuda_do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    const int N = p->N;
    const int M = p->M;
    double * temperature_old = (double *) malloc((N + 2) * M * sizeof(double));
    double * temperature_new = (double *) malloc((N + 2) * M * sizeof(double));
    double * conductivity = (double *) malloc(N * M * sizeof(double));
    double * d_temperature_old; 
    double * d_temperature_new; 
    double * d_conductivity;
    double * d_maxdiff;
    double * d_sum; 
    double * d_min; 
    double * d_max;
    int it = 1;
    const int grid_size = (p->N * p->M);
    int offset;
    const int maxiter = p->maxiter;
    const int period = p->period;
    const double threshold = p->threshold;
    int converged, compute_statistics, iters_to_next_period = period - 1;
    double maxdiff, tmin, tmax, tsum;
    cudaError_t err;

    initialize(p, temperature_old, temperature_new, conductivity);

    //allocate GPU memory
    err = cudaMalloc((void **)&d_temperature_old, (N + 2) * M * sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_temperature_old: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_temperature_new, (N + 2) * M *sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_temperature_new: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_conductivity, M*N*sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_conductivity: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_maxdiff, M*N*sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_maxdiff: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_min,  M*N*sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_min: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_max,  M*N*sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_max: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_sum,  M*N*sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_sum: %s\n", cudaGetErrorString(err));
    
    // copy to GPU memory
    err = cudaMemcpy(d_temperature_new, temperature_new, (N + 2) * M * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy d_temperature_new: %s\n", cudaGetErrorString( err ));
    err = cudaMemcpy(d_temperature_old, temperature_old, (N + 2) * M * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy d_temperature_old: %s\n", cudaGetErrorString( err ));
    err = cudaMemcpy(d_conductivity, conductivity, N * M * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy d_conductivity: %s\n", cudaGetErrorString( err ));

    //setup the grid and thread blocks
    int nblocks_x = int(ceilf(M/(double)(BLOCK_SIZE_X-2)));//n divided by thread block size rounded up
    int nblocks_y = int(ceilf(N/(double)(BLOCK_SIZE_Y-2)));
    int nblocks_x_statistics = int(ceilf(grid_size / (double)BLOCK_SIZE));
    dim3 threads_temperatures(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1);
    dim3 threads_statistics(BLOCK_SIZE,1,1);
    dim3 grid_temperatures(nblocks_x, nblocks_y, 1);
    dim3 grid_statistics(nblocks_x_statistics,1,1);

    struct timespec before, after;
    cudaDeviceSynchronize();
    clock_gettime(CLOCK_MONOTONIC, &before);

    do {
        // Check convergence every timestep
        converged = 0;
        compute_statistics = (!iters_to_next_period || it == maxiter);
        update_temperatures<<<grid_temperatures, threads_temperatures>>>(d_temperature_old, d_temperature_new, d_conductivity, M, N);
        cudaDeviceSynchronize();
        //check to see if all went well
        err = cudaGetLastError();
        if (err != cudaSuccess) fprintf(stderr, "Error during kernel execution: %s\n", cudaGetErrorString(err));
        grid_statistics.x = nblocks_x_statistics;
        get_maxdiff_per_thread<<<grid_statistics, threads_statistics>>>(d_temperature_old, d_temperature_new, d_maxdiff, M, N);
        cudaDeviceSynchronize();
        err = cudaGetLastError();
        if (err != cudaSuccess) fprintf(stderr, "Error during kernel execution: %s\n", cudaGetErrorString(err));
        offset=1;
        while (offset * BLOCK_SIZE < grid_size)
        {
            get_maxdiff_per_block<<<grid_statistics, threads_statistics>>>(d_maxdiff, offset, grid_size);
            cudaDeviceSynchronize();
            err = cudaGetLastError();
            if (err != cudaSuccess) fprintf(stderr, "Error during kernel execution: %s\n", cudaGetErrorString(err));
            offset *= BLOCK_SIZE;
            grid_statistics.x = int(ceilf(grid_statistics.x / (double)offset));
        };
        grid_statistics.x = 1; 
        get_maxdiff_per_block<<<grid_statistics, threads_statistics>>>(d_maxdiff, offset, grid_size);
        cudaDeviceSynchronize();
        err = cudaGetLastError();
        if (err != cudaSuccess) fprintf(stderr, "Error during kernel execution: %s\n", cudaGetErrorString(err));

        err = cudaMemcpy(&maxdiff, d_maxdiff, sizeof(double), cudaMemcpyDeviceToHost);
        if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy &maxdiff: %s\n", cudaGetErrorString( err ));
        if (maxdiff <= threshold)
        {
            converged = 1;
        }

        if (converged || compute_statistics)
        {
            grid_statistics.x = nblocks_x_statistics;
            initialize_statistics<<<grid_statistics, threads_statistics>>>(d_temperature_new, d_min, d_max, d_sum, M, N);
            cudaDeviceSynchronize();
            err = cudaGetLastError();
            if (err != cudaSuccess) fprintf(stderr, "Error during kernel execution: %s\n", cudaGetErrorString(err));
            offset=1;
            while (offset * BLOCK_SIZE < grid_size)
            {
                grid_statistics.x = int(ceilf(grid_statistics.x / (double)offset));
                get_statistics_per_block<<<grid_statistics, threads_statistics>>>(d_min, d_max, d_sum, offset, grid_size);
                cudaDeviceSynchronize();
                err = cudaGetLastError();
                if (err != cudaSuccess) fprintf(stderr, "Error during kernel execution: %s\n", cudaGetErrorString(err));
                offset *= BLOCK_SIZE;
            };
            grid_statistics.x = 1; 
            get_statistics_per_block<<<grid_statistics, threads_statistics>>>(d_min, d_max, d_sum, offset, grid_size);
            cudaDeviceSynchronize();
            err = cudaGetLastError();
            if (err != cudaSuccess) fprintf(stderr, "Error during kernel execution: %s\n", cudaGetErrorString(err));

            err = cudaMemcpy(&tmin, d_min, sizeof(double), cudaMemcpyDeviceToHost);
            if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy &tmin: %s\n", cudaGetErrorString( err ));
            err = cudaMemcpy(&tmax, d_max, sizeof(double), cudaMemcpyDeviceToHost);
            if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy &tmax: %s\n", cudaGetErrorString( err ));
            err = cudaMemcpy(&tsum, d_sum, sizeof(double), cudaMemcpyDeviceToHost);
            if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy &tsum: %s\n", cudaGetErrorString( err ));


            r->niter = it;
            r->tmin = tmin;
            r->tmax = tmax;
            r->tavg = tsum/grid_size;
            r->maxdiff = maxdiff;
            clock_gettime(CLOCK_MONOTONIC, &after);
            r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;
            
            if (it < maxiter && !converged && p->printreports){
                // Only call print if it's not the last iteration and the print-parameter is set 
                report_results(p,r);
            }
        }

        // Flip old and new values
        {double * tmp = d_temperature_old; d_temperature_old = d_temperature_new; d_temperature_new = tmp;} 
        it++;
        if (iters_to_next_period)
        {
            iters_to_next_period--;
        }
        else
        {
            iters_to_next_period += period-1;
        }
    } while ((it <= maxiter) && (!converged));
    
    clock_gettime(CLOCK_MONOTONIC, &after);
    r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    double flops_per_it = 12.0;
    double Flops = (double)p->N * (double)p->M * 
                    (double)(r->niter * flops_per_it +
                    (double)r->niter / p->period) / r->time;
                    
    //clean up GPU memory allocations
    cudaFree(d_temperature_old);
    cudaFree(d_temperature_new);
    cudaFree(d_conductivity);
}

// static void checkCudaCall(cudaError_t result) {
//     if (result != cudaSuccess) {
//         printf("cuda error \n");
//         exit(1);
//     }
// }


// __global__ void vectorAddKernel(float* deviceA, float* deviceB, float* deviceResult) {
//     unsigned i = blockIdx.x * blockDim.x + threadIdx.x;
// // insert operation here
//     deviceResult[i] = deviceA[i]+deviceB[i];
// }

// extern "C" 
// void cuda_do_compute() {
//     int threadBlockSize = 512;
//     int n=1024; 
//     float a[1024], b[1024], result[1024];
//     // allocate the vectors on the GPU
//     float* deviceA = NULL;
//     checkCudaCall(cudaMalloc((void **) &deviceA, n * sizeof(float)));
//     if (deviceA == NULL) {
//         printf("Error in cudaMalloc! \n");
// 	return;
//     }
//     float* deviceB = NULL;
//     checkCudaCall(cudaMalloc((void **) &deviceB, n * sizeof(float)));
//     if (deviceB == NULL) {
//         checkCudaCall(cudaFree(deviceA));
//         printf("Error in cudaMalloc! \n");
//         return;
//     }
//     float* deviceResult = NULL;
//     checkCudaCall(cudaMalloc((void **) &deviceResult, n * sizeof(float)));
//     if (deviceResult == NULL) {
//         checkCudaCall(cudaFree(deviceA));
//         checkCudaCall(cudaFree(deviceB));
//         printf("Error in cudaMalloc! \n");
//         return;
//     }


//     // copy the original vectors to the GPU
//     checkCudaCall(cudaMemcpy(deviceA, a, n*sizeof(float), cudaMemcpyHostToDevice));
//     checkCudaCall(cudaMemcpy(deviceB, b, n*sizeof(float), cudaMemcpyHostToDevice));

//     // execute kernel
//     vectorAddKernel<<<n/threadBlockSize, threadBlockSize>>>(deviceA, deviceB, deviceResult);
//     cudaDeviceSynchronize();

//     // check whether the kernel invocation was successful
//     checkCudaCall(cudaGetLastError());

//     // copy result back
//     checkCudaCall(cudaMemcpy(result, deviceResult, n * sizeof(float), cudaMemcpyDeviceToHost));
//     checkCudaCall(cudaMemcpy(b, deviceB, n * sizeof(float), cudaMemcpyDeviceToHost));

//     checkCudaCall(cudaFree(deviceA));
//     checkCudaCall(cudaFree(deviceB));
//     checkCudaCall(cudaFree(deviceResult));

// }
