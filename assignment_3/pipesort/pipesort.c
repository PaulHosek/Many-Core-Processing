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
//  3. clean up nodes correctly
//  4. make sure to only destroy the IN-buffer after ever node cleanup.


#define END_SIGNAL -1
#define BUFFER_SIZE 10

// signaling output thread is finished
pthread_cond_t out_finished_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t out_finished_mutex = PTHREAD_MUTEX_INITIALIZER;
int out_finished_bool = 0;

// global array holding all thread ids
pthread_mutex_t arr_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t *arr_thread;
int arr_thread_size;
int arr_thread_idx;

void* test_func(void *c_arg);
void* HelloWorld_simple(void *out_args);
void* HelloWorld_node(void *c_arg) ;

void* gen_thread(void *t_arg);
void* out_thread(void *o_arg);

void send_bb(bounded_buffer *buffer, int num);
void destroy_bb(bounded_buffer* buffer);
bounded_buffer* create_bb(int capacity);
thread_node *create_next_node(thread_node *cur_node);


// helper functions -------------------------------
// send data to bounded_buffer
void append_bb(bounded_buffer *buffer, int num){
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
int pop_bb(bounded_buffer *buffer){
    pthread_mutex_lock(&buffer->lock);
    // wait if full
    while (buffer->size == 0){
        pthread_cond_wait(&buffer->not_empty, &buffer->lock);
    }
    // write to buffer and update parameters
    int num = buffer->buffer[buffer->tail];
    buffer->buffer[buffer->tail] = 0; // not needed but may make debugging easier later
    buffer->tail = (buffer->tail + buffer->capacity - 1) % buffer->capacity;
    buffer->size--;
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->lock);
    return num;
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
int main(){
    printf("Master thread is %lu\n", (unsigned long)pthread_self());
    int length = 10; // hard-coded for now
    arr_thread_size = length;
    arr_thread = (pthread_t*)malloc(arr_thread_size*sizeof(pthread_t));


    // create generator node
    pthread_t thread_id;
    bounded_buffer *out_buffer = create_bb(BUFFER_SIZE);
    thread_node head_node = {thread_id,NULL,out_buffer,NULL};
    thread_args args = {length, &head_node};1
    pthread_create(&head_node.thread_id, NULL, &gen_thread, &args);


    // join all threads if done
    pthread_mutex_lock(&out_finished_mutex);
    while (!out_finished_bool){
        pthread_cond_wait(&out_finished_cond,&out_finished_mutex);
    }
    pthread_mutex_unlock(&out_finished_mutex);

    for(int loop = 0; loop < arr_thread_size; loop++)
        printf("Tid: %lu \n", (unsigned long)arr_thread[loop]);

    for (int i=0; i<arr_thread_size;i++){
        pthread_join(arr_thread[i],NULL);
    }

    free(arr_thread);
    return 0;
}

void* gen_thread(void *g_arg){
    add_id_global(NULL);
    thread_args *cur_args = (thread_args*)g_arg;
    thread_node *cur_node = cur_args->Node;

    // If there is no downstream thread, create an output node/thread
    if (cur_node->next == NULL) {
        thread_node *out_node = create_next_node(cur_node);
        thread_args *out_args = create_next_args(cur_args,out_node);
        pthread_create(&out_node->thread_id, NULL, &out_thread, out_args);
    }

    bounded_buffer *out_buffer = cur_node->out_buffer;
    for (int i=0; i<cur_args->length; i++){
        int value = rand() % (100 + 1 - 50) + 50; // range 50-100
        append_bb(out_buffer,value);
    }

    // send 2x END signal
    append_bb(out_buffer,END_SIGNAL);
    append_bb(out_buffer,END_SIGNAL);
    return NULL;
}

void* out_thread(void *o_arg){
    add_id_global(NULL);
    thread_args *cur_args = (thread_args*)o_arg;
    thread_node *cur_node = cur_args->Node;


    bounded_buffer * cur_in_bb = cur_node->in_buffer;
    int cur_num = pop_bb(cur_in_bb);
    while(cur_num != END_SIGNAL){
        printf("Num is: %d \n",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }
    // skip first END
    cur_num = pop_bb(cur_in_bb);
    while(cur_num != END_SIGNAL){
        printf("Num is: %d \n",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }


    out_finished_bool = 1;
    pthread_cond_signal(&out_finished_cond);
    return o_arg;
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
    if (t_args != NULL) {
        printf("HS thread id is %lu\n", (unsigned long)t_args->Node->thread_id);
    } else {
        printf("Error: t_args->Node is null\n");
    }


    // output function should be calling this
    out_finished_bool = 1;
    pthread_cond_signal(&out_finished_cond);
    return out_args;
}