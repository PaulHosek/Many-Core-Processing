#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "compute.h"

const double SQRT2 = sqrt(2);

void initialize(const struct parameters* p, double* temperature_old, double* temperature_new, 
double* conductivity);

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
    double * temperature_old = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * temperature_new = (double * ) malloc((p->N + 2) * p->M * sizeof(double));
    double * conductivity = (double * ) malloc((p->N) * p->M * sizeof(double));
    const double c_cdir = 0.25 * SQRT2 / (SQRT2 + 1.0);
    const double c_cdiag = 0.25 / (SQRT2 + 1.0);
    const double threshold = p->threshold;
    int it = 1;
    const int N = p->N;
    const int grid_end = p->M  * (p->N + 1);
    const int grid_size = (p->N * p->M);
    const int maxiter = p->maxiter;
    const int period = p->period;
    int iters_to_next_period = period-1;
    const int M = p->M;
    int row, col, index_left_col, index_right_col, converged, compute_statistics, row_offset=0;
    double new_temperature, old_temperature, old_temperature_left, old_temperature_right, maxdiff, tmin, tmax, tsum, c_c, rest_c_c, diff;

    initialize(p, temperature_old, temperature_new, conductivity);
    
    int index;

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);
    do {
        maxdiff = 0.0;
        tmin = DBL_MAX;
        tsum = 0.0;
        tmax = DBL_MIN;
        row_offset = 0;
        // Check convergence every timestep
        converged = 1;
        compute_statistics = (!iters_to_next_period || it == maxiter);

        for (row_offset=M; row_offset < grid_end; row_offset+=M)
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
                if (diff > maxdiff){
                    maxdiff = diff;
                }
                converged = converged & (diff < threshold);
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
            if (diff > maxdiff){
                maxdiff = diff;
            }
            converged = converged & (diff < threshold);

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
            if (diff > maxdiff){
                maxdiff = diff;
            }
            converged = converged & (diff < threshold);
        }

        // Go over temperatures and check minimum, maximum temperature and maximum difference
        if (compute_statistics || converged){
            for (index = M; index < grid_end; ++index){
                new_temperature = temperature_new[index];
                tsum += new_temperature;
                if (new_temperature > tmax){
                    tmax = new_temperature;
                }
                if (new_temperature < tmin){
                    tmin = new_temperature;
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
        // Flip old and new values
        {double * tmp = temperature_old; temperature_old = temperature_new; temperature_new = tmp;} 
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

    // Print to csv file for measuring 
    double flops_per_it = 12.0;
    double Flops = (double)p->N * (double)p->M * 
                    (double)(r->niter * flops_per_it +
                    (double)r->niter / p->period) / r->time;
                    

    free(temperature_old);
    free(temperature_new);
    free(conductivity);
}