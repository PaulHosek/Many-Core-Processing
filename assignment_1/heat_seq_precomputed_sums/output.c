#include "output.h"
//#include "config.h"
#include <stdio.h>

#define FPOPS_PER_POINT_PER_ITERATION (                 \
        1     /* current point 1 mul */ +               \
        3 + 1 /* direct neighbors 3 adds + 1 mul */ +   \
        3 + 1 /* diagonal neighbors 3 adds + 1 mul */ + \
        2     /* final add */ +                         \
        1     /* difference old/new */                  \
        )

void report_results(const struct parameters *p, const struct results *r)
{
    FILE * output;
    output = fopen(p->output_file, "a");
    if (!output)
    {
        die("invalid output file");
    }
    static volatile int init = 0;
    if (!init) {
        fprintf(output, "Output:\n\n"
               "%13s %13s %13s %13s %13s %13s %13s\n",
               //PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_BUGREPORT,
               "Iterations",
               "T(min)", "T(max)", "T(diff)", "T(avg)", "Time", "FLOP/s");
        init = 1;
    }

    fprintf(output, "%-13zu % .6e % .6e % .6e % .6e % .6e % .6e\n",
           r->niter,
           r->tmin,
           r->tmax,
           r->maxdiff,
           r->tavg,
           r->time,
           (double)p->N * (double)p->M * 
           (double)(r->niter * FPOPS_PER_POINT_PER_ITERATION +
                    (double)r->niter / p->period) / r->time);
    
    fclose(output);
}

