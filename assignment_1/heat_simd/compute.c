#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include <stdio.h>

#include <immintrin.h>

__m256d update_4(int index, int M,int N, double * temps_new,
 double * temps_old, double * conductivity, double * direct, double * indirect);
void initialise(const struct parameters* p);

double * temps_old;
double * temps_new;
double * conductivity;
double * direct;
double * indirect;

void print_grid(double *my_grid, int M,int N){
    /* Print temperature with halo cells */
    int i = 0;
    for (; i < N; ++i){
        for (int j = 0; j < M; ++j){
            printf("%10.3f ", my_grid[i * M + j]);
        }
        printf("\n");
    }
}

void do_compute(const struct parameters* p, struct results *r){
    initialise(p);
    // print_grid(indirect, p->M, p->N);

    // Measure time
    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    int it = 1;
    int grid_start = p->M;
    int grid_end = p->M  * (p->N + 1);
    int grid_size = (p->N * p->M);
    int converged;
    double maxdiff;
    double tmin;
    double tmax;
    __m256d tsum;

    do {
        maxdiff = 0.0;
        tmin = p->io_tmax;
        tsum = _mm256_set1_pd(0.0);
        tmax = p->io_tmin;
        // Check convergence every timestep
        converged = 1;

        for (int index = grid_start; index < grid_end; index += 4){
             __m256d new_res = update_4(p->M, p->M,p->N, temps_old,temps_new, conductivity, direct,indirect);
            double temperatures[4] __attribute__ ((aligned (32)));
            _mm256_store_pd(temperatures, new_res);
            printf("%f %f %f %f\n", temperatures[0], temperatures[1], temperatures[2], temperatures[3]);
 
            double difference[4] __attribute__ ((aligned (32)));
            __m256d cur_old_temperatures = _mm256_load_pd(&temps_old[index]);
            _mm256_store_pd(difference, _mm256_sub_pd(cur_old_temperatures, new_res));

            // _mm256_abs_ph only for availabe single precision --> element-wise abs
            double cur_max_difference = 0.0;
            for (int i=0; i<4; i++)
            {
                if (difference[i] < 0)
                {
                    difference[i] *= -1.0;
                }
                if (difference[i] > cur_max_difference)
                {
                    cur_max_difference = difference[i];
                }
            }

            // Continue loop if one difference > threshold
            if (cur_max_difference >= p->threshold){
                converged = 0;
            }
            
            if (it % p->period == 0 || converged || it == p->maxiter){
                // Update results 
                tsum = _mm256_add_pd(tsum, new_res);

                double new_temperatures_arr[4];
                _mm256_store_pd(new_temperatures_arr, new_res);
                // TODO: Avoid fmax, fmin because math libraries have to be linked in Makefile (gcc -lm)
                const double cur_max_temp = fmax(fmax(fmax(new_temperatures_arr[0], new_temperatures_arr[1]), new_temperatures_arr[2]), new_temperatures_arr[3]);
                const double cur_min_temp = fmin(fmin(fmin(new_temperatures_arr[0], new_temperatures_arr[1]), new_temperatures_arr[2]), new_temperatures_arr[3]);

                if (cur_max_difference > maxdiff){
                    maxdiff = cur_max_difference;
                }
                if (cur_max_temp > tmax){
                    tmax = cur_max_temp;
                }
                if (cur_min_temp < tmin){
                    tmin = cur_min_temp;
                }
            }
            
        }

        // TODO: Updates for the last elements
        
        // Update results
        if (it % p->period == 0 || converged || it == p->maxiter){
            r->niter = it;
            r->tmin = tmin;
            r->tmax = tmax;

             double tsums[4] __attribute__ ((aligned (32)));
            _mm256_store_pd(tsums, tsum);

            r->tavg = (tsums[0] + tsums[1] + tsums[2] + tsums[3])/grid_size;
            r->maxdiff = maxdiff;
            clock_gettime(CLOCK_MONOTONIC, &after);
            r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;
            
            if (it < p->maxiter && !converged && p->printreports){
                // Only call print if it's not the last iteration
                report_results(p,r);
            }
        }

        // Flip old and new values
        double * tmp = temps_old;
        temps_old = temps_new;
        temps_new = tmp;        

        ++ it; 
    } while ((it <= p->maxiter) && (!converged));

    // TODO: loop etc here
    _mm_free(temps_old);
    _mm_free(temps_new);
    _mm_free(conductivity);
    _mm_free(direct);
    _mm_free(indirect);
}


