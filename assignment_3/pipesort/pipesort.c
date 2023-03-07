#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include "pipesort.h"
// TODO:general stuff
//  1. go over and check if all bb destroyed
//  2. check for memory leaks
//  3.


#define END_SIGNAL -1
#define BUFFER_SIZE 10 // Should probs j be 1
pthread_cond_t out_finished_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t out_finished_mutex = PTHREAD_MUTEX_INITIALIZER;
int out_finished_bool = 0;

pthread_mutex_t arr_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t *arr_thread;
int arr_thread_size;
int arr_thread_idx;

void* test_func(void *c_arg);
void* HelloWorld_simple(void *out_args);
void* HelloWorld_node(void *c_arg) ;

void* gen_thread(void *t_arg);
void send_bb(bounded_buffer *buffer, int num);
void destroy_bb(bounded_buffer* buffer);
bounded_buffer* create_bb(int capacity);
thread_node *create_next_node(thread_node *cur_node);


// helper functions -------------------------------
// send data to bounded_buffer
void send_bb(bounded_buffer *buffer, int num){
    pthread_mutex_lock(&buffer->lock);
    // wait if full
    while (buffer->size == buffer->capacity){
        pthread_cond_wait(&buffer->not_full, &buffer->lock);
    }
    // write to buffer and update parameters
    buffer->buffer[buffer->tail] = num;
    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->size++;
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->lock);
}
// initialise bb
bounded_buffer* create_bb(int capacity) {
    // Allocate memory for the bounded buffer and buffer array
    bounded_buffer* buffer = (bounded_buffer*) malloc(sizeof(bounded_buffer));
    buffer->buffer = (int*) malloc(capacity * sizeof(int));

    // Initialise
    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->head = 0;
    buffer->tail = 0;
    pthread_mutex_init(&buffer->lock, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);

    return buffer;
}
void destroy_bb(bounded_buffer* buffer) {
    // Destroy mutex and condition variables
    pthread_mutex_destroy(&buffer->lock);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);

    // Free memory used by buffer array and buffer struct
    free(buffer->buffer);
    free(buffer);
}
thread_node *create_next_node(thread_node *cur_node){
    thread_node *out_node = (thread_node *) malloc(sizeof(thread_node));

    out_node->in_buffer = cur_node->out_buffer; // map buffer of prev to next
    out_node->out_buffer = create_bb(BUFFER_SIZE);
    out_node->next = NULL;
    return out_node;
}
thread_args *create_next_args(thread_args *cur_args, thread_node *out_node){
    thread_args *out_args = (thread_args *) malloc(sizeof(thread_args));
    // Avoid race condition
    memcpy(out_args, cur_args, sizeof(thread_args));
    out_args->Node = out_node;
    return out_args;
}
// add thread id to global array for cleanup
void *add_id_global(void*args){
    pthread_mutex_lock(&arr_thread_mutex);
    pthread_t thread_id = pthread_self();
    arr_thread[arr_thread_idx++] = thread_id;
    pthread_mutex_unlock(&arr_thread_mutex);
    return NULL;
}





// FIXME: reminder: get segfault if do: head_node.next->thread_id, bc next is NULL pointer



// ------------------------------------------------
int main(){ // only for testing rn
    printf("Master thread is %lu\n", (unsigned long)pthread_self());
    int data[] = {4, 2, 1, 3};
    int length = sizeof(data) / sizeof(data[0]);
    arr_thread_size = length;
    arr_thread = (pthread_t*)malloc(arr_thread_size*sizeof(pthread_t));


    // create head node
    pthread_t thread_id;
    bounded_buffer *out_buffer = create_bb(BUFFER_SIZE);
    thread_node head_node = {thread_id,NULL,out_buffer,NULL};
    thread_args args = {data, length, &head_node};
    printf("The pointer is: %p\n", head_node.next);
    printf("test1\n");

//    pthread_create(&head_node.thread_id, NULL, &gen_thread, &args);
    pthread_create(&head_node.thread_id, NULL, &test_func, &args);


    // join all if done
    pthread_mutex_lock(&out_finished_mutex);
    while (!out_finished_bool){
        pthread_cond_wait(&out_finished_cond,&out_finished_mutex);
    }
    pthread_mutex_unlock(&out_finished_mutex);

    for(int loop = 0; loop < arr_thread_size; loop++)
        printf("%lu \n", (unsigned long)arr_thread[loop]);

    for (int i=0; i<arr_thread_size;i++){
        pthread_join(arr_thread[i],NULL);
    }

    free(arr_thread);
    return 0;
}

