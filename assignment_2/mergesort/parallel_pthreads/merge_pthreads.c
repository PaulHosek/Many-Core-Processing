#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>

struct merge_data{
   long  first;
   long  last;
   int *v_source;
   int *v_dest;
   int number_threads;
   pthread_t *p_threads;
   int *threads_in_use;
   long thread_input_size;
};

/* Ordering of the vector */
typedef enum Ordering {ASCENDING, DESCENDING, RANDOM} Order;

int debug = 0;


void *TopDownSplitMerge(void * data);
void msort(int *v, long l, int num_threads, long thread_min_input_size);
void print_v(int *v, long l);

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
    struct merge_data * const merge_parameters = (struct merge_data *)data;
    const long first = merge_parameters->first;
    const long last = merge_parameters->last;

    const int size = last - first;
    
    if (size <= 1) {
        return NULL;
    }

    static pthread_mutex_t threads_in_use_lock = PTHREAD_MUTEX_INITIALIZER;
    static pthread_attr_t attr;
    pthread_attr_init(&attr); pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    int local_threads[2];
    local_threads[0] = -1;
    local_threads[1] = -1;

    int index;
    
    const long mid = (first + last) / 2;
    int * const v_source = merge_parameters->v_source;
    int * const v_dest = merge_parameters->v_dest;
    const int num_threads = merge_parameters->number_threads;
    int * const threads_in_use = merge_parameters->threads_in_use;
    pthread_t * const p_threads = merge_parameters->p_threads;
    const long thread_min_input_size = merge_parameters->thread_input_size;

    struct merge_data child1_merge_parameters = *merge_parameters;
    struct merge_data child2_merge_parameters = *merge_parameters;

    child1_merge_parameters.first = first;
    child1_merge_parameters.last = mid;
    child1_merge_parameters.v_source = v_dest;
    child1_merge_parameters.v_dest = v_source;

    child2_merge_parameters.first = mid;
    child2_merge_parameters.last = last;
    child2_merge_parameters.v_source = v_dest;
    child2_merge_parameters.v_dest = v_source;

    pthread_mutex_lock(&threads_in_use_lock);
    if ((mid - first >= thread_min_input_size) && ((index=getIndexOfNextZero(threads_in_use,num_threads)) != -1))
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
    if ((last - mid >= thread_min_input_size) && ((index=getIndexOfNextZero(threads_in_use,num_threads)) != -1))
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
        if (i < mid && (j >= last || v_source[i] <= v_source[j])) {
            v_dest[k] = v_source[i];
            i++;
        } else {
            v_dest[k] = v_source[j];
            j++;
        }
    }
    return NULL;
}


void msort(int *v, long l, int num_threads, long thread_input_size) {
    struct merge_data merge_parameters;
    int * v_temp = malloc(l * sizeof(int));
    memcpy(v_temp, v, l * sizeof(int));

    pthread_t p_threads[num_threads];
    int threads_in_use[num_threads];
    memset( threads_in_use, 0, num_threads*sizeof(int) );

    merge_parameters.first = 0;
    merge_parameters.last = l;
    merge_parameters.v_source = v_temp;
    merge_parameters.v_dest = v;
    merge_parameters.p_threads = p_threads;
    merge_parameters.number_threads = num_threads;
    merge_parameters.threads_in_use = threads_in_use;
    merge_parameters.thread_input_size = thread_input_size;

    TopDownSplitMerge((void *) &merge_parameters);
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