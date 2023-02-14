#include "grid.h"
#include <stdio.h>

void print_grid(struct grid * grid){
    /* Print temperature with halo cells */
    for (int i = 0; i < grid->N+2; ++i){
        for (int j = 0; j < grid->M; ++j){
            printf("%10.3f ", T(grid, (i) * grid->M + j));
        }
        printf("\n");
    }
}