// TODO actual main function below

//int main() {
//
//    printf("Master thread is %lu\n", (unsigned long)pthread_self());
//    int data[] = {4, 2, 1, 3};
//    int length = sizeof(data) / sizeof(data[0]);
//    arr_thread_size = length;
//    arr_thread_id = (long unsigned int*)malloc(arr_thread_size*sizeof(long unsigned int));
//
//
//    // create head node
//    pthread_t thread_id;
//    bounded_buffer *out_buffer = create_bb(BUFFER_SIZE);
//    thread_node head_node = {thread_id,NULL,out_buffer,NULL,0};
//    thread_args args = {data, length, &head_node};
//    printf("The pointer is: %p\n", head_node.next);
//    printf("test1\n");
//
////    pthread_create(&head_node.thread_id, NULL, &gen_thread, &args);
//    pthread_create(&head_node.thread_id, NULL, &test_func, &args);
//
//
//    // join all if done
//    pthread_mutex_lock(&out_finished_mutex);
//    while (!out_finished_bool){
//        pthread_cond_wait(&out_finished_cond,&out_finished_mutex);
//    }
//    pthread_mutex_unlock(&out_finished_mutex);
//    for (int i=0; i<arr_thread_size;i++){
//        pthread_join(arr_thread_id[i],NULL);
//    }
//
//    free(arr_thread_id);
//    return 0;
//}


void* gen_thread(void *c_arg){
    add_id_global(NULL);
    thread_args *cur_args = (thread_args*)c_arg;
    thread_node *cur_node = cur_args->Node;

    // If there is no downstream thread, create an output node/thread
    if (cur_node->next == NULL) {
        thread_node *out_node = create_next_node(cur_node);
        thread_args *out_args = create_next_args(cur_args,out_node);
        pthread_create(&out_node->thread_id, NULL, &HelloWorld_node, out_args);
    }

    bounded_buffer *out_buffer = cur_node->out_buffer;
    for (int i=0; i<cur_args->length; i++){
        int value = rand();
        send_bb(out_buffer,value);
    }

    // send 2x END signal
    send_bb(out_buffer,END_SIGNAL);
    send_bb(out_buffer,END_SIGNAL);
    return NULL;
}


