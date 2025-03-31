// io_uring + 多线程 + Event Loop 示例 (Echo Server)
// Linux 系统下编译: gcc -o iouring_server iouring_server.c -luring -lpthread

#include <liburing.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8888
#define BACKLOG 512
#define BUF_SIZE 2048
#define THREAD_COUNT 4

int server_fd; // 主监听 socket

// 每个线程的工作参数
typedef struct {
    struct io_uring ring;
    int thread_id;
} WorkerArgs;

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 简单处理连接的函数: 接收 -> 发送回去
typedef struct {
    int client_fd;
    char *buf;
} TaskContext;

void submit_recv(struct io_uring *ring, int client_fd) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    TaskContext *ctx = malloc(sizeof(TaskContext));
    ctx->client_fd = client_fd;
    ctx->buf = malloc(BUF_SIZE);

    io_uring_prep_recv(sqe, client_fd, ctx->buf, BUF_SIZE, 0);
    io_uring_sqe_set_data(sqe, ctx);
    io_uring_submit(ring);
}

void* event_loop(void *arg) {
    WorkerArgs *wargs = (WorkerArgs*)arg;
    struct io_uring *ring = &wargs->ring;
    printf("[Thread %d] Event loop started\n", wargs->thread_id);

    while (1) {
        struct io_uring_cqe *cqe;
        int ret = io_uring_wait_cqe(ring, &cqe);
        if (ret < 0) continue;

        TaskContext *ctx = (TaskContext*)io_uring_cqe_get_data(cqe);
        int res = cqe->res;
        int client_fd = ctx->client_fd;

        if (res <= 0) {
            close(client_fd);
            free(ctx->buf);
            free(ctx);
        } else {
            struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
            io_uring_prep_send(sqe, client_fd, ctx->buf, res, 0);
            io_uring_sqe_set_data(sqe, ctx); // 复用上下文
            io_uring_submit(ring);
        }

        io_uring_cqe_seen(ring, cqe);
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_COUNT];
    WorkerArgs args[THREAD_COUNT];

    // 创建监听 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(server_fd);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, BACKLOG);

    // 启动工作线程，每线程一个 io_uring 环境
    for (int i = 0; i < THREAD_COUNT; i++) {
        io_uring_queue_init(64, &args[i].ring, 0);
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, event_loop, &args[i]);
    }

    // 主线程负责接受连接，并分发给子线程
    int round_robin = 0;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd < 0) continue;
        set_nonblocking(client_fd);

        int target = round_robin++ % THREAD_COUNT;
        submit_recv(&args[target].ring, client_fd);
    }

    return 0;
}
