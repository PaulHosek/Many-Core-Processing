#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include <stdio.h>

void update_temperature_sums(int m,int n,double * temperatures, double * temperature_horizontal_sums, double * temperature_vertical_sums, int * indices_left, int * indices_right)
{
    const int mn = m*n;

    for (int index = m; index < mn + m; ++ index)
    {
        temperature_horizontal_sums[index] = temperatures[indices_left[index-m]] + temperatures[indices_right[index-m]];
        temperature_vertical_sums[index-m] = temperatures[index-m] + temperatures[index + m];
    }
}

void initialize(const struct parameters* p, double* temperature_old, double* temperature_new, double * temperature_horizontal_sums, double * temperature_vertical_sums,
double* conductivity, double * weight_direct, double * weight_indirect, int* indices_left, int * indices_right){
    int m = p->M;
    int n = p->N;
    int MN = m*n;

    // Halo rows
    for (int index = 0; index < m; index++)
    {
        temperature_old[index] = p->tinit[index];
        temperature_new[index] = p->tinit[index];
        temperature_old[MN + m + index] = p->tinit[MN - m + index];
        temperature_new[MN + m + index] = p->tinit[MN - m + index];
    }

    if (p->use_precomputed_sums)
    {
        for (int index = 0; index < p->M; index++)
        {
            const int lower_halo_row_offset = MN + m;
            // Temperature sums
            int index_left =  index - 1;
            int index_right = index + 1;
            if (index % p->M == 0)
            {
                index_left = index + p->M - 1;
            }
            if (index % p->M == p->M - 1)
            {
                index_right = index - p->M + 1;
            }
            // Upper halo
            temperature_horizontal_sums[index] = temperature_old[index_left] + temperature_old[index_right];
            // Lower halo
            temperature_horizontal_sums[index + lower_halo_row_offset] = temperature_old[index_left + lower_halo_row_offset] + temperature_old[index_right + lower_halo_row_offset];
        }
    }

    // Fill the temperature values into the grid cells
    #pragma GCC ivdep
    for (int index = 0; index < MN; index++)
    {
        temperature_old[m + index] = p->tinit[index];
        const double cond = p->conductivity[index];
        const double joint_weight_diagonal_neighbors = (1 - cond) / (sqrt(2.0) + 1);
        const double joint_weight_direct_neighbors = 1 - cond - joint_weight_diagonal_neighbors;

        conductivity[index] = cond;
        weight_direct[index] = joint_weight_direct_neighbors / 4.0;
        weight_indirect[index] = joint_weight_diagonal_neighbors / 4.0;
    }

    // Initialize the indices 
    #pragma GCC ivdep
    for (int index = m; index < MN + m; index ++){
        // Find indices of direct neighbors
        int index_left =  index - 1;
        int index_right = index + 1;
        if (index % m == 0)
        {
            index_left = index + m - 1;
        }
          
        if (index % m == m - 1)
        {
            index_right = index - m + 1;
        }
        indices_left[index - m] = index_left;
        indices_right[index - m] = index_right;
    }

    if (p->use_precomputed_sums) 
    {
        update_temperature_sums(m,n,temperature_old, temperature_horizontal_sums, temperature_vertical_sums,indices_left,indices_right);
    }
}

void do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    double * temperature_old = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * temperature_new = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * temperature_horizontal_sums = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * temperature_vertical_sums = (double * ) malloc(p->N * p->M * sizeof(double));
    double * conductivity = (double * ) malloc((p->N) * p->M * sizeof(double));
    double * weight_direct = (double * ) malloc((p->N) * p->M * sizeof(double));
    double * weight_indirect = (double * ) malloc((p->N) * p->M * sizeof(double));
    int * indices_left = (int * ) malloc ( p->N * p->M * sizeof(int));
    int * indices_right = (int * ) malloc ( p->N * p->M * sizeof(int));
    initialize(p, temperature_old, temperature_new, temperature_horizontal_sums, temperature_vertical_sums, conductivity, weight_direct, weight_indirect, indices_left, indices_right);

    // Measure time
    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    int it = 1;
    int grid_start = p->M;
    int m = p->M;
    int n = p->N;
    int grid_end = p->M  * (p->N + 1);
    int grid_size = (p->N * p->M);
    int converged;
    double threshold = p->threshold;
    double maxdiff;
    double tmin;
    double tmax;
    double tsum;

    do {
        maxdiff = 0.0;
        tmin = p->io_tmax;
        tsum = 0.0;
        tmax = p->io_tmin;
        // Check convergence every timestep
        converged = 1;

        #pragma GCC ivdep
        for (int index = grid_start; index < grid_end; ++ index){
            int index_left = indices_left[index - m];
            int index_right = indices_right[index - m];

            double new_temperature = temperature_old[index] * conductivity[index - m];
            if (p->use_precomputed_sums)
            {
                // Adjacent neighbors
                new_temperature += (temperature_horizontal_sums[index] + temperature_vertical_sums[index-m]) * weight_direct[index - m];
            
                // Diagonal neighbors
                new_temperature += (temperature_vertical_sums[index_left-m] + temperature_vertical_sums[index_right-m]) * weight_indirect[index - m];
            }
            else
            {
                // Adjacent neighbors
                new_temperature += (temperature_old[index_left] + temperature_old[index - m] + temperature_old[index_right] + 
                temperature_old[index + m]) * weight_direct[index - m];
            
                // Diagonal neighbors
                new_temperature += (temperature_old[index_left - m] + temperature_old[index_right - m] + 
                temperature_old[index_left + m] + temperature_old[index_right + m]) * weight_indirect[index - m];
            }
            
            temperature_new[index] = new_temperature;

            // Only converge if all values below threshold
            double diff = fabs(temperature_old[index] - temperature_new[index]);
            converged = converged & (diff < threshold);
        }
        
        // Go over temperatures and check minimum, maximum temperature and maximum difference
        if (it % p->period == 0 || converged || it == p-> maxiter){
            #pragma GCC ivdep
            for (int index = grid_start; index < grid_end; ++ index){
                tsum += temperature_new[index];
                if (temperature_new[index] > tmax){
                    tmax = temperature_new[index];
                }
                if (temperature_new[index] < tmin){
                    tmin = temperature_new[index];
                }
                double diff = fabs(temperature_new[index] - temperature_old[index]);
                if (diff > maxdiff){
                    maxdiff = diff;
                }
            }
        }

        // Update results
        if (it % p->period == 0 || converged || it == p-> maxiter){
            r->niter = it;
            r->tmin = tmin;
            r->tmax = tmax;
            r->tavg = tsum/grid_size;
            r->maxdiff = maxdiff;
            clock_gettime(CLOCK_MONOTONIC, &after);
            r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;
            
            if (it < p->maxiter && !converged && p->printreports){
                // Only call print if it's not the last iteration and the print-parameter is set 
                report_results(p,r);
            }
        }

        double * tmp = temperature_old;
        temperature_old = temperature_new;
        temperature_new = tmp; 

        if (p->use_precomputed_sums) 
        {
            update_temperature_sums(m,n,temperature_old,temperature_horizontal_sums,temperature_vertical_sums,indices_left,indices_right);
        }
        ++ it; 
    } while ((it <= p->maxiter) && (!converged));

    free(temperature_old);
    free(temperature_new);
    free(temperature_horizontal_sums);
    free(temperature_vertical_sums);
    free(conductivity);
    free(weight_direct);
    free(weight_indirect);
    free(indices_left);
    free(indices_right);
}
