#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "cuda_compute.h"
#include "compute.h"

void print_grid(double const * grid, int m, int n)
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

void initialize(const struct parameters* p, double* temperature_old, double* temperature_new, 
double* conductivity)
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

void do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    const int nthreads = p->nthreads;
    double * temperature_old = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * temperature_new = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * conductivity = (double * ) malloc((p->N) * p->M * sizeof(double));
    struct grid_data * grid_parameters = (struct grid_data *) malloc(nthreads * sizeof(struct grid_data));
    double * d_temperature_old; double * d_temperature_new; double * d_conductivity;
    pthread_t thread_ids[nthreads];
    int converged_threads[2 * nthreads];
    int it = 1;
    const int N = p->N;
    const int grid_start = p->M;
    const int grid_end = p->M  * (p->N + 1);
    const int grid_size = (p->N * p->M);
    const int rows_per_thread = N / nthreads;
    int remaining_rows = N % nthreads;
    int current_row = 1;
    const int maxiter = p->maxiter;
    const int period = p->period;
    const int M = p->M;
    int result_index = 0;
    int converged, compute_statistics, iters_to_next_period = period - 1;
    double maxdiff, tmin, tmax, tsum, maxdiff_thread, tmin_thread, tmax_thread;

    initialize(p, temperature_old, temperature_new, conductivity);

    //allocate GPU memory
    err = cudaMalloc((void **)&d_temperature_old, (N + 2) * M * sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_a: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_temperature_new, (N + 2) * M *sizeof(double));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_a: %s\n", cudaGetErrorString(err));
    err = cudaMalloc((void **)&d_conductivity, M*N*sizeof(float));
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMalloc d_c: %s\n", cudaGetErrorString(err));
    
    // copy to GPU memory
    err = cudaMemcpy(d_temperature_new, temperature_new, (N + 2) * M * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy: %s\n", cudaGetErrorString( err ));
    err = cudaMemcpy(d_temperature_old, temperature_old, (N + 2) * M * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy: %s\n", cudaGetErrorString( err ));
    err = cudaMemcpy(d_conductivity, conductivity, N * M * sizeof(double), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) fprintf(stderr, "Error in cudaMemcpy: %s\n", cudaGetErrorString( err ));
    
    //setup the grid and thread blocks
    const int block_size_x = 32;//thread block size
    const int block_size_y = 32;
    int nblocks_x = int(ceilf(M/(float)block_size_x));//n divided by thread block size rounded up
    int nblocks_y = int(ceilf(N/(float)block_size_y));
    dim3 threads(block_size_x, block_size_y, 1);
    dim3 grid(nblocks_x, nblocks_y, 1);

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    /* ... */

    clock_gettime(CLOCK_MONOTONIC, &after);
    r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    /* ... */
}

__global__ void update_temperatures(double * temperature_old, double * temperature_new, )
{
    struct grid_data * parameters = (struct grid_data *) grid_parameters;
    const double * conductivity = parameters->conductivity;
    const int M = parameters->M;
    const double threshold = parameters->threshold;
    static const double c_cdir = 0.25 * SQRT2 / (SQRT2 + 1.0);
    static const double c_cdiag = 0.25 / (SQRT2 + 1.0);
    const int first_row = parameters->first_row;
    const int first_row_offset = first_row * M;
    const int last_row = parameters->last_row;
    const int last_row_offset = last_row * M;
    double * temperature_new = parameters->temperature_new;
    double * temperature_old = parameters->temperature_old;
    int * converged = parameters->converged;
    const int thread_id = parameters->thread_id;
    const int nthreads = parameters->nthreads;
    int iter = 1;
    const int maxiter = parameters->maxiter;
    const int period = parameters->period;
    int converged_global, converged_local = 1;
    int iters_to_next_period = period-1;
    int index,row,col;
    int row_offset;
    int index_left_col, index_right_col;
    int result_index = 0;
    double maxdiff_thread, tmin_thread, tmax_thread, tsum_thread, new_temperature, old_temperature, diff, c_c, rest_c_c;
    double old_temperature_left, old_temperature_right;
    do {
        maxdiff_thread = 0.0;
        tmin_thread = DBL_MAX;
        tmax_thread = DBL_MIN;
        tsum_thread = 0.0;
        converged_local = 1;
        for (row_offset=first_row_offset; row_offset < last_row_offset; row_offset+=M)
        {
            // Inner cells
            index_right_col = row_offset + M-1;
            for (index=row_offset+1; index < index_right_col; index++)
            {
                old_temperature = temperature_old[index];
                c_c = conductivity[index - M];
                rest_c_c = 1 - c_c;
                // Scale old temperature at the cell

                new_temperature = old_temperature * c_c;

                // Adjacent neighbors
                new_temperature += (temperature_old[index-1] + temperature_old[index - M] + temperature_old[index+1] + 
                                    temperature_old[index + M]) * rest_c_c * c_cdir;

                // Diagonal neighbors
                new_temperature += (temperature_old[index-1 - M] + temperature_old[index+1 - M] + 
                                    temperature_old[index-1 + M] + temperature_old[index+1 + M])
                                    * rest_c_c * c_cdiag;

                temperature_new[index] = new_temperature;

                // Only converge if all values below threshold
                diff = fabs(old_temperature - new_temperature);
                if (diff > maxdiff_thread){
                    maxdiff_thread = diff;
                }
                converged_local = converged_local & (diff < threshold);
            }

            // Outer cells
            index_left_col = row_offset;
            old_temperature_left = temperature_old[index_left_col];
            old_temperature_right = temperature_old[index_right_col];

            // Left
            // Scale old temperature at the cell
            c_c = conductivity[index_left_col - M];
            rest_c_c = 1 - c_c;
            new_temperature = old_temperature_left * c_c;

            // Adjacent neighbors
            new_temperature += (old_temperature_right + temperature_old[index_left_col - M] + temperature_old[index_left_col+1] + 
                                temperature_old[index_left_col + M]) * rest_c_c * c_cdir;

            // Diagonal neighbors
            new_temperature += (temperature_old[index_right_col - M] + temperature_old[index_left_col+1 - M] + 
                                temperature_old[index_right_col + M] + temperature_old[index_left_col+1 + M])
                                * rest_c_c * c_cdiag;

            temperature_new[index_left_col] = new_temperature;

            // Only converge if all values below threshold
            double diff = fabs(old_temperature_left - new_temperature);
            if (diff > maxdiff_thread){
                maxdiff_thread = diff;
            }
            converged_local = converged_local & (diff < threshold);

            // Right
            // Scale old temperature at the cell
            c_c = conductivity[index_right_col - M];
            rest_c_c = 1 - c_c;
            new_temperature = old_temperature_right * c_c;

            // Adjacent neighbors
            new_temperature += (old_temperature_left + temperature_old[index_right_col - M] + temperature_old[index_right_col-1] + 
                                temperature_old[index_right_col + M]) * rest_c_c * c_cdir;

            // Diagonal neighbors
            new_temperature += (temperature_old[index_left_col - M] + temperature_old[index_right_col-1 - M] + 
                                temperature_old[index_left_col + M] + temperature_old[index_right_col-1 + M])
                                * rest_c_c * c_cdiag;

            temperature_new[index_right_col] = new_temperature;

            // Only converge if all values below threshold
            diff = fabs(old_temperature_right - new_temperature);
            if (diff > maxdiff_thread){
                maxdiff_thread = diff;
            }
            converged_local = converged_local & (diff < threshold);
        }

        converged[2 * thread_id + result_index] = converged_local;

        // Check global convergence
        converged_global = 1;
        pthread_barrier_wait (&barrier);
        for (index = 0; index < nthreads; index++)
        {
            converged_global &= converged[2 * index + result_index];
        }

        // Go over temperatures and check minimum, maximum temperature
        if (!iters_to_next_period || iter == maxiter || converged_global){
            for (index=first_row_offset; index < last_row_offset; index++)
            {
                new_temperature = temperature_new[index];
                tsum_thread += new_temperature;
                if (new_temperature > tmax_thread){
                    tmax_thread = new_temperature;
                }
                if (new_temperature < tmin_thread){
                    tmin_thread = new_temperature;
                    }
            }

            parameters->tmin[result_index] = tmin_thread;
            parameters->tmax[result_index] = tmax_thread;
            parameters->tsum[result_index] = tsum_thread;
            parameters->maxdiff[result_index] = maxdiff_thread;
        }

        iter++;
        if (iters_to_next_period)
        {
            iters_to_next_period--;
        }
        else
        {
            iters_to_next_period += period-1;
        }
        result_index ^= 1;
        {double * tmp = temperature_old; temperature_old = temperature_new; temperature_new = tmp;}
    } while ((iter <= maxiter) && (!converged_global));
    pthread_barrier_wait (&barrier);
}