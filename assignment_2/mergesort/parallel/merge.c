#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

/* Ordering of the vector */
typedef enum Ordering {ASCENDING, DESCENDING, RANDOM} Order;

int debug = 0;


void TopDownSplitMerge(int * v_source, long first, long last, int * v_dest, long thread_input_size);
void msort(int *v, long l, int threads, long thread_input_size);
void print_v(int *v, long l);

void TopDownSplitMerge(int * v_source, long first, long last, int * v_dest, long thread_input_size) {
    const long size = last - first;
    if (size <= 1) {
        return;
    }

    const long mid = (last + first) / 2;

    #pragma omp task shared(v_source, v_dest) if(mid-first >= thread_input_size)
    TopDownSplitMerge(v_dest, first, mid, v_source, thread_input_size);
    #pragma omp task shared(v_source, v_dest) if(last-mid >= thread_input_size)
    TopDownSplitMerge(v_dest, mid, last, v_source, thread_input_size);
    #pragma omp taskwait

    #pragma omp task shared(v_source, v_dest) if(size >= thread_input_size)
    {
        long i = first;
        long j = mid;
        for (long k = first; k < last; k++) {
            if (i < mid && (j >= last || v_source[i] <= v_source[j])) {
                v_dest[k] = v_source[i];
                i++;
            } else {
                v_dest[k] = v_source[j];
                j++;
            }
        }
    }
    #pragma omp taskwait
}

void msort(int *v, long l, int threads, long thread_input_size) {
    int * v_temp = malloc(l*sizeof(int));
    memcpy(v_temp, v, l * sizeof(int));
    #pragma omp parallel num_threads(threads) if(l >= thread_input_size)
    {
        #pragma omp master
        {
        #pragma omp task
            TopDownSplitMerge(v_temp, 0, l, v, thread_input_size);
        }
    }
    free(v_temp);
}

void print_v(int *v, long l) {
    printf("\n");
    for(long i = 0; i < l; i++) {
        if(i != 0 && (i % 10 == 0)) {
            printf("\n");
        }
        printf("%d ", v[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {

    int c;
    int seed = 42;
    long length = 1e4;
    long thread_min_input_size = 1000;
    int num_threads = 16;
    Order order = ASCENDING;
    int *vector;
    char *output_file = NULL;

    struct timespec before, after;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "adrgp:l:s:o:m:")) != -1) {
        switch(c) {
            case 'a':
                order = ASCENDING;
                break;
            case 'd':
                order = DESCENDING;
                break;
            case 'r':
                order = RANDOM;
                break;
            case 'l':
                length = atol(optarg);
                break;
            case 'm':
                thread_min_input_size = atol(optarg);
                break;
            case 'g':
                debug = 1;
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'p':
                num_threads = atoi(optarg);
                break;
            case 'o':
            {
                const size_t str_len = strlen(optarg);
                if (!(output_file = malloc((str_len + 1) * sizeof(char))))
                {
                    fprintf(stderr, "Malloc failed...\n");
                    return -1;
                }
                strcpy(output_file, optarg);
                break;
            }
            case '?':
                if(optopt == 'l' || optopt == 's') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if(isprint(optopt)) {
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                }
                else {
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                }
                return -1;
            default:
                return -1;
        }
    }

    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    /* Allocate vector. */
    vector = (int*)malloc(length*sizeof(int));
    if(vector == NULL) {
        fprintf(stderr, "Malloc failed...\n");
        return -1;
    }

    /* Fill vector. */
    switch(order){
        case ASCENDING:
            for(long i = 0; i < length; i++) {
                vector[i] = (int)i;
            }
            break;
        case DESCENDING:
            for(long i = 0; i < length; i++) {
                vector[i] = (int)(length - i);
            }
            break;
        case RANDOM:
            for(long i = 0; i < length; i++) {
                vector[i] = rand();
            }
            break;
    }

    if(debug) {
        print_v(vector, length);
    }

    clock_gettime(CLOCK_MONOTONIC, &before);
    /* Sort */
    msort(vector, length, num_threads, thread_min_input_size);
    // test if successful sorting
    // for (long i =0; i<length; i++){
    //     printf("%d ", vector[i]);
    // }
    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;
    printf("Mergesort took: % .6e seconds \n", time);
    
    if (output_file)
    {
        FILE * output;
        output = fopen(output_file, "a");
        if (!output)
        {
            fprintf(stderr, "invalid output file\n");
        }
        fprintf(output, "% .6e\n", time);
        fclose(output);
    }
    if(debug) {
        print_v(vector, length);
    }

    if (output_file) free(output_file);
    if (vector) free(vector);
    return 0;
}