// vm_writer.c
#include "vm_writer.h"
#include <liburing.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>

void* vm_writer_loop(void *arg) {
    RingBuffer *rb = (RingBuffer *)arg;
    struct io_uring ring;
    io_uring_queue_init(32, &ring, 0);

    while (1) {
        NetLog log;
        if (ringbuf_pop(rb, &log) == 0) {
            char line[1024];
            snprintf(line, sizeof(line),
                "net_bytes{src_ip=\"%s\",dst_ip=\"%s\",proto=\"%s\",type=\"%s\"} %d %lu\n",
                log.src_ip, log.dst_ip, log.protocol, log.device_type, log.bytes, log.ts_ms);

            int sock = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in server;
            server.sin_family = AF_INET;
            server.sin_port = htons(8428);
            inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
            connect(sock, (struct sockaddr*)&server, sizeof(server));

            char request[2048];
            int body_len = strlen(line);
            int req_len = snprintf(request, sizeof(request),
                "POST /api/v1/import HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %d\r\n\r\n%s",
                body_len, line);

            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            io_uring_prep_write(sqe, sock, request, req_len, -1);
            io_uring_submit(&ring);

            struct io_uring_cqe *cqe;
            io_uring_wait_cqe(&ring, &cqe);
            io_uring_cqe_seen(&ring, cqe);
            close(sock);
        } else {
            usleep(1000);
        }
    }
    io_uring_queue_exit(&ring);
    return NULL;
}