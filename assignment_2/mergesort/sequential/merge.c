#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <omp.h>
#include <string.h> /* for memcpy

/* Ordering of the vector */
typedef enum Ordering {ASCENDING, DESCENDING, RANDOM} Order;

int debug = 0;

void TopDownSplitMerge(int * v_source, long first, long last, int * v_dest);
void msort(int *v, long l);
void print_v(int *v, long l);

void TopDownSplitMerge(int * v_source, long first, long last, int * v_dest) {
    const long size = last - first;
    if (size <= 1) {
        return;
    }

    const long mid = (last + first) / 2;

    TopDownSplitMerge(v_dest, first, mid, v_source);
    TopDownSplitMerge(v_dest, mid, last, v_source);
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


void msort(int *v, long l) {
    int * v_temp = malloc(l*sizeof(int));
    memcpy(v_temp, v, l * sizeof(int));
    TopDownSplitMerge(v_temp, 0, l, v);
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
    int num_threads = 1;
    Order order = ASCENDING;
    int *vector;
    char *output_file = "data.csv";

    struct timespec before, after;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "adrgp:l:s:o:")) != -1) {
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
    msort(vector, length);

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Mergesort took: % .6e seconds \n", time);
    FILE * output;
    output = fopen(output_file, "a");
    if (!output)
    {
        fprintf(stderr, "invalid output file\n");
    }

    fprintf(output, "% .6e\n", time);
    fclose(output);

//
//    for (long myi =0;myi < length/2;myi++) {
//        printf("%d ",vector[myi]);
//    }

    if(debug) {
        print_v(vector, length);
    }

    return 0;
}

//
// Created by Paul Hosek on 18.02.23.
//
//
// Created by Paul Hosek on 21.02.23.
//
