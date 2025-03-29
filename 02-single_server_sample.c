// server_single.c
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(2000);

	if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr))) {
		printf("bind failed: %s\n", strerror(errno));
		return -1;
	}

	listen(sockfd, 10);
	printf("listen finished: %d\n", sockfd);

	struct sockaddr_in  clientaddr;
	socklen_t len = sizeof(clientaddr);

	printf("accept\n");
	int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
	printf("accept finished\n");

	char buffer[1024] = {0};
	int count = recv(clientfd, buffer, 1024, 0);
	printf("RECV: %s\n", buffer);

	count = send(clientfd, buffer, count, 0);
	printf("SEND: %d\n", count);

	close(clientfd);
	close(sockfd);

	return 0;
}
