//#include <time.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <getopt.h>
//#include <stdio.h>
//#include <ctype.h>
//#include <errno.h>
//#include <pthread.h>
//#include "pipesort.h"
//
//#define END_SIGNAL -1
//void* HelloWorld(void *a);
//void* gen_thread(int * data, int length, thread_node * cur_node);
////void* comparator_thread(void* args);
//
//// helper functions
//// send data to bounded_buffer
//void send_bb(bounded_buffer *buffer, int num){
//    pthread_mutex_lock(&buffer->lock);
//    // wait if full
//    while (buffer->size == buffer->capacity){
//        pthread_cond_wait(&buffer->not_full, &buffer->lock);
//    }
//    // write to buffer and update parameters
//    buffer->buffer[buffer->tail] = num;
//    buffer->tail = (buffer->tail + 1) % buffer->capacity;
//    buffer->size++;
//    pthread_cond_signal(&buffer->not_empty);
//    pthread_mutex_unlock(&buffer->lock);
//}
//
//// ------------------------------------------------
////void * init_gen_node(){
////
////}
//
//
//// Assume we are sorting integers
//void* gen_thread(int * data, int length, thread_node * cur_node){ // TODO assumed that threads exist already, need to create them in iteration function
//    // If there is no downstream thread, create an output node/thread
//    if (cur_node->out_buffer == NULL) {
//        thread_node *out_node = (thread_node *) malloc(sizeof(thread_node));
//        cur_node->out_buffer = out_node->in_buffer; // map buffer of prev to next
//        out_node->out_buffer = NULL;
//        out_node->next = NULL;
//        pthread_create(&out_node->thread_id, NULL, &HelloWorld, (void *) out_node); // TODO see if function call correct here
//    }
////
////    bounded_buffer *out_buffer = cur_node->out_buffer;
////
////    // gen RAND + write to buffer
////    for (int i=0; i<length; i++){
////        int value = rand();
////        send_bb(out_buffer,value);
////    }
////    // send 2x END signal
////    send_bb(out_buffer,END_SIGNAL);
////    send_bb(out_buffer,END_SIGNAL);
//
//
//    return NULL;
//}
//
//void* HelloWorld(void *a) {
//    int* tid_p = (int*) a;
//    int tid = *tid_p;
//    printf("Thread id is %d/n", tid);
//    return a;
//}
//
//
//// TODO: here are the old comparator thread functions, may be able to reuse a lot
////void* comparator_thread(void* args) {
////    thread_args * comp_args = (thread_args*)args;
////    bounded_buffer* in_buffer = comp_args->in_buffer;
////    bounded_buffer* out_buffer = comp_args->out_buffer;
////    int stored_value = END_SIGNAL;
////    int received_value;
////
////    while (1) {
////
////        // get input from in_buffer
////        pthread_mutex_lock(&in_buffer->lock);
////        while (in_buffer->size == 0) { // reason: pthread_cond cannot be evaluated by while
////            pthread_cond_wait(&in_buffer->not_empty, &in_buffer->lock);
////        }
////        received_value = in_buffer->buffer[in_buffer->head];
////        in_buffer->head = (in_buffer->head + 1) % in_buffer->capacity;
////        in_buffer->size--;
////        // wake upstream thread
////        pthread_cond_signal(&in_buffer->not_full);
////        pthread_mutex_unlock(&in_buffer->lock);
////
////        if (received_value == END_SIGNAL) {
////            // TODO: If no downstream, create output thread here
////            if (!comparator_thread.next) {
////
////            }
////            // If get END and downstream exists:
////            // 1. forward END
////            // 2. forward stored
////            // 3. forward everything up to and including second END
////            // 4. terminate thread
////
////            // 1. send received_value/ END to output buffer
////            pthread_mutex_lock(&out_buffer->lock);
////            while (out_buffer->size == out_buffer->capacity) {
////                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
////            }
////            out_buffer->buffer[out_buffer->tail] = received_value;
////            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
////            out_buffer->size++;
////            pthread_cond_signal(&out_buffer->not_empty);
////            pthread_mutex_unlock(&out_buffer->lock);
////
////            // 2. send stored_value to output buffer
////            pthread_mutex_lock(&out_buffer->lock);
////            while (out_buffer->size == out_buffer->capacity) {
////                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
////            }
////            out_buffer->buffer[out_buffer->tail] = stored_value;
////            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
////            out_buffer->size++;
////            pthread_cond_signal(&out_buffer->not_empty);
////            pthread_mutex_unlock(&out_buffer->lock);
////
////            // 3. forward everything up to and including second END
////            do  {
////                // get input from in_buffer
////                pthread_mutex_lock(&in_buffer->lock);
////                while (in_buffer->size == 0) { // pthread_cond cannot be evaluated by while
////                    pthread_cond_wait(&in_buffer->not_empty, &in_buffer->lock);
////                }
////                received_value = in_buffer->buffer[in_buffer->head];
////                in_buffer->head = (in_buffer->head + 1) % in_buffer->capacity;
////                in_buffer->size--;
////                // wake upstream thread
////                pthread_cond_signal(&in_buffer->not_full);
////                pthread_mutex_unlock(&in_buffer->lock);
////                // send received_value to output buffer
////                pthread_mutex_lock(&out_buffer->lock);
////                while (out_buffer->size == out_buffer->capacity) {
////                    pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
////                }
////                out_buffer->buffer[out_buffer->tail] = received_value;
////                out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
////                out_buffer->size++;
////                pthread_cond_signal(&out_buffer->not_empty);
////                pthread_mutex_unlock(&out_buffer->lock);
////                // 3. Get next received value
////            } while (received_value != END_SIGNAL);
////
////            // 4. terminate thread
////            pthread_exit(NULL);
////        }
////
////        // compare received_value with stored_value
////        if (received_value > stored_value) {
////            // send stored_value to output buffer and update stored_value
////            pthread_mutex_lock(&out_buffer->lock);
////            while (out_buffer->size == out_buffer->capacity) {
////                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
////            }
////            out_buffer->buffer[out_buffer->tail] = stored_value;
////            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
////            out_buffer->size++;
////            pthread_cond_signal(&out_buffer->not_empty);
////            pthread_mutex_unlock(&out_buffer->lock);
////            stored_value = received_value;
////        } else {
////            // send received_value to output buffer
////            pthread_mutex_lock(&out_buffer->lock);
////            while (out_buffer->size == out_buffer->capacity) {
////                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
////            }
////            out_buffer->buffer[out_buffer->tail] = received_value;
////            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
////            out_buffer->size++;
////            pthread_cond_signal(&out_buffer->not_empty);
////            pthread_mutex_unlock(&out_buffer->lock);
////        }
////    }
////}
//
//
//// --------------------------------------------------------
////void* comparator_thread(void* args) {
////    thread_args * comp_args = (thread_args*)args;
////    bounded_buffer* in_buffer = comp_args->in_buffer;
////    bounded_buffer* out_buffer = comp_args->out_buffer;
////    int stored_value = END_SIGNAL;
////    int received_value;
////
////    while (1) {
////
////        // get input from in_buffer
////        pthread_mutex_lock(&in_buffer->lock);
////        while (in_buffer->size == 0) { // reason: pthread_cond cannot be evaluated by while
////            pthread_cond_wait(&in_buffer->not_empty, &in_buffer->lock);
////        }
////        received_value = in_buffer->buffer[in_buffer->head];
////        in_buffer->head = (in_buffer->head + 1) % in_buffer->capacity;
////        in_buffer->size--;
////        // wake upstream thread
////        pthread_cond_signal(&in_buffer->not_full);
////        pthread_mutex_unlock(&in_buffer->lock);
////
////        if (received_value == END_SIGNAL) {
////            // If get END and downstream exists:
////            // 1. forward END
////            // 2. forward stored
////            // 3. forward everything up to and including second END
////            // 4. terminate thread
////
////            // 1. send END to output buffer
////            pthread_mutex_lock(&out_buffer->lock);
////            while (out_buffer->size == out_buffer->capacity) {
////                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
////            }
////            out_buffer->buffer[out_buffer->tail] = END_SIGNAL;
////            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
////            out_buffer->size++;
////            pthread_cond_signal(&out_buffer->not_empty);
////            pthread_mutex_unlock(&out_buffer->lock);
////
////            if (comp_args->next != NULL) {
////                // 2. send stored_value to downstream buffer
////                pthread_mutex_lock(&comp_args->next->in_buffer->lock);
////                while (comp_args->next->in_buffer->size == comp_args->next->in_buffer->capacity) {
////                    pthread_cond_wait(&comp_args->next->in_buffer->not_full, &comp_args->next->in_buffer->lock);
////                }
////                comp_args->next->in_buffer->buffer[comp_args->next->in_buffer->tail] = stored_value;
////                comp_args->next->in_buffer->tail = (comp_args->next->in_buffer->tail + 1) % comp_args->next->in_buffer->capacity;
////                comp_args->next->in_buffer->size++;
////                pthread_cond_signal(&comp_args->next->in_buffer->not_empty);
////                pthread_mutex_unlock(&comp_args->next->in_buffer->lock);
////
////                // 3. forward everything up to and including second END
////                do  {
////                    // get input from in_buffer
////                    pthread_mutex_lock(&in_buffer->lock);
////                    while (in_buffer->size == 0) { // pthread_cond cannot be evaluated by while
////                        pthread_cond_wait(&in_buffer->not_empty, &in_buffer->lock);
////                    }
////                    received_value = in_buffer->buffer[in_buffer->head];
////                    in_buffer->head = (in_buffer->head + 1) % in_buffer->capacity;
////                    in_buffer->size--;
////                    //
////
////
////
////
////
////// --------------------------------------------------------
////
////
//int main(int argc, char *argv[]){
//    int c;
//    int seed = 42;
//    long length = 1e4;
//
//    struct timespec before;
//    struct timespec  after;
//
//
//
//    /* Read command-line options. */
//    while((c = getopt(argc, argv, "l:s:")) != -1) {
//        switch(c) {
//            case 'l':
//                length = atol(optarg);
//                break;
//            case 's':
//                seed = atoi(optarg);
//                break;
//            case '?':
//                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
//                return -1;
//            default:
//                return -1;
//        }
//    }
//
//    /* Seed such that we can always reproduce the same random vector */
//    srand(seed);
//
//    clock_gettime(CLOCK_MONOTONIC, &before);
//    /* Do your thing here */
//
//
//
//
//
//    /* Do your thing here */
//    clock_gettime(CLOCK_MONOTONIC, &after);
//    double time = (double)(after.tv_sec - before.tv_sec) +
//                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;
//
//    printf("Pipesort took: % .6e seconds \n", time);
//
//}
