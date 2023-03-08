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

pthread_cond_t nr_active_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t nr_active_mutex = PTHREAD_MUTEX_INITIALIZER;
int nr_active = 0;

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
void * comp_thread(void *c_arg);

void send_bb(bounded_buffer *buffer, int num);
void destroy_bb(bounded_buffer* buffer);
bounded_buffer* create_bb(int capacity);
thread_node *create_next_node(thread_node *cur_node);


// helper functions -------------------------------

// push element to tail position in bb
//void push_bb(bounded_buffer *buffer, int num){
//    pthread_mutex_lock(&buffer->lock);
//    // wait if full
//    while (buffer->tail == buffer->capacity){
//        pthread_cond_wait(&buffer->not_full, &buffer->lock);
//
//    }
//    // write to buffer and update parameters
//    buffer->buffer[buffer->tail] = num;
//    buffer->tail++;
//
////    print_bb(*buffer);
//    pthread_cond_signal(&buffer->not_empty);
//    pthread_mutex_unlock(&buffer->lock);
//}
//
//int pop_bb(bounded_buffer *buffer){
//    pthread_mutex_lock(&buffer->lock);
//    // wait if full
//    while (!(buffer->tail)){
//        pthread_cond_wait(&buffer->not_empty, &buffer->lock);
//
//    }
//    // write to buffer and update parameters
//
//    int num = buffer->buffer[buffer->tail-1];
//    //    printf("The tail is: %d, num: %d\n",buffer->tail-1, num);
////    print_bb(*buffer);
//    buffer->buffer[buffer->tail-1] = 0; // not needed but may make debugging easier later
//
//    buffer->tail--; // decrease buffer by 1
//
////    print_bb(*buffer);
//    pthread_cond_signal(&buffer->not_full);
//    pthread_mutex_unlock(&buffer->lock);
//    return num;
//}

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
//    printf("%lu pushed %d  | ",(unsigned long)pthread_self(), num);
//    print_bb(*buffer);

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
    buffer->buffer[buffer->head] = 0; // probs don't need this but for debugging if print_bb
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->count--;
//    printf("%lu popped %d  | ",(unsigned long)pthread_self(), num);
//    print_bb(*buffer);

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

void destroy_node_safe(thread_node *cur_node, int head_node){
//    printf("(attempt) %lu attempts to destroy its node\n", (long unsigned)pthread_self());
    pthread_mutex_lock(&out_finished_mutex);
    while (!out_finished_bool){
        pthread_cond_wait(&out_finished_cond,&out_finished_mutex);
    }
    if (!head_node){
        destroy_bb(cur_node->in_buffer); // FIXME these cause the segfault/ mallloc crah
    }
    free(cur_node);
    pthread_mutex_unlock(&out_finished_mutex);
//    printf("(destoyed) %lu has destroyed its node\n", (long unsigned)pthread_self());
}

// create downstream node, link in-and out-buffers
thread_node *create_next_node(thread_node *cur_node){
    thread_node *out_node = (thread_node *) malloc(sizeof(thread_node));

    out_node->in_buffer = cur_node->out_buffer; // map buffer of prev to next
    out_node->out_buffer = create_bb(BUFFER_SIZE);
    out_node->next = NULL;
    out_node->stored = -2; // untouched, -1 is end flag
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

void *add_nr_active(void*args){
    pthread_mutex_lock(&nr_active_mutex);
    nr_active++;
    pthread_mutex_unlock(&nr_active_mutex);
    return NULL;
}

void *remove_nr_active(void*args){
    pthread_mutex_lock(&nr_active_mutex);
    nr_active--;
    if (nr_active == 0){
        pthread_cond_signal(&nr_active_cond);
    }
    pthread_mutex_unlock(&nr_active_mutex);
    return NULL;
}


// FIXME: reminder: get segfault if do: head_node.next->thread_id, bc next is NULL pointer



// ------------------------------------------------
int main(){
//    printf("Master thread is %lu\n", (unsigned long)pthread_self());
    int length = 8; // hard-coded for now
    arr_thread_size = length;
    arr_thread = (pthread_t*)malloc(arr_thread_size*sizeof(pthread_t));
    memset(arr_thread, 0, arr_thread_size*sizeof(pthread_t));

    // create generator node
    pthread_t thread_id;
    bounded_buffer *out_buffer = create_bb(BUFFER_SIZE);
    thread_node head_node = {thread_id,NULL,out_buffer,-1,NULL}; // FIXME: problem is we are not assigning any memory for in buffer of  first node
    thread_args args = {length, &head_node};
    pthread_create(&head_node.thread_id, NULL, &gen_thread, &args);


    // know out is done
    pthread_mutex_lock(&out_finished_mutex);
    while (!out_finished_bool){
        pthread_cond_wait(&out_finished_cond,&out_finished_mutex);
    }
    pthread_mutex_unlock(&out_finished_mutex);

    // wait for all other to free memory
    pthread_mutex_lock(&nr_active_mutex);
    while (nr_active){
        pthread_cond_wait(&nr_active_cond,&nr_active_mutex);
//        printf("waiting ");
    }


    for(int loop = 0; loop < arr_thread_size; loop++)
        printf("Tid: %lu \n", (unsigned long)arr_thread[loop]); // testing

    for (int i=0; i<arr_thread_size;i++){
        pthread_t cur_thread = arr_thread[i];

        if (!(long unsigned)cur_thread){
            // to account for the array being longer than there are threads assigned, but should not happen ideally
            // because we know exactly how many threads we need to sort any number sequence
            break;
        }
        pthread_join(arr_thread[i],NULL);
    }
    pthread_mutex_unlock(&nr_active_mutex);

    free(arr_thread);
    return 0;
}

void* gen_thread(void *g_arg){
    add_id_global(NULL);
    add_nr_active(NULL);
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
        int value = rand() % (200 + 1 - 50) + 50; // range 50-200
        push_bb(out_buffer,value);

    }

    // send 2x END signal
    push_bb(out_buffer,END_SIGNAL);
    push_bb(out_buffer,END_SIGNAL);

//    printf("gen done: nr active %d\n",nr_active);
//    destroy_node_safe(cur_node, 1); // FIXME -> this line only leads to problems, but not in other functions only here
//    printf("gen node in buffer %p", cur_node->in_buffer);

    remove_nr_active(NULL);
    return NULL;
}

