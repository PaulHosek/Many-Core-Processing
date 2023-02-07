#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "compute.h"

/// Replace/erase the following line:
#include "grid.c"

void do_compute(const struct parameters* p, struct results *r)
{
/// Replace/erase the following line:
#include "ref2.c"
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
