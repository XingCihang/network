// ringbuf.h
#ifndef RINGBUF_H
#define RINGBUF_H
#include "common.h"
#include <pthread.h>

typedef struct {
    NetLog *entries;
    int size;
    int head, tail;
    pthread_mutex_t lock;
} RingBuffer;

RingBuffer* ringbuf_create(int size);
void ringbuf_destroy(RingBuffer *rb);
int ringbuf_push(RingBuffer *rb, NetLog *log);
int ringbuf_pop(RingBuffer *rb, NetLog *log);

#endif