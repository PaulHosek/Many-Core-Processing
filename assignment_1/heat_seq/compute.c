#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include "grid.c"

double update(int index, struct grid * grid)
{
    const int n = grid->N;

    int index_left =  index - 1;
    int index_right = index + 1;

    if (index % n == 0)
    {
        index_left = index + n - 1;
    }
    
    if (index % n == n - 1)
    {
        index_right = index - n + 1;
    }

    double new_temperature = 0.0;

    // Scaled old temperature at the cell
    new_temperature += T(grid, index) * C(grid, index);

    // Adjacent neighbors
    new_temperature += (T(grid, index_left) + T(grid, index - n) + T(grid, index_right) + T(grid, index + n))
                    * WD(grid, index);
    
    // Diagonal neighbors
    new_temperature += (T(grid, index_left - n) + T(grid, index_right - n) + T(grid, index_left + n) + T(grid, index_right + n))
                    * WI(grid, index);

    TN(grid, index) = new_temperature;

    return new_temperature;
}


void do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    struct grid grid = initialize(p);

    // Measure time
    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    int it = 0;
    int converged = 1;
    do {
        double maxdiff = 0.0;
        double tmin = p->io_tmax;
        double tsum = 0.0;
        double tmax = p->io_tmin;
        // Check convergence every timestep
        int converged = 1;

        for (int index = p->N; index < p->N * (p->M + 1); ++ index){
            double new_temperature = update(index, &grid);

            double diff = abs(T(&grid, index) - new_temperature);

            // Check all-below-threshold condition
            if (diff >= p->threshold){
                converged = 0;
            }

            if (it % p->period == 0){
                // Update results 
                tsum += new_temperature;

                if (diff > maxdiff){
                    maxdiff = diff;
                }
                if (new_temperature > tmax){
                    tmax = new_temperature;
                }
                if (new_temperature < tmin){
                    tmin = new_temperature;
                }
            }
            
        }

        // Report results
        if (it % p->period == 0){
            r->niter = it;
            r->tmin = tmin;
            r->tmax = tmax;
            r->tavg = tsum/(p->N * p->M);
            r->maxdiff = maxdiff;
            clock_gettime(CLOCK_MONOTONIC, &after);
            r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;
            report_results(p, r);
        }

        // Flip old and new values
        grid.old ^= 1;

        ++ it; 
    } while ((it < p->maxiter) && (!converged));
}

