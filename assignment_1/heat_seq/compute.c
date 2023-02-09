#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include <stdio.h>
#include "grid.h"


struct grid initialize(const struct parameters* p)
{
    struct grid cylinder_grid;

    cylinder_grid.M = p->M;
    cylinder_grid.N = p->N;

    cylinder_grid.points = (struct pointType *) malloc((p->N + 2) * p->M * sizeof(struct pointType));

    // Halo rows
    for (int index = 0; index < p->M; index++)
    {
        T(&cylinder_grid, index) = p->tinit[index];
        T(&cylinder_grid, (p->N + 1) * p->M + index) = p->tinit[(p->N - 1) * p->M + index];
    }

    // Fill the temperature values into the grid cells
    for (int index = 0; index < p->M * p->N; index++)
    {
        T(&cylinder_grid, p->M + index) = p->tinit[index];

        const double conductivity = p->conductivity[index];
        const double joint_weight_diagonal_neighbors = (1 - conductivity) / (sqrt(2.0) + 1);
        const double joint_weight_direct_neighbors = 1 - conductivity - joint_weight_diagonal_neighbors;

        C(&cylinder_grid, p->M + index) = conductivity;
        WD(&cylinder_grid, p->M + index) = joint_weight_direct_neighbors / 4.0;
        WI(&cylinder_grid, p->M + index) = joint_weight_diagonal_neighbors / 4.0;
    }

    return cylinder_grid;
}

double update(int index, struct grid * grid)
{
    const int m = grid->M;

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

    double new_temperature = 0.0;

    // Scaled old temperature at the cell
    new_temperature += T(grid, index) * C(grid, index);

    // Adjacent neighbors
    new_temperature += (T(grid, index_left) + T(grid, index - m) + T(grid, index_right) + T(grid, index + m))
                    * WD(grid, index);
    
    // Diagonal neighbors
    new_temperature += (T(grid, index_left - m) + T(grid, index_right - m) + T(grid, index_left + m) + T(grid, index_right + m))
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

    int it = 1;
    int converged;
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


        for (int index = p->M; index < p->M * (p->N + 1); ++ index){
            double new_temperature = update(index, &grid);

            double diff = fabs(T(&grid, index) - new_temperature);

            // Continue loop if one difference > threshold
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

        // Update results
        if (it % p->period == 0){
            r->niter = it;
            r->tmin = tmin;
            r->tmax = tmax;
            r->tavg = tsum/(p->N * p->M);
            r->maxdiff = maxdiff;
            clock_gettime(CLOCK_MONOTONIC, &after);
            r->time = (double)(after.tv_sec - before.tv_sec) +
              (double)(after.tv_nsec - before.tv_nsec) / 1e9;
            report_results(p,r);
        }

        // Flip old and new values
        grid.old ^= 1;

        ++ it; 
    } while ((it <= p->maxiter) && (!converged));

    print_grid(&grid);
    free(grid.points);
}