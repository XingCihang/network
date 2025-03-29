// server_epoll.c
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(2000);

	if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
		printf("bind failed: %s\n", strerror(errno));
		return -1;
	}

	listen(sockfd, 10);
	printf("listen finished: %d\n", sockfd);

	int epfd = epoll_create(1);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);

	while (1) {
		struct epoll_event events[1024];
		int nready = epoll_wait(epfd, events, 1024, -1);

		for (int i = 0; i < nready; i++) {
			int connfd = events[i].data.fd;

			if (connfd == sockfd) {
				int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
				printf("accept finished: %d\n", clientfd);
				ev.events = EPOLLIN;
				ev.data.fd = clientfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
			} else if (events[i].events & EPOLLIN) {
				char buffer[1024] = {0};
				int count = recv(connfd, buffer, 1024, 0);
				if (count == 0) {
					printf("client disconnect: %d\n", connfd);
					close(connfd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
					continue;
				}
				printf("RECV: %s\n", buffer);
				count = send(connfd, buffer, count, 0);
				printf("SEND: %d\n", count);
			}
		}
	}
}
