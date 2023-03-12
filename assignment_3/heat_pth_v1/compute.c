#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <pthread.h>

#include "compute.h"

pthread_barrier_t barrier_start;
pthread_barrier_t barrier_finished;

struct grid_data {
   int  first;
   int  last;
   double * temperature_old;
   double * temperature_new;
   double * conductivity;
   double * weight_direct;
   double * weight_indirect;
   int * indices_left;
   int * indices_right;
   double tsum;
   double maxdiff;
   double tmin;
   double tmax;
   int compute_statistics;
   double threshold;
   int converged;
   int M;
   int * finish_execution;
};

void initialize(const struct parameters* p, double* temperature_old, double* temperature_new, 
double* conductivity, double * weight_direct, double * weight_indirect, int* indices_left, int * indices_right);
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
double* conductivity, double * weight_direct, double * weight_indirect, int* indices_left, int * indices_right)
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
        const double cond = p->conductivity[index];
        const double joint_weight_diagonal_neighbors = (1 - cond) / (sqrt(2.0) + 1);
        const double joint_weight_direct_neighbors = 1 - cond - joint_weight_diagonal_neighbors;
        conductivity[index] = cond;
        weight_direct[index] = joint_weight_direct_neighbors / 4.0;
        weight_indirect[index] = joint_weight_diagonal_neighbors / 4.0;
    }
    // Initialize the indices 
    for (index = M; index < MN + M; index ++){
        // Find indices of direct neighbors
        int index_left =  index - 1;
        int index_right = index + 1;
        if (index % M == 0)
        {
            index_left = index + M - 1;
        }
        if (index % M == M - 1)
        {
            index_right = index - M + 1;
        }
        indices_left[index - M] = index_left;
        indices_right[index - M] = index_right;
    }
}

