#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <pthread.h>
#include <immintrin.h>

#include "compute.h"

const double SQRT2 = sqrt(2);
pthread_barrier_t barrier;

struct grid_data {
   size_t  first_row;
   size_t  last_row;
   double * temperature_old;
   double * temperature_new;
   double * conductivity;
   int * converged;
   double tsum[2];
   double maxdiff[2];
   double tmin[2];
   double tmax[2];
   double threshold;
   int M;
   int maxiter;
   int period;
   int thread_id;
   int nthreads;
   int padding_left;
   int padding_right;
};

void initialize(const struct parameters* p, double* temperature_old, double* temperature_new, 
double* conductivity, int padding_left, int padding_right);
void * update_temperatures(void * grid_parameters);

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
double* conductivity, int padding_left, int padding_right)
{
    int MN = p->N * p-> M; 
    int N = p->N;
    int M = p->M;
    int index;
    // Halo rows

    for (index=0; index < padding_left; index++)
    {
        temperature_old[index] = 0.0;
        temperature_new[index] = 0.0;

        temperature_old[(N+1)*(M+padding_left+padding_right)+index] = 0;
        temperature_new[(N+1)*(M+padding_left+padding_right)+index] = 0;
    }

    for (index=M+padding_left; index < padding_left+M+padding_right; index++)
    {
        temperature_old[index] = 0;
        temperature_new[index] = 0;

        temperature_old[(N+1)*(M+padding_left+padding_right)+index] = 0;
        temperature_new[(N+1)*(M+padding_left+padding_right)+index] = 0;
    }

    for (index=0; index<M; index++)
    {
        temperature_old[padding_left+index] = p->tinit[index];
        temperature_new[padding_left+index] = p->tinit[index];

        temperature_old[(N+1)*(M+padding_left+padding_right)+padding_left+index] = p->tinit[MN - M + index];
        temperature_new[(N+1)*(M+padding_left+padding_right)+padding_left+index] = p->tinit[MN - M + index];
    }

    // Fill the temperature values into the grid cells
    for (int row = 1; row < N+1; row++)
    {
        for (index=row*(padding_left+M+padding_right); index < row*(padding_left+M+padding_right)+padding_left; index++)
        {
            temperature_old[index] = 0;
            temperature_new[index] = 0;
            conductivity[index - (padding_left+M+padding_right)] = 1;
        }
        for (int col = 0; col < M; col++)
        {
            temperature_old[row*(padding_left+M+padding_right)+padding_left + col] = p->tinit[(row-1)*M+col];
            conductivity[(row-1)*(padding_left+M+padding_right)+padding_left + col] = p->conductivity[(row-1)*M+col];
        }
        for (index=row*(padding_left+M+padding_right)+padding_left+M; index < (row+1)*(padding_left+M+padding_right); index++)
        {
            temperature_old[index] = 0;
            temperature_new[index] = 0;
            conductivity[index - (padding_left+M+padding_right)] = 1;
        }
    }
}