void initialise(const struct parameters* p){
    // initialise array
    int M = p->M;
    int N = p->N;
    int MN = M*N; 

    temps_old = (double *) _mm_malloc((N + 2) * M * sizeof(double), 32);
    temps_new = (double *) _mm_malloc((N + 2) * M * sizeof(double), 32);
    conductivity = (double *) _mm_malloc((N) * M * sizeof(double), 32); // TODO: check if right size
    direct = (double *) _mm_malloc((N) * M * sizeof(double), 32);
    indirect = (double *) _mm_malloc((N) * M * sizeof(double), 32);


    // Halo rows
    for (int index = 0; index < M; index++)
    {
        temps_old[index] = p->tinit[index];
        temps_new[index] = temps_old[index];
        temps_old[MN + M + index] = p->tinit[MN - M + index];
        temps_new[MN + M + index] = temps_old[MN + M + index];
    }
    
     // Fill the temperature values into the grid cells
    for (int index = 0; index < M * N; index++)
    {   
        temps_new[M+index] = p->tinit[index];
        double cur_conduct = p->conductivity[index];
        double joint_weight_diagonal_neighbors = (1 - cur_conduct) / (sqrt(2.0) + 1);
        double joint_weight_direct_neighbors = 1 - cur_conduct - joint_weight_diagonal_neighbors;

        // T(&cylinder_grid, M + index) = p->tinit[index];

        conductivity[index] = cur_conduct;
        direct[index] = joint_weight_direct_neighbors / 4.0;
        indirect[index] = joint_weight_diagonal_neighbors / 4.0;
    }
}
// printf("hallo\n");
// __m256d gelieza = _mm256_load_pd(temps_old);
// __m256d paul = _mm256_load_pd(temps_old+4);
// __m256d my_vector = _mm256_add_pd(gelieza,paul);
// __attribute__ ((aligned (32))) double output[4];
// _mm256_store_pd(output, my_vector);



// printf("%f %f %f %f\n",
//         output[0], output[1], output[2], output[3]);

__m256d update_4(int index, int M,int N, double * temps_new,
 double * temps_old, double * conductivity, double * direct, double * indirect)
{
    // generate index arrays
    int indices[4] = {index, index+1, index+2,index+3};
    int indices_left[4];
    int indices_right[4];
    int i;
    for(i=0; i<4;i++){
        indices_left[i] =  indices[i] - 1;
        indices_right[i] = indices[i] + 1;

        if (indices[i] % M == 0)
        {
            indices_left[i] = indices[i] + M - 1;
        }

        if (indices[i] % M == M - 1)
        {
            indices_right[i] = indices[i] - M + 1;
        }
    }
    
    // TODO: fix the aweful indexing there 
    // Load temperatures and condictivity

    __m256d cur_old_temps = _mm256_load_pd(&(temps_old[index]));
    __m256d cur_conduct = _mm256_load_pd(&conductivity[index-M]);
    
    __m256d left_direct = _mm256_set_pd(temps_old[indices_left[3]],temps_old[indices_left[2]],temps_old[indices_left[1]],temps_old[indices_left[0]]);
    __m256d left_indirect_top = _mm256_set_pd(temps_old[indices_left[3]-M],temps_old[indices_left[2]-M],temps_old[indices_left[1]-M],temps_old[indices_left[0]-M]);
    __m256d left_indirect_below = _mm256_set_pd(temps_old[indices_left[3]+M],temps_old[indices_left[2]+M],temps_old[indices_left[1]+M],temps_old[indices_left[0]+M]);

    __m256d right_direct = _mm256_set_pd(temps_old[indices_right[3]],temps_old[indices_right[2]],temps_old[indices_right[1]],temps_old[indices_right[0]]);
    __m256d right_indirect_top = _mm256_set_pd(temps_old[indices_right[3]-M],temps_old[indices_right[2]-M],temps_old[indices_right[1]-M],temps_old[indices_right[0]-M]);
    __m256d right_indirect_below = _mm256_set_pd(temps_old[indices_right[3]+M],temps_old[indices_right[2]+M],temps_old[indices_right[1]+M],temps_old[indices_right[0]+M]);

    __m256d cur_direct_top = _mm256_load_pd(&(temps_old[index-M]));
    __m256d cur_direct_below = _mm256_load_pd(&temps_old[index+M]);
    
    __m256d indirect_weight = _mm256_load_pd(&indirect[index-M]);
    __m256d direct_weight = _mm256_load_pd(&direct[index-M]);

    // Compute stuff
    __m256d cur_temp_res = _mm256_mul_pd(cur_old_temps,cur_conduct);

    __m256d indirect_temp_res = _mm256_add_pd(_mm256_add_pd(left_indirect_top,left_indirect_below), _mm256_add_pd(right_indirect_top,right_indirect_below));
    indirect_temp_res = _mm256_mul_pd(indirect_temp_res, indirect_weight);

    __m256d direct_temp_res = _mm256_add_pd(_mm256_add_pd(cur_direct_top,cur_direct_below), _mm256_add_pd(right_direct,left_direct));
    direct_temp_res = _mm256_mul_pd(direct_temp_res, direct_weight);

    // add to final sum and store
    __m256d final_sum = _mm256_add_pd(_mm256_add_pd(indirect_temp_res, direct_temp_res), cur_temp_res);
    _mm256_store_pd(&temps_new[index],final_sum);

    return final_sum; 
}