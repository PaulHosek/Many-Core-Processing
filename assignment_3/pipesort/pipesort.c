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
int buffer_size;

// signaling output thread is finished
pthread_cond_t out_finished_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t out_finished_mutex = PTHREAD_MUTEX_INITIALIZER;
int out_finished_bool = 0;

pthread_cond_t nr_active_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t nr_active_mutex = PTHREAD_MUTEX_INITIALIZER;
int nr_active = 0;

// global array holding all thread ids
pthread_mutex_t arr_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t *arr_thread;
int arr_thread_size;
int arr_thread_idx = 0;

// helper functions
void send_bb(bounded_buffer *buffer, int num);
void destroy_bb(bounded_buffer* buffer);
bounded_buffer* create_bb(int capacity);
thread_node *create_next_node(thread_node *cur_node);

// main program
void* gen_thread(void *t_arg);
void* out_thread(void *o_arg);
void * comp_thread(void *c_arg);
int pipesort_scheduler(int length);

// ----------------- helper functions -----------------
// queue implementation
void push_bb(bounded_buffer *buffer, int num){
    pthread_mutex_lock(&buffer->lock);
    // wait if full
    while (buffer->count == buffer->capacity){
        pthread_cond_wait(&buffer->not_full, &buffer->lock);

    }
    // write to buffer and update parameters
    buffer->buffer[(buffer->head + buffer->count) % buffer->capacity] = num;
    buffer->count++;

    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->lock);
}

int pop_bb(bounded_buffer *buffer){
    pthread_mutex_lock(&buffer->lock);
    // wait if empty
    while (buffer->count == 0){
        pthread_cond_wait(&buffer->not_empty, &buffer->lock);
    }
    // read from buffer and update parameters
    int num = buffer->buffer[buffer->head];
    buffer->buffer[buffer->head] = 0;
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->count--;
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->lock);
    return num;
}

