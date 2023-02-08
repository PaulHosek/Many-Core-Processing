#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include <stdio.h>
#include "grid.c"

void update_temperature_sums(struct grid * grid)
{
    const int M = grid->M;
    const int N = grid->N;

    for (int index = N; index < N * (M + 1); ++ index)
    {
        int index_left =  index - 1;
        int index_right = index + 1;

        if (index % N == 0)
        {
            index_left = index + N - 1;
        }

        if (index % N == N - 1)
        {
            index_right = index - N + 1;
        }

        TSH(grid, index) = T(grid, index_left) + T(grid, index_right);
        TSV(grid, index) = T(grid, index - N) + T(grid, index + N);
    }
}

struct grid initialize(const struct parameters* p)
{
    struct grid cylinder_grid;

    cylinder_grid.M = p->M;
    cylinder_grid.N = p->N;

    cylinder_grid.points = (struct pointType *) malloc((p->M + 2) * p->N * sizeof(struct pointType));

    // Halo rows
    for (int index = 0; index < p->N; index++)
    {
        T(&cylinder_grid, index) = p->tinit[index];
        T(&cylinder_grid, (p->M + 1) * p->N + index) = p->tinit[(p->M - 1) * p->N + index];
    }

    // Fill the temperature values into the grid cells
    const double denominator = sqrt(2.0) + 1;
    for (int index = 0; index < p->N * p->M; index++)
    {
        T(&cylinder_grid, p->N + index) = p->tinit[index];

        const double conductivity = p->conductivity[index];
        const double joint_weight_diagonal_neighbors = (1 - conductivity) / denominator;
        const double joint_weight_direct_neighbors = 1 - conductivity - joint_weight_diagonal_neighbors;

        C(&cylinder_grid, p->N + index) = conductivity;
        WD(&cylinder_grid, p->N + index) = joint_weight_diagonal_neighbors / 4.0;
        WI(&cylinder_grid, p->N + index) = joint_weight_direct_neighbors / 4.0;
    }
    update_temperature_sums(&cylinder_grid);
    return cylinder_grid;
}

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

    // Scaled old temperature at the cell
    double new_temperature = T(grid, index) * C(grid, index);

    // Adjacent neighbors
    new_temperature += (TSV(grid, index) + TSH(grid, index))
                    * WD(grid, index);
    
    // Diagonal neighbors
    new_temperature += (TSV(grid, index_left) + TSV(grid, index_right))
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
        converged = 1;
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
        update_temperature_sums(&grid);
        ++ it;
    } while ((it < p->maxiter) && (!converged));
}

