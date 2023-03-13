#ifndef MANY_CORE_PROCESSING_FIFO_QUEUE_H
#define MANY_CORE_PROCESSING_FIFO_QUEUE_H

#include <stdlib.h>
#include <assert.h>

typedef struct {
    size_t head;
    size_t tail;
    size_t size;
    void** data;
} queue_t;

void* queue_read(queue_t *queue) {
    if (queue->tail == queue->head) {
        return NULL;
    }
    void* handle = queue->data[queue->tail];
    queue->data[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->size;
    return handle;
}

int queue_write(queue_t *queue, void* handle) {
    if (((queue->head + 1) % queue->size) == queue->tail) {
        return -1;
    }
    queue->data[queue->head] = handle;
    queue->head = (queue->head + 1) % queue->size;
    return 0;
}
#endif //MANY_CORE_PROCESSING_FIFO_QUEUE_H