// allocate and initialise bb
bounded_buffer* create_bb(int capacity) {
    // Allocate memory for the bounded buffer and buffer array
    bounded_buffer* buffer = (bounded_buffer*) malloc(sizeof(bounded_buffer)); // 160 bytes
    buffer->buffer = (int*) malloc(capacity * sizeof(int)); // 40 bytes
    // Initialise
    buffer->capacity = capacity;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
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

void destroy_node_safe(thread_node *cur_node, int destroy_inbuffer){
    // wait for out-node to be done
    pthread_mutex_lock(&out_finished_mutex);
    while (!out_finished_bool){
        pthread_cond_wait(&out_finished_cond,&out_finished_mutex);
    }
    pthread_mutex_unlock(&out_finished_mutex);

    if (destroy_inbuffer){
        destroy_bb(cur_node->in_buffer); // FIXME these cause the segfault/ mallloc crah
    }
    free(cur_node);

}

// create downstream node, link in-and out-buffers
thread_node *create_next_node(thread_node *cur_node){
    thread_node *out_node = (thread_node *) malloc(sizeof(thread_node));

    out_node->in_buffer = cur_node->out_buffer; // map buffer of prev to next
    out_node->out_buffer = create_bb(buffer_size);
    out_node->next = NULL;
    out_node->stored = -2; // untouched, -1 is end flag
    return out_node;
}

// copy args of node for initialisation of downstream node
thread_args *create_next_args(thread_args *cur_args, thread_node *out_node){
    thread_args *out_args = (thread_args *) malloc(sizeof(thread_args)); // 16 bytes

    // Avoid race condition
    memcpy(out_args, cur_args, sizeof(thread_args));
    out_args->Node = out_node;
    return out_args;
}

// add thread id to global array for cleanup later
void *add_id_global(void*args){
    pthread_mutex_lock(&arr_thread_mutex);
    pthread_t thread_id = pthread_self();
    arr_thread[arr_thread_idx++] = thread_id;// FIXME: Valgrind detects invalid write here
    pthread_mutex_unlock(&arr_thread_mutex);
    return NULL;
}

// increment nr active threads
void *increment_active(void*args){
    pthread_mutex_lock(&nr_active_mutex);
    nr_active++;
    pthread_mutex_unlock(&nr_active_mutex);
    return NULL;
}

// decrement nr active threads
void *decrement_active(void*args){
    pthread_mutex_lock(&nr_active_mutex);
    nr_active--;
    if (nr_active == 0){
        pthread_cond_signal(&nr_active_cond);
    }
    pthread_mutex_unlock(&nr_active_mutex);
    return NULL;
}

// ------------------- main program -------------------

// start pipeline and manage threads
int pipesort_scheduler(int length){
//    printf("Master thread is %lu\n", (unsigned long)pthread_self());
    arr_thread_size = length+2;
    arr_thread = (pthread_t*)malloc(arr_thread_size*sizeof(pthread_t));
    memset(arr_thread, 0, arr_thread_size*sizeof(pthread_t));

    // create generator node
    pthread_t thread_id;
    bounded_buffer *out_buffer = create_bb(buffer_size);

    thread_node head_node = {thread_id,NULL,out_buffer,-1,NULL}; // FIXME: problem is we are not assigning any memory for in buffer of  first node
    thread_args args = {length, &head_node};
    pthread_create(&head_node.thread_id, NULL, &gen_thread, &args);


    // know out-node is done
    pthread_mutex_lock(&out_finished_mutex);
    while (!out_finished_bool){
        pthread_cond_wait(&out_finished_cond,&out_finished_mutex);
    }
    pthread_mutex_unlock(&out_finished_mutex);

    // wait for all other to free memory
    pthread_mutex_lock(&nr_active_mutex);
    while (nr_active){
        pthread_cond_wait(&nr_active_cond,&nr_active_mutex);
    }

    for (int i=0; i<arr_thread_size;i++){
        pthread_t cur_thread = arr_thread[i];
        pthread_join(arr_thread[i],NULL);
    }
    pthread_mutex_unlock(&nr_active_mutex);

    free(arr_thread);
    return 0;
}

// generator thread: RNG + pass numbers downstream
void* gen_thread(void *g_arg){
    add_id_global(NULL);
    increment_active(NULL);
    thread_args *cur_args = (thread_args*)g_arg;
    thread_node *cur_node = cur_args->Node;

    // If there is no downstream thread, create an output node/thread
    if (cur_node->next == NULL) {
        thread_node *comp_node = create_next_node(cur_node);
        thread_args *comp_args = create_next_args(cur_args,comp_node);

        pthread_create(&comp_node->thread_id, NULL, &comp_thread, comp_args);
    }
    bounded_buffer *out_buffer = cur_node->out_buffer;

    for (int i=0; i<cur_args->length; i++){
        int value = rand(); //% (200 + 1 - 50) + 50; // range 50-200
        push_bb(out_buffer,value);

    }

    // send 2x END signal
    push_bb(out_buffer,END_SIGNAL);
    push_bb(out_buffer,END_SIGNAL);

    decrement_active(NULL);
    return NULL;
}

// comparator thread: store highest, pass lowest, recursively create downstream threads
void * comp_thread(void *c_arg){
    add_id_global(NULL);
    increment_active(NULL);
    thread_args *cur_args = (thread_args*)c_arg;
    thread_node *cur_node = cur_args->Node;
    thread_node *ds_node = create_next_node(cur_node);
    thread_args *ds_args = create_next_args(cur_args,ds_node);
    bounded_buffer *in_buffer = cur_node->in_buffer;
    bounded_buffer *out_buffer = cur_node->out_buffer;
    int no_downstream = 1;
    int stored = cur_node->stored;


    // 1. wait for input as long as not END
    int num = pop_bb(in_buffer);
    while (num != END_SIGNAL){
        // a. store number if empty
        if (stored == -2){
            stored = num;
        } else {
            // b. I create downstream comp_node if not exist
            if (no_downstream){
                pthread_create(&ds_node->thread_id, NULL, &comp_thread, ds_args);
                no_downstream = 0;
            }
            // b. II comparison
            if (stored < num){
                push_bb(out_buffer,stored);
                stored = num;
            } else {
                push_bb(out_buffer,num);
            }
        }

        num = pop_bb(in_buffer);
    }

    // 2.a if no downstream but end, create output thread
    if (no_downstream){
        pthread_create(&ds_node->thread_id, NULL, &out_thread, ds_args);
    }
    // 2.b send END, send stored, keep sending input until second END
    push_bb(out_buffer,num); // send END
    if (stored != -2){
        push_bb(out_buffer,stored); // send stored
    }
    num = pop_bb(in_buffer);
    while (num != END_SIGNAL){
        push_bb(out_buffer, num);
        num = pop_bb(in_buffer);
    }
    push_bb(out_buffer,num); // send 2nd END
    
    // free memory of node, only destroy in-buffer
    decrement_active(NULL);
    free(cur_args);
    destroy_node_safe(cur_node, 1);
    return NULL;
}

// print out numbers
void* out_thread(void *o_arg){
    increment_active(NULL);
    add_id_global(NULL);
    thread_args *cur_args = (thread_args*)o_arg;
    thread_node *cur_node = cur_args->Node;


    bounded_buffer * cur_in_bb = cur_node->in_buffer;
    int cur_num = pop_bb(cur_in_bb);
    while(cur_num != END_SIGNAL){
        printf("%d ",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }
    // skip first END
    cur_num = pop_bb(cur_in_bb);
    while(cur_num != END_SIGNAL){
        printf("%d ",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }
    printf("\n");

    // signal other nodes to self-destroy
    out_finished_bool = 1;
    pthread_cond_broadcast(&out_finished_cond);
    decrement_active(NULL);
    free(cur_args);
    destroy_bb(cur_node->out_buffer);
    destroy_node_safe(cur_node, 1);

    return o_arg;
}

int main(int argc, char *argv[]){
    int c;
    int seed = 42;
    long length = 1e3;
    int print_exec_time = 1;
    buffer_size = 10;

    struct timespec before;
    struct timespec  after;



    /* Read command-line options. */
    while((c = getopt(argc, argv, "l:s:pb:")) != -1) {
        switch(c) {
            case 'l':
                length = atol(optarg);
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'p':
                print_exec_time = 0;
                break;
            case 'b':
                buffer_size = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return -1;
            default:
                return -1;
        }
    }

    /* Seed such that we can always reproduce the same random vector */
    srand(seed);

    clock_gettime(CLOCK_MONOTONIC, &before);
    /* Do your thing here */
    pipesort_scheduler(length);


    /* Do your thing here */
    clock_gettime(CLOCK_MONOTONIC, &after);
    double time = (double)(after.tv_sec - before.tv_sec) +
                  (double)(after.tv_nsec - before.tv_nsec) / 1e9;
    if (print_exec_time){
        printf("Pipesort took: % .6e seconds \n", time);
    }


}

