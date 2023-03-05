#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include "pipesort.h"
#include <string.h>

// TODO:general stuff
//  1. go over and check if all bb destroyed
//  2. check for memory leaks
//  3.

// Assume we are sorting integers > 1!


#define END_SIGNAL -1
#define BUFFER_SIZE 10 // Should probs j be 1
void* HelloWorld(void *c_arg) ;
void* gen_thread(void *t_arg);

// Thread management
void add_active_thread(thread_node *node);
void remove_from_active_thread(thread_node *node);
void join_all_threads();
// helper functions
void send_bb(bounded_buffer *buffer, int num);
void destroy_bb(bounded_buffer* buffer);
bounded_buffer* create_bb(int capacity);
thread_node *create_next_node(thread_node *cur_node);

//void* comparator_thread(void* args);

// singly-Linked list of active threads
thread_node *active_threads = NULL;
pthread_mutex_t active_threads_mutex = PTHREAD_MUTEX_INITIALIZER;

// thread management functions -------------------------------
// Add thread-node to list of active nodes
void add_active_thread(thread_node *node){
    pthread_mutex_lock(&active_threads_mutex);
    node->next = active_threads;
    active_threads = node;
    pthread_mutex_unlock(&active_threads_mutex);
}
// Remove thread-node from list of active nodes
void remove_from_active_thread(thread_node *node){
    pthread_mutex_lock(&active_threads_mutex);
    if(active_threads == node){
        active_threads = node->next;
    } else {
        thread_node *prev = active_threads;
        while (prev->next != node){
            prev = prev->next;
        }
        prev->next = node->next;
    }
    pthread_mutex_unlock(&active_threads_mutex);
}
// Join all thread-nodes
// Note: cannot do this within each function,bc else may get garbage data
void join_all_threads(){
    pthread_mutex_lock(&active_threads_mutex);
    thread_node *node = active_threads;
    while (node != NULL){
        pthread_join(node->thread_id, NULL);
        node = node->next;
        destroy_bb(node->in_buffer);
        destroy_bb(node->out_buffer);
    }
    pthread_mutex_unlock(&active_threads_mutex);
}
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



// ------------------------------------------------
int main() {
    bounded_buffer *in_buffer = create_bb(BUFFER_SIZE);
    bounded_buffer *out_buffer = create_bb(BUFFER_SIZE);

    int data[] = {4, 2, 1, 3};
    int length = sizeof(data) / sizeof(data[0]);
    pthread_t thread_id;
    thread_node head_node = {thread_id,NULL,NULL,NULL};


    thread_args args = {data, length, &head_node};

    add_active_thread(&head_node);
    pthread_create(&head_node.thread_id, NULL, &gen_thread, &args);

    // Wait for the thread to finish
    printf("Head thread id is %lu\n", (unsigned long)head_node.thread_id);
//    pthread_join(head_node.thread_id, NULL);
    join_all_threads();
    printf("join done\n");
    // Clean up


    return 0;
}


void* gen_thread(void *c_arg){
    thread_args *cur_args = (thread_args*)c_arg;
    thread_node *cur_node = cur_args->Node;
    // If there is no downstream thread, create an output node/thread
    printf("Thread 1 created\n");
    if (cur_node->next == NULL) {

        thread_node *out_node =  create_next_node(cur_node);
        thread_args *out_args = (thread_args*)malloc(sizeof(thread_args));
        // Avoid race condition
        memcpy(out_args, cur_args, sizeof(thread_args));
        out_args->Node = out_node;
        printf("second thread created\n");
        add_active_thread(out_node);

        pthread_create(&out_node->thread_id, NULL, &HelloWorld, out_args);
        printf("HW handed to thread.\n");
    }
//    bounded_buffer *out_buffer = cur_node->out_buffer;


    return NULL;
}

void* HelloWorld(void *out_args) {
    printf("HW-call\n");
    thread_args *t_args = (thread_args*)out_args;
    printf("t_args is %p\n", (void*)t_args);
    if (t_args != NULL) {
        printf("Thread id is %lu\n", (unsigned long)t_args->Node->thread_id);
    } else {
        printf("Error: t_args->Node is null\n");
    }
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