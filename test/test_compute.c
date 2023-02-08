#include <stdlib.h>
#include <stdio.h>
#include "../assignment_1/heat_seq/grid.c"
#include "../assignment_1/heat_seq/compute.c"


int main(int argc, char **argv)
{
    struct parameters p;
    struct results r;

    read_parameters(&p, argc, argv);

    struct grid grid = initialize(&p);

    printf("Get point N+2:  \n", T(&grid, p.N +2));

    free(grid.points);

    return 0;
}