void do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    const int M = p->M;
    const int padding_left=3;
    const int padding_right = (4 - (M + padding_left) % 4) % 4;
    const int nthreads = p->nthreads;
    double * temperature_old = (double * ) _mm_malloc((p->N + 2) * (M+padding_left+padding_right) * sizeof(double),32);
    double * temperature_new = (double * ) _mm_malloc((p->N + 2) * (M+padding_left+padding_right) * sizeof(double),32);
    double * conductivity = (double * ) _mm_malloc((p->N) * (M+padding_left+padding_right) * sizeof(double),32);
    struct grid_data * grid_parameters = (struct grid_data *) malloc(nthreads * sizeof(struct grid_data));
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
    int result_index = 0;
    int converged, compute_statistics, iters_to_next_period = period - 1;
    double maxdiff, tmin, tmax, tsum; 
    double maxdiff_thread, tmin_thread, tmax_thread;
    pthread_barrier_init(&barrier, NULL, nthreads+1);

    initialize(p, temperature_old, temperature_new, conductivity, padding_left, padding_right);
    
    //print_grid(temperature_old, M+padding_left+padding_right, N+2);
    int index;
    for (index=0; index < nthreads; index++)
    {
        grid_parameters[index].thread_id = index;
        grid_parameters[index].temperature_old = temperature_old;
        grid_parameters[index].temperature_new = temperature_new;
        grid_parameters[index].conductivity = conductivity;
        grid_parameters[index].tsum[result_index] = 0;
        grid_parameters[index].tmin[result_index] = p->io_tmax;
        grid_parameters[index].tmax[result_index] = p->io_tmin;
        grid_parameters[index].maxdiff[result_index] = 0.0;
        grid_parameters[index].first_row = current_row;
        current_row += rows_per_thread;
        if (remaining_rows)
        {
            remaining_rows--;
            current_row++;
        }
        grid_parameters[index].last_row = current_row;
        grid_parameters[index].threshold = p->threshold;
        grid_parameters[index].M = M;
        grid_parameters[index].maxiter = maxiter;
        grid_parameters[index].period = period;
        grid_parameters[index].converged = converged_threads;
        grid_parameters[index].nthreads = nthreads;
        grid_parameters[index].padding_left = padding_left;
        grid_parameters[index].padding_right = padding_right;
    }
    grid_parameters[nthreads-1].last_row = N+1;

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);
    
    for (index=0; index<nthreads; index++) {
        pthread_create(&thread_ids[index],
        NULL,
        update_temperatures,
        (void *)&grid_parameters[index]);
    }
    
    pthread_barrier_wait (&barrier);
    do {
        maxdiff = 0.0;
        tmin = DBL_MAX;
        tsum = 0;
        tmax = DBL_MIN;
        // Check convergence every timestep
        converged = 1;
        compute_statistics = (!iters_to_next_period || it == maxiter);

        pthread_barrier_wait (&barrier);

        for (index=0; index<nthreads; index++) {
            converged &= converged_threads[index * 2 + result_index];
        }

        // Go over temperatures and check minimum, maximum temperature and maximum difference
        if (compute_statistics || converged){
            for (index = 0; index < nthreads; index++){
                tsum = grid_parameters[index].tsum[result_index];
                tmax_thread = grid_parameters[index].tmax[result_index];
                tmin_thread = grid_parameters[index].tmin[result_index];
                maxdiff_thread = grid_parameters[index].maxdiff[result_index];
                if (tmax_thread > tmax){
                    tmax = tmax_thread;
                }
                if (tmin_thread < tmin){
                    tmin = tmin_thread;
                }
                if (maxdiff_thread > maxdiff){
                    maxdiff = maxdiff_thread;
                }
            }

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
        it++;
        if (iters_to_next_period)
        {
            iters_to_next_period--;
        }
        else
        {
            iters_to_next_period += period-1;
        }
        result_index ^= 1;
    } while ((it <= maxiter) && (!converged));
    clock_gettime(CLOCK_MONOTONIC, &after);
    r->time = (double)(after.tv_sec - before.tv_sec) + 
        (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    // Print to csv file for measuring 
    double flops_per_it = 12.0;
    double Flops = (double)p->N * (double)p->M * 
                    (double)(r->niter * flops_per_it +
                    (double)r->niter / p->period) / r->time;
    FILE *fpt;
    fpt = fopen("data_pth_v4.csv", "a+");
    fprintf(fpt,"% .6e, % .6e \n", r->time, Flops);
    fclose(fpt);

    free(temperature_old);
    free(temperature_new);
    free(conductivity);
    free(grid_parameters);
}

void * update_temperatures(void * grid_parameters)
{
    struct grid_data * parameters = (struct grid_data *) grid_parameters;
    const double * conductivity = parameters->conductivity;
    const int M = parameters->M;
    const double threshold = parameters->threshold;
    static const double c_cdir = 0.25 * SQRT2 / (SQRT2 + 1.0);
    static const double c_cdiag = 0.25 / (SQRT2 + 1.0);
    const int padding_left = parameters->padding_left;
    const int padding_right = parameters->padding_right;
    const int first_row = parameters->first_row;
    const int row_size = M + padding_left + padding_right;
    const int first_row_offset = first_row * row_size + padding_left;
    const int last_row = parameters->last_row;
    const int last_row_offset = last_row * row_size + padding_left;
    const int inner_end = (3 - padding_right);
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
    double maxdiff_thread, tmin_thread, tmax_thread, new_temperature, old_temperature, diff, c_c_dbl, rest_c_c_dbl; 
    __m256d c_c, rest_c_c;
    __m256d new_temperatures, old_temperatures, tsum_thread;
    double old_temperature_left, old_temperature_right;
    do {
        maxdiff_thread = 0.0;
        tmin_thread = DBL_MAX;
        tmax_thread = DBL_MIN;
        tsum_thread = _mm256_setzero_pd();
        converged_local = 1;
        for (row_offset=first_row_offset; row_offset < last_row_offset; row_offset+=row_size)
        {
            // Inner cells
            index_right_col = row_offset + M-1;
            for (index=row_offset+1; index < index_right_col - inner_end; index+=4)
            {
                old_temperatures = _mm256_load_pd(&temperature_old[index]);
                c_c = _mm256_load_pd(&conductivity[index - row_size]);
                rest_c_c = _mm256_sub_pd(_mm256_set1_pd(1.0), c_c);
                // Scale old temperature at the cell

                new_temperatures = _mm256_mul_pd(old_temperatures, c_c);

                // Adjacent neighbors
                new_temperatures = _mm256_add_pd(new_temperatures,_mm256_mul_pd(_mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_loadu_pd(&temperature_old[index-1]),_mm256_load_pd(&temperature_old[index - row_size])), _mm256_loadu_pd(&temperature_old[index+1])), 
                                    _mm256_load_pd(&temperature_old[index + row_size])),rest_c_c), _mm256_set1_pd(c_cdir))); 
                // Diagonal neighbors
                new_temperatures = _mm256_add_pd(new_temperatures,_mm256_mul_pd(_mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_loadu_pd(&temperature_old[index-1 - row_size]),_mm256_loadu_pd(&temperature_old[index+1 - row_size])), _mm256_loadu_pd(&temperature_old[index-1 + row_size])), 
                                    _mm256_load_pd(&temperature_old[index+1 + row_size])),rest_c_c), _mm256_set1_pd(c_cdiag))); 

                _mm256_store_pd(&temperature_new[index], new_temperatures);

                // Only converge if all values below threshold
                double difference[4] __attribute__ ((aligned (32)));
                _mm256_store_pd(difference, _mm256_sub_pd(old_temperatures, new_temperatures));
                for (int i=0; i<4; i++)
                {
                    if (difference[i] < 0)
                    {
                        difference[i] *= -1.0;
                    }
                    if (difference[i] > maxdiff_thread)
                    {
                        maxdiff_thread = difference[i];
                    }
                }
            }

            // Inner end to last cell
            for (; index < index_right_col; index++)
            {
                old_temperature = temperature_old[index];
                c_c_dbl = conductivity[index - row_size];

                rest_c_c_dbl = 1 - c_c_dbl;
                new_temperature = old_temperature * c_c_dbl;

                // Adjacent neighbors
                new_temperature += (temperature_old[index-1] + temperature_old[index - row_size] + temperature_old[index+1] + 
                                temperature_old[index+ row_size]) * rest_c_c_dbl * c_cdir;

                // Diagonal neighbors
                new_temperature += (temperature_old[index - 1 - row_size] + temperature_old[index +1 - row_size] + 
                                temperature_old[index -1 + row_size] + temperature_old[index +1 + row_size])
                                * rest_c_c_dbl * c_cdiag;

                temperature_new[index] = new_temperature;

                diff = fabs(old_temperature - new_temperature);
                if (diff > maxdiff_thread){
                    maxdiff_thread = diff;
                }
            }

            // Outer cells
            index_left_col = row_offset;
            old_temperature_left = temperature_old[index_left_col];
            old_temperature_right = temperature_old[index_right_col];

            // Left
            // Scale old temperature at the cell
            c_c_dbl = conductivity[index_left_col - row_size];
            rest_c_c_dbl = 1 - c_c_dbl;
            new_temperature = old_temperature_left * c_c_dbl;

            // Adjacent neighbors
            new_temperature += (old_temperature_right + temperature_old[index_left_col - row_size] + temperature_old[index_left_col+1] + 
                                temperature_old[index_left_col + row_size]) * rest_c_c_dbl * c_cdir;

            // Diagonal neighbors
            new_temperature += (temperature_old[index_right_col - row_size] + temperature_old[index_left_col+1 - row_size] + 
                                temperature_old[index_right_col + row_size] + temperature_old[index_left_col+1 + row_size])
                                * rest_c_c_dbl * c_cdiag;

            temperature_new[index_left_col] = new_temperature;

            // Only converge if all values below threshold
            diff = fabs(old_temperature_left - new_temperature);
            if (diff > maxdiff_thread){
                maxdiff_thread = diff;
            }

            // Right
            // Scale old temperature at the cell
            c_c_dbl = conductivity[index_right_col - row_size];
            rest_c_c_dbl = 1 - c_c_dbl;
            new_temperature = old_temperature_right * c_c_dbl;

            // Adjacent neighbors
            new_temperature += (old_temperature_left + temperature_old[index_right_col - row_size] + temperature_old[index_right_col-1] + 
                                temperature_old[index_right_col + row_size]) * rest_c_c_dbl * c_cdir;

            // Diagonal neighbors
            new_temperature += (temperature_old[index_left_col - row_size] + temperature_old[index_right_col-1 - row_size] + 
                                temperature_old[index_left_col + row_size] + temperature_old[index_right_col-1 + row_size])
                                * rest_c_c_dbl * c_cdiag;

            temperature_new[index_right_col] = new_temperature;

            // Only converge if all values below threshold
            diff = fabs(old_temperature_right - new_temperature);
            if (diff > maxdiff_thread){
                maxdiff_thread = diff;
            }
        }

        converged[2 * thread_id + result_index] = maxdiff_thread < threshold;

        // Check global convergence
        converged_global = 1;
        pthread_barrier_wait (&barrier);
        for (index = 0; index < nthreads; index++)
        {
            converged_global &= converged[2 * index + result_index];
        }

        // Go over temperatures and check minimum, maximum temperature
        if (!iters_to_next_period || iter == maxiter || converged_global){
            for (row_offset=first_row_offset; row_offset < last_row_offset; row_offset+=row_size)
            {
                // Left cell
                new_temperature = temperature_new[row_offset];
                if (new_temperature > tmax_thread){
                    tmax_thread = new_temperature;
                }
                if (new_temperature < tmin_thread){
                    tmin_thread = new_temperature;
                }
                tsum_thread = _mm256_add_pd(tsum_thread, _mm256_set_pd(new_temperature, 0.0, 0.0, 0.0));
                
                // Inner cells
                index_right_col = row_offset + M-1;

                for (index=row_offset+1; index < index_right_col - inner_end; index+=4)
                {
                    for (int element = 0; element < 4; element++)
                    {
                        new_temperature = temperature_new[index+element];
                        if (new_temperature > tmax_thread){
                            tmax_thread = new_temperature;
                        }
                        if (new_temperature < tmin_thread){
                            tmin_thread = new_temperature;
                        }
                    }

                    tsum_thread = _mm256_add_pd(tsum_thread, _mm256_load_pd(&temperature_new[index]));
                }

                // Last inner cells and right column
                for (; index <= index_right_col; index++)
                {
                    new_temperature = temperature_new[index];
                    if (new_temperature > tmax_thread){
                        tmax_thread = new_temperature;
                    }
                    if (new_temperature < tmin_thread){
                        tmin_thread = new_temperature;
                    }
                    tsum_thread = _mm256_add_pd(tsum_thread, _mm256_set_pd(new_temperature, 0.0, 0.0, 0.0));
                }
            }

            double tsum_elements[4] __attribute__ ((aligned (32)));
            _mm256_store_pd(tsum_elements, tsum_thread);

            parameters->tmin[result_index] = tmin_thread;
            parameters->tmax[result_index] = tmax_thread;
            parameters->tsum[result_index] = tsum_elements[0]+tsum_elements[1]+tsum_elements[2]+tsum_elements[3];
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