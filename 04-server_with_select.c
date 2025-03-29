// server_select.c
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

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

	fd_set rfds, rset;
	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);
	int maxfd = sockfd;

	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);

	while (1) {
		rset = rfds;
		int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset)) {
			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
			printf("accept finished: %d\n", clientfd);
			FD_SET(clientfd, &rfds);
			if (clientfd > maxfd) maxfd = clientfd;
		}

		for (int i = sockfd + 1; i <= maxfd; i++) {
			if (FD_ISSET(i, &rset)) {
				char buffer[1024] = {0};
				int count = recv(i, buffer, 1024, 0);
				if (count == 0) {
					printf("client disconnect: %d\n", i);
					close(i);
					FD_CLR(i, &rfds);
					continue;
				}
				printf("RECV: %s\n", buffer);
				count = send(i, buffer, count, 0);
				printf("SEND: %d\n", count);
			}
		}
	}
}