//void* comparator_thread(void* args) {
//    thread_args * comp_args = (thread_args*)args;
//    thread_node * cur_node = comp_args->Node;
//    bounded_buffer* in_buffer = cur_node->in_buffer;
//    bounded_buffer* out_buffer = cur_node->out_buffer;
//    int stored_value = END_SIGNAL;
//    int received_value;
//
//    while (1) {
//
//        // get input from in_buffer
//        pthread_mutex_lock(&in_buffer->lock);
//        while (in_buffer->size == 0) { // pthread_cond cannot be evaluated by while
//            pthread_cond_wait(&in_buffer->not_empty, &in_buffer->lock); // unlock lock and wait until signal
//        }
//        received_value = in_buffer->buffer[in_buffer->head];
//        in_buffer->head = (in_buffer->head + 1) % in_buffer->capacity;
//        in_buffer->size--;
//        // wake upstream thread
//        pthread_cond_signal(&in_buffer->not_full);
//        pthread_mutex_unlock(&in_buffer->lock);
//
//        if (received_value == END_SIGNAL) {
//            // TODO: If no downstream, create output thread here
//            if (!comparator_thread.next) {
//
//            }
//            // If get END and downstream exists:
//            // 1. forward END
//            // 2. forward stored
//            // 3. forward everything up to and including second END
//            // 4. terminate thread
//
//            // 1. send received_value/ END to output buffer
//            pthread_mutex_lock(&out_buffer->lock);
//            while (out_buffer->size == out_buffer->capacity) {
//                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
//            }
//            out_buffer->buffer[out_buffer->tail] = received_value;
//            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
//            out_buffer->size++;
//            pthread_cond_signal(&out_buffer->not_empty);
//            pthread_mutex_unlock(&out_buffer->lock);
//
//            // 2. send stored_value to output buffer
//            pthread_mutex_lock(&out_buffer->lock);
//            while (out_buffer->size == out_buffer->capacity) {
//                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
//            }
//            out_buffer->buffer[out_buffer->tail] = stored_value;
//            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
//            out_buffer->size++;
//            pthread_cond_signal(&out_buffer->not_empty);
//            pthread_mutex_unlock(&out_buffer->lock);
//
//            // 3. forward everything up to and including second END
//            do  {
//                // get input from in_buffer
//                pthread_mutex_lock(&in_buffer->lock);
//                while (in_buffer->size == 0) { // pthread_cond cannot be evaluated by while
//                    pthread_cond_wait(&in_buffer->not_empty, &in_buffer->lock);
//                }
//                received_value = in_buffer->buffer[in_buffer->head];
//                in_buffer->head = (in_buffer->head + 1) % in_buffer->capacity;
//                in_buffer->size--;
//                // wake upstream thread
//                pthread_cond_signal(&in_buffer->not_full);
//                pthread_mutex_unlock(&in_buffer->lock);
//                // send received_value to output buffer
//                pthread_mutex_lock(&out_buffer->lock);
//                while (out_buffer->size == out_buffer->capacity) {
//                    pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
//                }
//                out_buffer->buffer[out_buffer->tail] = received_value;
//                out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
//                out_buffer->size++;
//                pthread_cond_signal(&out_buffer->not_empty);
//                pthread_mutex_unlock(&out_buffer->lock);
//                // 3. Get next received value
//            } while (received_value != END_SIGNAL);
//
//            // 4. terminate thread
//            pthread_exit(NULL);
//        }
//
//        // compare received_value with stored_value
//        if (received_value > stored_value) {
//            // send stored_value to output buffer and update stored_value
//            pthread_mutex_lock(&out_buffer->lock);
//            while (out_buffer->size == out_buffer->capacity) {
//                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
//            }
//            out_buffer->buffer[out_buffer->tail] = stored_value;
//            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
//            out_buffer->size++;
//            pthread_cond_signal(&out_buffer->not_empty);
//            pthread_mutex_unlock(&out_buffer->lock);
//            stored_value = received_value;
//        } else {
//            // send received_value to output buffer
//            pthread_mutex_lock(&out_buffer->lock);
//            while (out_buffer->size == out_buffer->capacity) {
//                pthread_cond_wait(&out_buffer->not_full, &out_buffer->lock);
//            }
//            out_buffer->buffer[out_buffer->tail] = received_value;
//            out_buffer->tail = (out_buffer->tail + 1) % out_buffer->capacity;
//            out_buffer->size++;
//            pthread_cond_signal(&out_buffer->not_empty);
//            pthread_mutex_unlock(&out_buffer->lock);
//        }
//    }
//}

// ---------------------------------------------------------------------------------------------------
void* test_func(void *c_arg){
    add_id_global(NULL);
    thread_args *cur_args = (thread_args*)c_arg;
    thread_node *cur_node = cur_args->Node;
    printf("test2\n");
    pthread_t thread_id;

    pthread_create(&thread_id, NULL, &HelloWorld_simple, cur_args);

//    add_id_global(NULL);
//    out_finished_bool = 1;
//    pthread_cond_signal(&out_finished_cond);

    return NULL;
}

void* HelloWorld_simple(void *out_args) {
    add_id_global(NULL);
    pthread_t thread_id = pthread_self();
    printf("HW id is %lu\n", (unsigned long)thread_id);
    // output function should be calling this
    out_finished_bool = 1;
    pthread_cond_signal(&out_finished_cond);
    return out_args;
}


void* HelloWorld_node(void *out_args) {
    add_id_global(NULL);
    thread_args *t_args = (thread_args*)out_args;
    printf("test5\n");
    if (t_args != NULL) {
        printf("Thread id is %lu\n", (unsigned long)t_args->Node->thread_id);
    } else {
        printf("Error: t_args->Node is null\n");
    }


    // output function should be calling this
    out_finished_bool = 1;
    pthread_cond_signal(&out_finished_cond);
    return out_args;
}




//
//
//// gen RAND + write to buffer
//for (int i=0; i<length; i++){
//int value = rand();
//send_bb(out_buffer,value);
//}
//// send 2x END signal
//send_bb(out_buffer,END_SIGNAL);
//send_bb(out_buffer,END_SIGNAL);