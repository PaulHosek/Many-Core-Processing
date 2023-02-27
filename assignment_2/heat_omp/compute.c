#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <stdio.h>

#include "compute.h"

#define NUM_THREADS 16

void initialize(const struct parameters* p, double* temperature_old, double* temperature_new, 
double* conductivity, double * weight_direct, double * weight_indirect, int* indices_left, int * indices_right)
{
    int MN = p->N * p-> M; 
    int m = p->M;
    int index;
    #pragma parallel num_threads (NUM_THREADS)
    {
        #pragma omp for nowait
        // Halo rows
        for (index = 0; index < m; index++)
        {
            temperature_old[index] = p->tinit[index];
            temperature_new[index] = p->tinit[index];
            temperature_old[MN + m + index] = p->tinit[MN - m + index];
            temperature_new[MN + m + index] = p->tinit[MN - m + index];
        }

        // Fill the temperature values into the grid cells
        #pragma omp for nowait
        for (index = 0; index < MN; index++)
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
        #pragma omp for
        for (index = m; index < MN + m; index ++){
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
        
    }
}

void do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    double * temperature_old = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * temperature_new = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * conductivity = (double * ) malloc((p->N) * p->M * sizeof(double));
    double * weight_direct = (double * ) malloc((p->N) * p->M * sizeof(double));
    double * weight_indirect = (double * ) malloc((p->N) * p->M * sizeof(double));
    int * indices_left = (int * ) malloc ( p->N * p->M * sizeof(int));
    int * indices_right = (int * ) malloc ( p->N * p->M * sizeof(int));
    initialize(p, temperature_old, temperature_new, conductivity, weight_direct, weight_indirect, indices_left, indices_right);

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    int it = 1;
    int grid_start = p->M;
    int m = p->M;
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
        int index;

        #pragma omp parallel for num_threads (NUM_THREADS) reduction(&: converged)
        for (index = grid_start; index < grid_end; ++ index){
            int index_left = indices_left[index - m];
            int index_right = indices_right[index - m];

            // Scaled old temperature at the cell
            double new_temperature = temperature_old[index] * conductivity[index - m];

            // Adjacent neighbors
            new_temperature += (temperature_old[index_left] + temperature_old[index - m] + temperature_old[index_right] + 
                temperature_old[index + m]) * weight_direct[index - m];
            
                // Diagonal neighbors
            new_temperature += (temperature_old[index_left - m] + temperature_old[index_right - m] + 
                temperature_old[index_left + m] + temperature_old[index_right + m])
                    * weight_indirect[index - m];

            temperature_new[index] = new_temperature;

            // Only converge if all values below threshold
            double diff = fabs(temperature_old[index] - temperature_new[index]);
            converged = converged & (diff < threshold);
        }

        // Go over temperatures and check minimum, maximum temperature and maximum difference
        if (it % p->period == 0 || converged || it == p-> maxiter){
            #pragma omp parallel for num_threads (NUM_THREADS) reduction(+: tsum) reduction(max: tmax) reduction(min: tmin) reduction(max: maxdiff)
            for (index = grid_start; index < grid_end; ++ index){
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

        // Flip old and new values
        double * tmp = temperature_old;
        temperature_old = temperature_new;
        temperature_new = tmp; 

        ++ it; 
    } while ((it <= p->maxiter) && (!converged));

    // Print to csv file for measuring 
    double flops_per_it = 12.0;
    double Flops = (double)p->N * (double)p->M * 
                    (double)(r->niter * flops_per_it +
                    (double)r->niter / p->period) / r->time;
    FILE *fpt;
    fpt = fopen("data.csv", "a+");
    fprintf(fpt,"% .6e, % .6e \n", r->time, Flops);
    fclose(fpt);

    clock_gettime(CLOCK_MONOTONIC, &after);
    r->time = (double)(after.tv_sec - before.tv_sec) + 
        (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    free(temperature_old);
    free(temperature_new);
    free(conductivity);
    free(weight_direct);
    free(weight_indirect);
    free(indices_left);
    free(indices_right);
}
