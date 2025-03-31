// dpdk_rx.c
#include "dpdk_rx.h"
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <time.h>
#include <arpa/inet.h>

const char* get_protocol_by_port(uint16_t port) {
    switch (port) {
        case 502: return "modbus";
        case 9100: return "printer";
        default: return "unknown";
    }
}

const char* get_device_type_by_ip(const char* ip) {
    if (strstr(ip, ".100")) return "printer";
    if (strstr(ip, ".50")) return "PLC";
    return "unknown";
}

void* dpdk_rx_loop(void *arg) {
    RingBuffer *rb = (RingBuffer *)arg;
    int argc = 3;
    char *argv[] = {"demo", "-l", "0-1"};
    rte_eal_init(argc, argv);

    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", 8192, 250, 0,
            RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    uint16_t port_id = 0;
    rte_eth_dev_configure(port_id, 1, 1, NULL);
    rte_eth_rx_queue_setup(port_id, 0, 1024, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
    rte_eth_dev_start(port_id);

    struct rte_mbuf *bufs[32];

    while (1) {
        uint16_t nb_rx = rte_eth_rx_burst(port_id, 0, bufs, 32);
        for (int i = 0; i < nb_rx; i++) {
            char *data = rte_pktmbuf_mtod(bufs[i], char *);
            int len = rte_pktmbuf_data_len(bufs[i]);

            NetLog log;
            snprintf(log.src_ip, sizeof(log.src_ip), "192.168.0.1");
            snprintf(log.dst_ip, sizeof(log.dst_ip), "192.168.0.100");
            snprintf(log.protocol, sizeof(log.protocol), "%s", get_protocol_by_port(9100));
            snprintf(log.device_type, sizeof(log.device_type), "%s", get_device_type_by_ip(log.dst_ip));
            log.bytes = len;

            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            log.ts_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

            ringbuf_push(rb, &log);
            rte_pktmbuf_free(bufs[i]);
        }
    }
    return NULL;
}