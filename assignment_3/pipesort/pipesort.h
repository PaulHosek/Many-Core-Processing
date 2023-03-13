#ifndef MANY_CORE_PROCESSING_PIPESORT_H
#define MANY_CORE_PROCESSING_PIPESORT_H


#include <pthread.h>

//Decouple pipeline stages: bounded buffer implemented as a FIFO queue.
typedef struct {
    int *buffer;        // array to hold the numbers between stages: does all the buffering :)
    int capacity;       // max numbers
    int head;           // index first elem // TODO remove if not needed
    int tail;           // number of elements in stack
    int count;
    // Access synchronisation
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} bounded_buffer;

// Main DS: Pipeline implemented as singly linked list, where each node is a thread with a pointer to the downstream thread
// points to input and output buffers to move data between threads
typedef struct Node {
    pthread_t thread_id;
    bounded_buffer *in_buffer;
    bounded_buffer *out_buffer;// or Node.next.in_buffer
    int stored;
    struct Node *next;
} thread_node;


typedef struct {
    int length;
//    pthread_t upstream_tid;
    thread_node *Node;
} thread_args;



#endif //MANY_CORE_PROCESSING_PIPESORT_H

//what happens if i use pthread_join(