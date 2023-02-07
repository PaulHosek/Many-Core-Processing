#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"
#include <stdio.h>

/// Replace/erase the following line:
#include "grid.c"

struct grid initialize(const struct parameters *);
double update(int, struct grid *);

void do_compute(const struct parameters* p, struct results *r)
{
/// Replace/erase the following line:
struct grid grid = initialize(p);
printf("1. temperature: %f\n", T(&grid, 0));
#include "ref2.c"
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
    for (int index = 0; index < p->N * p->M; index++)
    {
        T(&cylinder_grid, p->N + index) = p->tinit[index];

        const double conductivity = p->conductivity[index];
        const double joint_weight_diagonal_neighbors = (1 - conductivity) / (sqrt(2.0) + 1);
        const double joint_weight_direct_neighbors = 1 - conductivity - joint_weight_diagonal_neighbors;

        C(&cylinder_grid, p->N + index) = conductivity;
        WD(&cylinder_grid, p->N + index) = joint_weight_diagonal_neighbors / 4.0;
        WI(&cylinder_grid, p->N + index) = joint_weight_direct_neighbors / 4.0;
    }

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
