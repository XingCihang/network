// ringbuf.c
#include "ringbuf.h"
#include <stdlib.h>
#include <string.h>

RingBuffer* ringbuf_create(int size) {
    RingBuffer *rb = malloc(sizeof(RingBuffer));
    rb->entries = malloc(sizeof(NetLog) * size);
    rb->size = size;
    rb->head = rb->tail = 0;
    pthread_mutex_init(&rb->lock, NULL);
    return rb;
}

void ringbuf_destroy(RingBuffer *rb) {
    free(rb->entries);
    pthread_mutex_destroy(&rb->lock);
    free(rb);
}

int ringbuf_push(RingBuffer *rb, NetLog *log) {
    pthread_mutex_lock(&rb->lock);
    int next_tail = (rb->tail + 1) % rb->size;
    if (next_tail == rb->head) {
        pthread_mutex_unlock(&rb->lock);
        return -1; // full
    }
    rb->entries[rb->tail] = *log;
    rb->tail = next_tail;
    pthread_mutex_unlock(&rb->lock);
    return 0;
}

int ringbuf_pop(RingBuffer *rb, NetLog *log) {
    pthread_mutex_lock(&rb->lock);
    if (rb->head == rb->tail) {
        pthread_mutex_unlock(&rb->lock);
        return -1; // empty
    }
    *log = rb->entries[rb->head];
    rb->head = (rb->head + 1) % rb->size;
    pthread_mutex_unlock(&rb->lock);
    return 0;
}