// Server side implementation of UDP. This is the receiver.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>

#define PORT	 8080
#define MAXLINE 1024

int main() {
	int sockfd1,sockfd2;
	int i;
	char buffer[MAXLINE];
	struct sockaddr_in servaddr, cliaddr;
	struct pollfd poll_set[2];
	int poll_ret = 0;
	
	// Creating socket file descriptor
	if ( (sockfd1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket1 creation failed");
		exit(EXIT_FAILURE);
	}
	// Creating socket file descriptor
	if ( (sockfd2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket2 creation failed");
		exit(EXIT_FAILURE);
	}
	int enable = 1;
	if (setsockopt(sockfd1, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed on sockfd1");
	}
	enable = 1;
	if (setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed on sockfd2");
	}
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);
	
	// Bind the socket with the server address
	if ( bind(sockfd1, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0 )
	{
		perror("bind failed for sockfd1");
		exit(EXIT_FAILURE);
	}
	// Bind the socket with the server address
	if ( bind(sockfd2, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0 )
	{
		perror("bind failed for sockfd2");
		exit(EXIT_FAILURE);
	}
	
	
	int len, n;

	len = sizeof(cliaddr); //len is value/resuslt

	for (i = 0; i < 2; ++i) {
		poll_set[i].fd = -1;
		poll_set[i].events = POLLIN;
		poll_set[i].revents = 0;
	}
	poll_set[0].fd = sockfd1;
	poll_set[1].fd = sockfd2;

	int timeout_ms = 1000;
	for (;;) {
		poll_ret = poll(poll_set, 2, timeout_ms);
		if (poll_ret > 0) {
			for (int n = 0; n < 2; n++) {
				printf("%d:fd=%d\n", n, poll_set[n].fd);
				printf("%d:events=%d\n", n, poll_set[n].events);
				printf("%d:revents=%d\n", n, poll_set[n].revents);
				if (poll_set[n].revents != 0) {
					int tmp = recvfrom(poll_set[n].fd, (char *)buffer, MAXLINE,
								MSG_WAITALL, ( struct sockaddr *) &cliaddr,
								&len);
					buffer[tmp] = '\0'; // FIXME: buffer overflow risk
					uint32_t host1 = ntohl(cliaddr.sin_addr.s_addr);
					printf("From Server: recvfrom socket offset %d, IP address %d.%d.%d.%d, Port %d,  Client : %s\n",
							n,
							host1 >> 24 & 0xff,
							host1 >> 16 & 0xff,
							host1 >> 8 & 0xff,
							host1 & 0xff,
							cliaddr.sin_port,
							buffer);
				}
			}
		}
	}

	return 0;
}


