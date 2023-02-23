#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include <stdio.h>
#include "grid.h"
#include <immintrin.h>


struct grid initialize(const struct parameters* p)
{
    struct grid cylinder_grid;

    cylinder_grid.M = p->M;
    cylinder_grid.N = p->N;
    cylinder_grid.old = 0;
    int MN = p->N * p-> M; 

    cylinder_grid.points = (struct pointType *) malloc((p->N + 2) * p->M * sizeof(struct pointType));

    // Halo rows
    for (int index = 0; index < p->M; index++)
    {
        T(&cylinder_grid, index) = p->tinit[index];
        TN(&cylinder_grid, index) = T(&cylinder_grid, index);
        T(&cylinder_grid, MN + p->M + index) = p->tinit[MN - p->M + index];
        TN(&cylinder_grid, MN + p->M + index) = T(&cylinder_grid, MN + p->M + index);
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
double update(int index, struct grid * restrict grid)
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
    // double new_temperature = vector_temp(grid, m, index, index_left,index_right);

    return new_temperature;
}

// update 4x
double* update_4(int index, struct grid * restrict grid)
{
    const int m = grid->M;

    // generate index arrays
    int indices[] = {index, index+1, index+2,index+3};
    int indices_left[4];
    int indices_right[4];
    int i;
    for(i=0; i<4;i++){
        indices_left[i] =  indices[i] - 1;
        indices_right[i] = indices[i] + 1;

        if (indices[i] % m == 0)
        {
            indices_left[i] = indices[i] + m - 1;
        }

        if (indices[i] % m == m - 1)
        {
            indices_right[i] = indices[i] - m + 1;
        }
    }


    double * return_temperatures = malloc(sizeof(double)*4);

    // 1.
    return_temperatures[0] += T(grid, indices[0]) * C(grid, indices[0]);
    return_temperatures[0] += (T(grid, indices_left[0]) + T(grid, indices[0] - m) + T(grid, indices_right[0]) + T(grid, indices[0] + m))
                    * WD(grid, indices[0]);
    return_temperatures[0] += (T(grid, indices_left[0] - m) + T(grid, indices_right[0] - m) + T(grid, indices_left[0] + m) + T(grid, indices_right[0] + m))
                    * WI(grid, indices[0]);
    TN(grid, index) = return_temperatures[0];


    // 2.
    return_temperatures[1] += T(grid, indices[1]) * C(grid, indices[1]);
    return_temperatures[1] += (T(grid, indices_left[1]) + T(grid, indices[1] - m) + T(grid, indices_right[1]) + T(grid, indices[1] + m))
                    * WD(grid, indices[1]);
    return_temperatures[1] += (T(grid, indices_left[1] - m) + T(grid, indices_right[1] - m) + T(grid, indices_left[1] + m) + T(grid, indices_right[1] + m))
                    * WI(grid, indices[1]);
    TN(grid, index) = return_temperatures[1];


    // 3.
    return_temperatures[2] += T(grid, indices[2]) * C(grid, indices[2]);
    return_temperatures[2] += (T(grid, indices_left[2]) + T(grid, indices[2] - m) + T(grid, indices_right[2]) + T(grid, indices[2] + m))
                    * WD(grid, indices[2]);
    return_temperatures[2] += (T(grid, indices_left[2] - m) + T(grid, indices_right[2] - m) + T(grid, indices_left[2] + m) + T(grid, indices_right[2] + m))
                    * WI(grid, indices[2]);
    TN(grid, index) = return_temperatures[2];

    // 4.
    return_temperatures[3] += T(grid, indices[3]) * C(grid, indices[3]);
    return_temperatures[3] += (T(grid, indices_left[3]) + T(grid, indices[3] - m) + T(grid, indices_right[3]) + T(grid, indices[3] + m))
                    * WD(grid, indices[3]);
    return_temperatures[3] += (T(grid, indices_left[3] - m) + T(grid, indices_right[3] - m) + T(grid, indices_left[3] + m) + T(grid, indices_right[3] + m))
                    * WI(grid, indices[3]);
    TN(grid, index) = return_temperatures[3];



    return return_temperatures; // returns array now
}


void do_compute(const struct parameters* p, struct results *r)
{
    // Initialize grid 
    struct grid grid = initialize(p);

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
    double tsum;
<<<<<<< HEAD
    int called = 0;
    int single_lp_idx;
=======
>>>>>>> 54681b63eac01b5422eecc3b5c4bcdbd2bc5c6ff

    do {
        maxdiff = 0.0;
        tmin = p->io_tmax;
        tsum = 0.0;
        tmax = p->io_tmin;
        single_lp_idx = 0;
        // Check convergence every timestep
        converged = 1;


        for (int index = grid_start; index < grid_end; index+= 4){
            // if fewer than 4 items are left
            if (grid_end - index < 4) {
                printf("break %d", grid_end-index);
                single_lp_idx = index;
                break;
            }
<<<<<<< HEAD
            double *temps = update_4(index, &grid);

            // update statistics
            int index_array[] = {index,index+1,index+2,index+3};
            int i;
            for (i=0;i<4;i++){
                
                double diff = fabs(T(&grid, index_array[i]) - temps[i]);
                // Continue loop if one difference > threshold
                if (diff >= p->threshold){
                    converged = 0;
=======
            
            if (it % p->period == 0 || converged || it == p->maxiter){
                // Update results 
                tsum += new_temperature;

                if (diff > maxdiff){
                    maxdiff = diff;
                }
                if (new_temperature > tmax){
                    tmax = new_temperature;
>>>>>>> 54681b63eac01b5422eecc3b5c4bcdbd2bc5c6ff
                }
                
                if (it % p->period == 0){
                    called ++;
                    // Update results 
                    tsum += temps[i];

                    if (diff > maxdiff){
                        maxdiff = diff;
                    }
                    if (temps[i] > tmax){
                        tmax = temps[i];
                    }
                    if (temps[i] < tmin){
                        tmin = temps[i];
                    }

            // free(temps); //TODO: idk why this is not needed

                }
            }
            if (single_lp_idx){
                printf("second loop");
                // need a single loop here for when there are <4 items left, note len(testfile) is divisable by 4.
                for (int index = single_lp_idx; index < grid_end; ++ index){
                    // printf("Second loop END:%d, INDEX:%d    ", grid_end, index);
                    double new_temperature = update(index, &grid);

                    double diff = fabs(T(&grid, index) - new_temperature);

                    // Continue loop if one difference > threshold
                    if (diff >= p->threshold){
                        converged = 0;
                    }
                    
                    if (it % p->period == 0){
                        called ++;
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
            }
        }

        // Update results
        if (it % p->period == 0 || converged || it == p->maxiter){
            r->niter = it;
            r->tmin = tmin;
            r->tmax = tmax;
            r->tavg = tsum/grid_size;
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
        grid.old ^= 1;

        ++ it; 
    } while ((it <= p->maxiter) && (!converged));


    free(grid.points);
}