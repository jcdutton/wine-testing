// Server side implementation of UDP client-server model
// Compile with: i686-w64-mingw32-gcc -o udpserver.exe win-udpserver.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include<winsock2.h>
//#include <Ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <poll.h>

#define PORT	 8080
#define MAXLINE 1024

// Driver code
#define POLLIN 0x0001
#define POLLPRI 0x0002 /* not used */
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010 /* not used */
#define POLLNVAL 0x0020 /* not used */

struct pollfd {
int fd;
int events; /* in param: what to poll for */
int revents; /* out param: what events occured */
};

int poll (struct pollfd *p, int num, int timeout);

int poll (struct pollfd *p, int num, int timeout)
{
	struct timeval tv;
	fd_set read, write, except;
	int i, n, ret;

	FD_ZERO (&read);
	FD_ZERO (&write);
	FD_ZERO (&except);

	n = -1;
	for (i = 0; i < num; i++)
	{
		if (p[i].fd < 0)
			continue;
		if (p[i].events & POLLIN)
			FD_SET (p[i].fd, &read);
		if (p[i].events & POLLOUT)
			FD_SET (p[i].fd, &write);
		if (p[i].events & POLLERR)
			FD_SET (p[i].fd, &except);
		if (p[i].fd > n)
		n = p[i].fd;
	}

	if (n == -1)
		return (0);

	if (timeout < 0)
		ret = select (n+1, &read, &write, &except, NULL);
	else
	{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = 1000 * (timeout % 1000);
		ret = select (n+1, &read, &write, &except, &tv);
	}

	for (i = 0; ret >= 0 && i < num; i++)
	{
		p[i].revents = 0;
		if (FD_ISSET (p[i].fd, &read))
			p[i].revents |= POLLIN;
		if (FD_ISSET (p[i].fd, &write))
			p[i].revents |= POLLOUT;
		if (FD_ISSET (p[i].fd, &except))
			p[i].revents |= POLLERR;
	}
	return (ret);
}

int main() {
	int sockfd1,sockfd2;
	int i;
	char buffer[MAXLINE];
	struct sockaddr_in servaddr, cliaddr;
	struct pollfd poll_set[2];
	int poll_ret = 0;

	WSADATA wsaData;

	SOCKET ListenSocket;

	int iResult = 0;

	//---------------------------------------
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"Error at WSAStartup()\n");
		return 1;
	}	
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
	char enable = 1;
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
					printf("From Server: recvfrom socket offset %d, IP address %d.%d.%d.%d, Port %d,  Client : %s\n",
							n,
							cliaddr.sin_addr.S_un.S_un_b.s_b1,
							cliaddr.sin_addr.S_un.S_un_b.s_b2,
							cliaddr.sin_addr.S_un.S_un_b.s_b3,
							cliaddr.sin_addr.S_un.S_un_b.s_b4,
							cliaddr.sin_port,
							buffer);
					printf("Note: wine on Linux returns offset 1,  The same .exe run in windows returns offset 0\n");
					printf("      This causes problems with some windows networking programs running on wine.\n");
				}
			}
		}
	}

	WSACleanup();
	return 0;
}


