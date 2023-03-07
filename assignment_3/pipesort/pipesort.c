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
void* HelloWorld_node(void *c_arg);
void print_bb(bounded_buffer bb);

void* gen_thread(void *t_arg);
void* out_thread(void *o_arg);

void send_bb(bounded_buffer *buffer, int num);
void destroy_bb(bounded_buffer* buffer);
bounded_buffer* create_bb(int capacity);
thread_node *create_next_node(thread_node *cur_node);


// helper functions -------------------------------

// push element to tail position in bb
//void push_bb(bounded_buffer *buffer, int num){
//    pthread_mutex_lock(&buffer->lock);
//    // wait if full
//    while (buffer->size == buffer->capacity){
//        printf("append\n");
//        pthread_cond_wait(&buffer->not_full, &buffer->lock);
//
//
//    }
//    // write to buffer and update parameters
//    buffer->buffer[buffer->tail] = num;  // FIXME: here tail can be equal to capacity!!
////    if (buffer->size){ // if not equal to
////        buffer->tail++;
////    }
//    buffer->tail = (buffer->tail+1) % buffer->capacity; // to be not always less than capacity
//    buffer->size++;
//
//    print_bb(*buffer);
//    pthread_cond_signal(&buffer->not_empty);
//    pthread_mutex_unlock(&buffer->lock);
//}

void push_bb(bounded_buffer *buffer, int num){
    pthread_mutex_lock(&buffer->lock);
    // wait if full
    while (buffer->tail == buffer->capacity){
        printf("append\n");
        pthread_cond_wait(&buffer->not_full, &buffer->lock);

    }
    // write to buffer and update parameters
    buffer->buffer[buffer->tail] = num;
    buffer->tail++;

    print_bb(*buffer);
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->lock);
}

int pop_bb(bounded_buffer *buffer){
    pthread_mutex_lock(&buffer->lock);
    // wait if full
    while (!(buffer->tail)){
        printf("pop\n");
        pthread_cond_wait(&buffer->not_empty, &buffer->lock);

    }
    // write to buffer and update parameters

    int num = buffer->buffer[buffer->tail-1];

    printf("The tail is: %d, num: %d\n",buffer->tail-1, num);
    print_bb(*buffer);
    buffer->buffer[buffer->tail-1] = 0; // not needed but may make debugging easier later

    buffer->tail--; // decrease buffer by 1

    print_bb(*buffer);
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
    buffer->head = 0;
    buffer->tail = 0;
    pthread_mutex_init(&buffer->lock, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);

    return buffer;
}

// free memory of bb
void destroy_bb(bounded_buffer* buffer) {
    // Destroy mutex and condition variables
    pthread_mutex_destroy(&buffer->lock);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);

    // Free memory used by buffer array and buffer struct
    free(buffer->buffer);
    free(buffer);
}

// create downstream node, link in-and out-buffers
thread_node *create_next_node(thread_node *cur_node){
    thread_node *out_node = (thread_node *) malloc(sizeof(thread_node));

    out_node->in_buffer = cur_node->out_buffer; // map buffer of prev to next
    out_node->out_buffer = create_bb(BUFFER_SIZE);
    out_node->next = NULL;
    return out_node;
}

// copy args of node for initialisation of downstream node
thread_args *create_next_args(thread_args *cur_args, thread_node *out_node){
    thread_args *out_args = (thread_args *) malloc(sizeof(thread_args));
    // Avoid race condition
    memcpy(out_args, cur_args, sizeof(thread_args));
    out_args->Node = out_node;
    return out_args;
}

// add thread id to global array for cleanup later
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
    int length = 11; // hard-coded for now
    arr_thread_size = length;
    arr_thread = (pthread_t*)malloc(arr_thread_size*sizeof(pthread_t));


    // create generator node
    pthread_t thread_id;
    bounded_buffer *out_buffer = create_bb(BUFFER_SIZE);
    thread_node head_node = {thread_id,NULL,out_buffer,NULL};
    thread_args args = {length, &head_node};
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
        push_bb(out_buffer,value);

    }

    // send 2x END signal
    push_bb(out_buffer,END_SIGNAL);
    push_bb(out_buffer,END_SIGNAL);
    printf("gen_done\n");
    return NULL;
}

void* out_thread(void *o_arg){
    add_id_global(NULL);
    thread_args *cur_args = (thread_args*)o_arg;
    thread_node *cur_node = cur_args->Node;


    bounded_buffer * cur_in_bb = cur_node->in_buffer;
    int cur_num = pop_bb(cur_in_bb);
    while(cur_num != END_SIGNAL){
        printf("Num1 is: %d \n",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }
    printf("END is: %d \n",cur_num);
    // skip first END
    cur_num = pop_bb(cur_in_bb);
    printf("between is: %d \n",cur_num);
    while(cur_num != END_SIGNAL){ // FIXME: race condition or sth fishy, escape loop but print num is: -2. also -2 is not a num we are generatign
        printf("Num2 is: %d \n",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }


    out_finished_bool = 1;
    pthread_cond_signal(&out_finished_cond);
    printf("out done\n");
    return o_arg;
}




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
// not thread-safe, use within locked region
void print_bb(bounded_buffer bb){
//    pthread_mutex_lock(&bb.lock);
    for (int i=0; i<bb.capacity;i++){
        printf("%d ",bb.buffer[i]);
    }
    printf("\n");
//    pthread_mutex_unlock(&bb.lock);
}

// pop element from tail-1 in bb
//int pop_bb(bounded_buffer *buffer){
//    pthread_mutex_lock(&buffer->lock);
//    // wait if full
//    while (!buffer->size){
//        printf("pop\n");
//        pthread_cond_wait(&buffer->not_empty, &buffer->lock);
//
//    }
//    // write to buffer and update parameters
////    buffer->tail--;  // FIXME: now buffersize incorrect
//
//    int num = buffer->buffer[buffer->tail];
//
//    printf("The buffersize is: %d, tail: %d, num: %d\n",buffer->size,buffer->tail, num);
//    print_bb(*buffer);
//    buffer->buffer[buffer->tail] = 0; // not needed but may make debugging easier later
//
//    buffer->tail = (buffer->tail + buffer->capacity-1) % buffer->capacity; // decrease buffer by 1
//
//
//    buffer->size--;
//
//    print_bb(*buffer);
//    pthread_cond_signal(&buffer->not_full);
//    pthread_mutex_unlock(&buffer->lock);
//    return num;
//}