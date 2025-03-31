// dpdk_vm_demo: main.c
// DPDK + io_uring + VictoriaMetrics (适用于工业互联网流量采集)

#include <stdio.h>
#include <pthread.h>
#include "dpdk_rx.h"
#include "vm_writer.h"
#include "ringbuf.h"

RingBuffer *global_ring = NULL;

int main(int argc, char **argv) {
    global_ring = ringbuf_create(1024);
    if (!global_ring) {
        fprintf(stderr, "Ring buffer allocation failed\n");
        return -1;
    }

    pthread_t rx_thread, vm_thread;
    pthread_create(&rx_thread, NULL, dpdk_rx_loop, global_ring);
    pthread_create(&vm_thread, NULL, vm_writer_loop, global_ring);

    pthread_join(rx_thread, NULL);
    pthread_join(vm_thread, NULL);

    ringbuf_destroy(global_ring);
    return 0;
}