void do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    const int num_threads = p->nthreads;
    double * temperature_old = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * temperature_new = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * conductivity = (double * ) malloc((p->N) * p->M * sizeof(double));
    double * weight_direct = (double * ) malloc((p->N) * p->M * sizeof(double));
    double * weight_indirect = (double * ) malloc((p->N) * p->M * sizeof(double));
    int * indices_left = (int * ) malloc ( p->N * p->M * sizeof(int));
    int * indices_right = (int * ) malloc ( p->N * p->M * sizeof(int));
    struct grid_data * grid_parameters = (struct grid_data *) malloc(num_threads * sizeof(struct grid_data));
    pthread_t thread_ids[num_threads];
    int it = 1;
    const int grid_start = p->M;
    const int grid_end = p->M  * (p->N + 1);
    const int grid_size = (p->N * p->M);
    const int grid_size_per_thread = grid_size / num_threads;
    const int maxiter = p->maxiter;
    const int period = p->period;
    const int M = p->M;
    int converged, compute_statistics, compute_statistics_new, finish_execution=0;
    double maxdiff, tmin, tmax, tsum, maxdiff_thread, tmin_thread, tmax_thread;

    initialize(p, temperature_old, temperature_new, conductivity, weight_direct, weight_indirect, indices_left, indices_right);
    compute_statistics_new = (it % period == 0 || it == maxiter);
    int index;
    for (index=0; index < num_threads; index++)
    {
        grid_parameters[index].temperature_old = temperature_old;
        grid_parameters[index].temperature_new = temperature_new;
        grid_parameters[index].conductivity = conductivity;
        grid_parameters[index].weight_direct = weight_direct;
        grid_parameters[index].weight_indirect = weight_indirect;
        grid_parameters[index].indices_left = indices_left;
        grid_parameters[index].indices_right = indices_right;
        grid_parameters[index].tsum = 0.0;
        grid_parameters[index].tmin = p->io_tmax;
        grid_parameters[index].tmax = p->io_tmin;
        grid_parameters[index].maxdiff = 0.0;
        grid_parameters[index].first = grid_start + index * grid_size_per_thread;
        grid_parameters[index].last = grid_start + (index+1) *grid_size_per_thread;
        grid_parameters[index].compute_statistics = compute_statistics_new;
        grid_parameters[index].threshold = p->threshold;
        grid_parameters[index].M = M;
        grid_parameters[index].finish_execution = &finish_execution;
    }
    grid_parameters[num_threads-1].last = grid_end;

    pthread_barrier_init(&barrier_start, NULL, num_threads+1);
    pthread_barrier_init(&barrier_finished, NULL, num_threads+1);

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);
    for (index=0; index<num_threads; index++) {
        pthread_create(&thread_ids[index],
        NULL,
        update_temperatures,
        (void *)&grid_parameters[index]);
    }
    pthread_barrier_wait (&barrier_start);
    do {
        maxdiff = 0.0;
        tmin = DBL_MAX;
        tsum = 0.0;
        tmax = DBL_MIN;
        // Check convergence every timestep
        converged = 1;
        compute_statistics = compute_statistics_new;

        pthread_barrier_wait (&barrier_finished);

        for (index=0; index<num_threads; index++) {
            converged &= grid_parameters[index].converged;
        }

        // Go over temperatures and check minimum, maximum temperature and maximum difference
        if (compute_statistics || converged){
            for (index = 0; index < num_threads; index++){
                tsum += grid_parameters[index].tsum;
                tmax_thread = grid_parameters[index].tmax;
                tmin_thread = grid_parameters[index].tmin;
                maxdiff_thread = grid_parameters[index].maxdiff;
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
        }

        if (it >= maxiter || converged)
        {
            finish_execution = 1;
        }
        else
        {
            compute_statistics_new = ((it+1) % period == 0 || (it+1) == maxiter);
            for (index=0; index<num_threads; index++) {
                grid_parameters[index].compute_statistics = compute_statistics_new;
            }
        }
        pthread_barrier_wait (&barrier_start);

        if (compute_statistics || converged){
            r->niter = it;
            r->tmin = tmin;
            r->tmax = tmax;
            r->tavg = tsum/grid_size;
            r->maxdiff = maxdiff;
            clock_gettime(CLOCK_MONOTONIC, &after);
            r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;
            
            if (it <= maxiter && !converged && p->printreports){
                // Only call print if it's not the last iteration and the print-parameter is set 
                report_results(p,r);
            }
        }
        it++;
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
    fpt = fopen("data.csv", "a+");
    fprintf(fpt,"% .6e, % .6e \n", r->time, Flops);
    fclose(fpt);

    free(temperature_old);
    free(temperature_new);
    free(conductivity);
    free(weight_direct);
    free(weight_indirect);
    free(indices_left);
    free(indices_right);
    free(grid_parameters);
}

void * update_temperatures(void * grid_parameters)
{
    struct grid_data * parameters = (struct grid_data *) grid_parameters;
    double * conductivity = parameters->conductivity;
    double * weight_direct = parameters->weight_direct;
    double * weight_indirect = parameters->weight_indirect;
    int * indices_left = parameters->indices_left;
    int* indices_right = parameters->indices_right;
    const int M = parameters->M;
    const double threshold = parameters->threshold;
    const int first = parameters->first;
    const int last = parameters->last;
    double * temperature_new = parameters->temperature_new;
    double * temperature_old = parameters->temperature_old;
    int * finish_execution = parameters->finish_execution;
    int index,converged;
    double maxdiff_thread, tmin_thread, tmax_thread, tsum_thread;

    while(1)
    {
        maxdiff_thread = 0.0;
        tmin_thread = DBL_MAX;
        tmax_thread = DBL_MIN;
        tsum_thread = 0.0;
        converged = 1;
        pthread_barrier_wait (&barrier_start);
        if (*finish_execution) return NULL;
        int compute_statistics = parameters->compute_statistics;

        for (index=first; index < last; index++)
        {
            int index_left = indices_left[index - M];
            int index_right = indices_right[index - M];

            const double old_temperature = temperature_old[index];

            // Scale old temperature at the cell
            double new_temperature = old_temperature * conductivity[index - M];

            // Adjacent neighbors
            new_temperature += (temperature_old[index_left] + temperature_old[index - M] + temperature_old[index_right] + 
                                temperature_old[index + M]) * weight_direct[index - M];

            // Diagonal neighbors
            new_temperature += (temperature_old[index_left - M] + temperature_old[index_right - M] + 
                                temperature_old[index_left + M] + temperature_old[index_right + M])
                                * weight_indirect[index - M];

            temperature_new[index] = new_temperature;

            // Only converge if all values below threshold
            double diff = fabs(old_temperature - new_temperature);
            if (diff > maxdiff_thread){
                maxdiff_thread = diff;
            }
            converged = converged & (diff < threshold);
        }

        // Go over temperatures and check minimum, maximum temperature
        if (compute_statistics || converged){
            for (int index = first; index < last; ++ index){
                const double new_temperature = temperature_new[index];
                tsum_thread += new_temperature;
                if (new_temperature > tmax_thread){
                    tmax_thread = new_temperature;
                }
                if (new_temperature < tmin_thread){
                    tmin_thread = new_temperature;
                }
            }

            parameters->tmin = tmin_thread;
            parameters->tmax = tmax_thread;
            parameters->tsum = tsum_thread;
            parameters->maxdiff = maxdiff_thread;
            parameters->converged = converged;
        }

        pthread_barrier_wait (&barrier_finished);

        {double * tmp = temperature_old; temperature_old = temperature_new; temperature_new = tmp;}
    }
}