void * comp_thread(void *c_arg){
    add_id_global(NULL);
    add_nr_active(NULL);
    thread_args *cur_args = (thread_args*)c_arg;
    thread_node *cur_node = cur_args->Node;
    bounded_buffer *in_buffer = cur_node->in_buffer;
    bounded_buffer *out_buffer = cur_node->out_buffer;
    int no_downstream = 1;
    int stored = cur_node->stored;
    // TODO: pseudo code of behaviour to implement
    //  1. wait for input as long as not END
    //      a. if store empty -> store number
    //      b. else
    //          I: if node.next == NULL -> create downstream node
    //          II. if store < new: -> send store away and save new in store
    //              -> else (store >new): -> send new number away
    //  2. if END
    //      a. if no downstream node -> create downstream OUTPUT node
    //      b.      else:
    //                   I. send END && stored in that order
    //                  II. while (new != END): send away immediately without storing
    //  3. send END
    //  done

    // 1. wait for input as long as not END
    int num = pop_bb(in_buffer);
    while (num != END_SIGNAL){
        // a. store number if empty

        if (stored == -2){
//            printf("init store replace %d with %d\n ",stored, num);
            stored = num;
//            num = pop_bb(in_buffer);
//            continue;
        } else {
//            printf("else num is %d\n", num);
            // b. I create downstream comp_node if not exist
            if (no_downstream){
//            if (cur_node->next == NULL){
                thread_node *ds_node = create_next_node(cur_node);
                thread_args *ds_args = create_next_args(cur_args,ds_node);
                // FIXME: Testing here with outnode first
                pthread_create(&ds_node->thread_id, NULL, &comp_thread, ds_args);
//                printf("comp %lu created comp thread\n", (long unsigned)pthread_self());
                no_downstream = 0;
            }
            // b. II comparison
            if (stored < num){
//                printf("replace stored: %d< %d\n", stored,num);
                push_bb(out_buffer,stored);
                stored = num;
            } else {
//                printf("comp %lu pushed %d to comp\n",(long unsigned)pthread_self(), num);
                push_bb(out_buffer,num);
            }
        }

        num = pop_bb(in_buffer);
//        printf("%lu 's num is %d \n", (long unsigned)pthread_self(), num);
    }

    // 2.a if no downstream but end, create output thread
    if (no_downstream){
//    if (cur_node->next == NULL){ // FIXME: this should only happen if there is no outnode
//        printf("comp %lu created out thread\n", (long unsigned)pthread_self());
        thread_node *ds_node = create_next_node(cur_node);
        thread_args *ds_args = create_next_args(cur_args,ds_node);
        pthread_create(&ds_node->thread_id, NULL, &out_thread, ds_args);
        no_downstream = 0;
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

//    printf("This should be 2nd END: %d\n", num);

//    printf("comp %lu about to destroy node\n",(long unsigned)pthread_self());
    destroy_node_safe(cur_node,0);
//    printf("comp %lu node destroyed\n",(long unsigned)pthread_self());
    remove_nr_active(NULL);
//    printf("comp %lu reduced: nr active %d\n",(long unsigned)pthread_self(),nr_active);
    return NULL;
}

void* out_thread(void *o_arg){
    add_nr_active(NULL);
    add_id_global(NULL);
    thread_args *cur_args = (thread_args*)o_arg;
    thread_node *cur_node = cur_args->Node;
//    printf("outthread %lu bb\n",(long unsigned)pthread_self());


    bounded_buffer * cur_in_bb = cur_node->in_buffer;
    int cur_num = pop_bb(cur_in_bb);
    while(cur_num != END_SIGNAL){
        printf("Num1 is: %d \n",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }
//    printf("END is: %d \n",cur_num);
    // skip first END
    cur_num = pop_bb(cur_in_bb);
//    printf("between is: %d \n",cur_num);
    while(cur_num != END_SIGNAL){
        printf("Num2 is: %d \n",cur_num);
        cur_num = pop_bb(cur_in_bb);
    }

    // signal other nodes to destroy
    out_finished_bool = 1;
    pthread_cond_broadcast(&out_finished_cond);
    destroy_bb(cur_node->out_buffer); // only outnode to destroy the outbuffer, bc no downstream node
//    printf("outbuffer outhread destroyed \n");
    destroy_node_safe(cur_node,0);
    // signal join that thread is done
//    printf("outnode destroyed: nr active %d\n",nr_active);
    remove_nr_active(NULL);
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

