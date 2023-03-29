#include "cuda_compute.h"
#include "input.h"
#include "output.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    struct parameters p;
    struct results r;

    read_parameters(&p, argc, argv);

    cuda_do_compute(&p, &r);

    report_results(&p, &r);

    return 0;
}
