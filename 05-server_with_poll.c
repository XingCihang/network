// server_poll.c
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

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

	struct pollfd fds[1024] = {0};
	fds[sockfd].fd = sockfd;
	fds[sockfd].events = POLLIN;
	int maxfd = sockfd;

	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);

	while (1) {
		int nready = poll(fds, maxfd + 1, -1);

		if (fds[sockfd].revents & POLLIN) {
			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
			printf("accept finished: %d\n", clientfd);
			fds[clientfd].fd = clientfd;
			fds[clientfd].events = POLLIN;
			if (clientfd > maxfd) maxfd = clientfd;
		}

		for (int i = sockfd + 1; i <= maxfd; i++) {
			if (fds[i].fd == -1) continue;
			if (fds[i].revents & POLLIN) {
				char buffer[1024] = {0};
				int count = recv(i, buffer, 1024, 0);
				if (count == 0) {
					printf("client disconnect: %d\n", i);
					close(i);
					fds[i].fd = -1;
					continue;
				}
				printf("RECV: %s\n", buffer);
				count = send(i, buffer, count, 0);
				printf("SEND: %d\n", count);
			}
		}
	}
}
