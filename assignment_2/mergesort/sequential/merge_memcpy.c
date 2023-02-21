//#include <time.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <getopt.h>
//#include <stdio.h>
//#include <ctype.h>
//#include <omp.h>
//#include <string.h> /* for memcpy
//
///* Ordering of the vector */
//typedef enum Ordering {ASCENDING, DESCENDING, RANDOM} Order;
//
//int debug = 0;
//
//void TopDownMerge(int *v, long first, long mid, long last, int *cur_v);
//void TopDownSplitMerge(int *cur_v, long first, long last, int*v);
//void msort(int *v, long l);
//
//
///* Sort vector v of l elements using mergesort */
//void msort(int *v, long l){
//    int * cur_v = (int*) malloc(l*sizeof(int));
//    memcpy(cur_v ,v, l * sizeof(int));
//
//    TopDownSplitMerge(cur_v,0,l,v);
//    TopDownMerge(cur_v,0, (int) l/2,l,v);
//}
//
//void TopDownSplitMerge(int *cur_v,  long first, long last,int *v){
//    if (last - first <= 1){
//        return;
//    }
//    long mid = (last + first) / 2;
//    // recursively sort both runs from halves and merge
//    TopDownSplitMerge(v, first,  mid, v);
//    TopDownSplitMerge(v, mid,    last, v);
//    TopDownMerge(cur_v, first, mid, last, v);
//
//}
//
//void TopDownMerge(int *cur_v, long first, long mid, long last, int *v) {
//    long i = first;
//    long j = mid;
//
//    for (long k = first; k < last; k++) {
//        if (i < mid && (j >= last || v[i] <= v[j])) {
//            cur_v[k] = v[i];
//            i++;
//        }
//        else {
//            cur_v[k] = v[j];
//            j++;
//        }
//    }
//
//}
//
//void print_v(int *v, long l) {
//    printf("\n");
//    for(long i = 0; i < l; i++) {
//        if(i != 0 && (i % 10 == 0)) {
//            printf("\n");
//        }
//        printf("%d ", v[i]);
//    }
//    printf("\n");
//}
//
//int main(int argc, char **argv) {
//
//    int c;
//    int seed = 42;
//    long length = 1e4;
//    int num_threads = 1;
//    Order order = ASCENDING;
//    int *vector;
//
//    struct timespec before, after;
//
//    /* Read command-line options. */
//    while((c = getopt(argc, argv, "adrgp:l:s:")) != -1) {
//        switch(c) {
//            case 'a':
//                order = ASCENDING;
//                break;
//            case 'd':
//                order = DESCENDING;
//                break;
//            case 'r':
//                order = RANDOM;
//                break;
//            case 'l':
//                length = atol(optarg);
//                break;
//            case 'g':
//                debug = 1;
//                break;
//            case 's':
//                seed = atoi(optarg);
//                break;
//            case 'p':
//                num_threads = atoi(optarg);
//                break;
//            case '?':
//                if(optopt == 'l' || optopt == 's') {
//                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
//                }
//                else if(isprint(optopt)) {
//                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
//                }
//                else {
//                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
//                }
//                return -1;
//            default:
//                return -1;
//        }
//    }
//
//    /* Seed such that we can always reproduce the same random vector */
//    srand(seed);
//
//    /* Allocate vector. */
//    vector = (int*)malloc(length*sizeof(int));
//    if(vector == NULL) {
//        fprintf(stderr, "Malloc failed...\n");
//        return -1;
//    }
//
//    /* Fill vector. */
//    switch(order){
//        case ASCENDING:
//            for(long i = 0; i < length; i++) {
//                vector[i] = (int)i;
//            }
//            break;
//        case DESCENDING:
//            for(long i = 0; i < length; i++) {
//                vector[i] = (int)(length - i);
//            }
//            break;
//        case RANDOM:
//            for(long i = 0; i < length; i++) {
//                vector[i] = rand();
//            }
//            break;
//    }
//
//    if(debug) {
//        print_v(vector, length);
//    }
//
//    clock_gettime(CLOCK_MONOTONIC, &before);
//
//    /* Sort */
//    msort(vector, length);
//
//    clock_gettime(CLOCK_MONOTONIC, &after);
//    double time = (double)(after.tv_sec - before.tv_sec) +
//                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;
//
//    printf("Mergesort took: % .6e seconds \n", time);
////
////    for (long myi =0;myi < length/2;myi++) {
////        printf("%d ",vector[myi]);
////    }
//
//    if(debug) {
//        print_v(vector, length);
//    }
//
//    return 0;
//}
//
////
//// Created by Paul Hosek on 18.02.23.
////