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
   int * available_threads;
   pthread_mutex_t * available_threads_lock;
};

/* Ordering of the vector */
typedef enum Ordering {ASCENDING, DESCENDING, RANDOM} Order;

int debug = 0;


void *TopDownSplitMerge(void * data);
void msort(int *v, long l);

void *TopDownSplitMerge(void * data) {
    struct merge_data * merge_parameters = (struct merge_data *)data;
    pthread_attr_t attr;
    pthread_t p_thread[2];
    pthread_attr_init(&attr); pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_mutex_t * lock = (pthread_mutex_t *) merge_parameters->available_threads_lock;
    int * available_threads = merge_parameters->available_threads;
    long first = merge_parameters->first;
    long last = merge_parameters->last;
    long mid = (first + last) / 2;
    int * v = merge_parameters->v;

    struct merge_data child1_merge_parameters;
    struct merge_data child2_merge_parameters;
    
    if (last - first <= 1) {
        return NULL;
    }

    child1_merge_parameters.first = first;
    child1_merge_parameters.last = mid;
    child1_merge_parameters.v = v;
    child1_merge_parameters.available_threads = available_threads;
    child1_merge_parameters.available_threads_lock = lock;

    child2_merge_parameters.first = mid;
    child2_merge_parameters.last = last;
    child2_merge_parameters.v = v;
    child2_merge_parameters.available_threads = available_threads;
    child2_merge_parameters.available_threads_lock = lock;

    pthread_mutex_lock(lock);
    if (*available_threads > 1)
    {
        *available_threads--;
        pthread_mutex_unlock(lock);
        pthread_create(&p_thread[0], &attr, TopDownSplitMerge, (void *)&child1_merge_parameters);
    }
    else
    {
        pthread_mutex_unlock(lock);
        TopDownSplitMerge((void *)&child1_merge_parameters);
    }

    pthread_mutex_lock(lock);
    if (*available_threads > 1)
    {
        *available_threads--;
        pthread_mutex_unlock(lock);
        pthread_create(&p_thread[1], &attr, TopDownSplitMerge, (void *)&child1_merge_parameters);
    }
    else
    {
        pthread_mutex_unlock(lock);
        TopDownSplitMerge((void *)&child1_merge_parameters);
    }
    pthread_join(p_thread[0], NULL);
    pthread_join(p_thread[1], NULL);

    pthread_mutex_lock(lock);
    *available_threads +=2;
    pthread_mutex_unlock(lock);

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
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    struct merge_data merge_parameters;
    int available_threads = NUM_THREADS;

    merge_parameters.first = 0;
    merge_parameters.last = l;
    merge_parameters.v = v;
    merge_parameters.available_threads = &available_threads;
    merge_parameters.available_threads_lock = &lock;

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