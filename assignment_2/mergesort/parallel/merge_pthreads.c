#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>

#define NUM_THREADS 16

struct merge_data{
   long  first;
   long  last;
   int *v;
};

/* Ordering of the vector */
typedef enum Ordering {ASCENDING, DESCENDING, RANDOM} Order;

int debug = 0;


void *TopDownSplitMerge(void * data);
void msort(int *v, long l);

int getIndexOfNextZero(int * bitmask, int length)
{
    int index = -1;

    for (int i=0; i < length; i++)
    {
        if (bitmask[i] == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

void *TopDownSplitMerge(void * data) {
    struct merge_data * merge_parameters = (struct merge_data *)data;
    static pthread_t p_threads[NUM_THREADS];
    static int threads_in_use[NUM_THREADS] = { 0 };
    static pthread_mutex_t threads_in_use_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_attr_t attr;
    int local_threads[2];
    int index;
    
    pthread_attr_init(&attr); pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    
    long first = merge_parameters->first;
    long last = merge_parameters->last;
    long mid = (first + last) / 2;
    int * v = merge_parameters->v;

    struct merge_data child1_merge_parameters;
    struct merge_data child2_merge_parameters;

    const int size = last - first;
    
    if (size <= 1) {
        return NULL;
    }

    child1_merge_parameters.first = first;
    child1_merge_parameters.last = mid;
    child1_merge_parameters.v = v;

    child2_merge_parameters.first = mid;
    child2_merge_parameters.last = last;
    child2_merge_parameters.v = v;

    local_threads[0] = -1;
    local_threads[1] = -1;

    pthread_mutex_lock(&threads_in_use_lock);
    if ((index=getIndexOfNextZero(threads_in_use,NUM_THREADS)) != -1)
    {
        local_threads[0] = index;
        threads_in_use[index] = 1;
        pthread_mutex_unlock(&threads_in_use_lock);
        pthread_create(&p_threads[index], &attr, TopDownSplitMerge, (void *)&child1_merge_parameters);
    }
    else
    {
        pthread_mutex_unlock(&threads_in_use_lock);
        TopDownSplitMerge((void *)&child1_merge_parameters);
    }

    pthread_mutex_lock(&threads_in_use_lock);
    if ((index=getIndexOfNextZero(threads_in_use,NUM_THREADS)) != -1)
    {
        local_threads[1] = index;
        threads_in_use[index] = 1;
        pthread_mutex_unlock(&threads_in_use_lock);
        pthread_create(&p_threads[index], &attr, TopDownSplitMerge, (void *)&child2_merge_parameters);
    }
    else
    {
        pthread_mutex_unlock(&threads_in_use_lock);
        TopDownSplitMerge((void *)&child2_merge_parameters);
    }

    // Join threads
    for (int i=0; i < 2; i++)
    {
        if (local_threads[i] != -1){
            pthread_join(p_threads[local_threads[i]], NULL);
            pthread_mutex_lock(&threads_in_use_lock);
            threads_in_use[local_threads[i]] = 0;
            pthread_mutex_unlock(&threads_in_use_lock);
        }
    }

    long i = first;
    long j = mid;
    for (long k = first; k < last; k++) {
        if (i < mid && (j >= last || v[i] <= v[j])) {
            v[k] = v[i];
            i++;
        } else {
            v[k] = v[j];
            j++;
        }
    }
    return NULL;
}


void msort(int *v, long l) {
    struct merge_data merge_parameters;

    merge_parameters.first = 0;
    merge_parameters.last = l;
    merge_parameters.v = v;

    TopDownSplitMerge((void *) &merge_parameters);
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

    struct timespec before, after;

    /* Read command-line options. */
    while((c = getopt(argc, argv, "adrgp:l:s:")) != -1) {
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
    // test if successful sorting
    for (long i =0; i<length; i++){
        printf("%d ", vector[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;

    printf("Mergesort took: % .6e seconds \n", time);

    if(debug) {
        print_v(vector, length);
    }

    return 0;
}