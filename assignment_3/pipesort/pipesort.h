#ifndef MANY_CORE_PROCESSING_PIPESORT_H
#define MANY_CORE_PROCESSING_PIPESORT_H


#include <pthread.h>

//Decouple pipeline stages: bounded buffer
typedef struct {
    int *buffer;        // array to hold the numbers between stages: does all the buffering :)
    int capacity;       // max numbers
    int size;           // cur numbers
    int head;           // index first elem
    int tail;           // index first free elem
    // Access synchronisation
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} bounded_buffer; // TODO change naming here to snake case

// Main DS: Pipeline implemented as linked list, where each node is a thread with a pointer to the downstream thread
// points to input and output buffers to move data between threads
typedef struct Node {
    pthread_t thread_id;
    bounded_buffer *in_buffer;
    bounded_buffer *out_buffer;// or Node.next.in_buffer
    struct Node *next;
} thread_node;

// TODO: not sure about this yet, want to avoid passing NULL but also dont want multiple DS
//// Arguments DS.
typedef struct {
    int *data;
    int length;
    thread_node *Node;
} thread_args;
//    bounded_buffer *in_buffer;
//    bounded_buffer *out_buffer;


#endif //MANY_CORE_PROCESSING_PIPESORT_H