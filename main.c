

/**************************************************************/
/* This program uses the Select function to control sockets   */
/**************************************************************/
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/ipc.h>
#include <sys/msg.h>

#include <sys/socket.h> 
#include <sys/wait.h> 
#include <pthread.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <time.h>
#include "lz4.h"
#include "lz4.c"




/* My Code */
#define BUFFER_SIZE 1024
#define COMPRESS_SIZE 1024
#define ACCELERATION 1
#define FILENAME "book.txt"
int desc[10] = {0};
int curr = 0;
void closeDescriptors()
{
	int i;
	for (i = 0; i < 10; i++)
	{
		if (desc[i])
		{
			printf("Closing %d\n", desc[i]);
			close(desc[i]);
		}
	}
	return;
}


void signalHandler(int sigNo)
{
	
	if (sigNo != SIGINT)
		return;
	closeDescriptors();
	exit(0);
}

int main(int argc, char* argv[])
{
	/* Variables Declarations */
	int 					sockfd;
	struct 	sockaddr_in 	their_addr; /* connector's address information */
	struct 	sockaddr_in 	our_addr; /* connector's address information */
	char buffer[BUFFER_SIZE] = {0};
	char compBuffer[BUFFER_SIZE] = {0};
	FILE* textBook;
	/* init */
	  
	if (signal(SIGINT,  signalHandler) == SIG_ERR)
	{
		printf("\ncan't catch SIGINT\n");
	}
  
	if (argc < 4)
	{
		printf("too few arguments\n");
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	desc[curr++] = sockfd;
	{
		int reuse = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) 
		{
			perror("Setting SO_REUSEADDR error\n\r");
			closeDescriptors();
			exit(1);
		}
	}
	{
		int port;
		int ttl = 4; /* max = 255 */
		sscanf(argv[2], "%d", &port);
		printf("Port %d\n", port);
		
		their_addr.sin_family = AF_INET;
		their_addr.sin_port = htons(port);
		their_addr.sin_addr.s_addr = inet_addr(argv[1]);
		our_addr.sin_family = AF_INET;
		our_addr.sin_port = port;
		our_addr.sin_addr.s_addr = inet_addr(argv[1]);;
				
		if (setsockopt(sockfd,  IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
		{
			perror("Adding multicast group error");
			closeDescriptors();
			
			exit(1);
		} 

	}
	textBook = fopen(FILENAME, "r");
	desc[curr++] = fileno(textBook);
	int cnt = 5;
	while(cnt--)
	{
		if (feof(textBook))
		{
			rewind(textBook);
		}
		size_t bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE, textBook);
		//size_t bytesCompressed = LZ4_compress_fast(buffer, compBuffer, bytesRead, COMPRESS_SIZE, ACCELERATION);
		/*
		if (bytesRead == 0 && bytesCompressed > COMPRESS_SIZE)
		{
			printf("compression error\n");
			closeDescriptors();
			exit(1);
		}
		printf("Orig size: %d -> %d\n", bytesRead,bytesRead);
		* */
		if (sendto(sockfd, buffer, bytesRead, 0, (struct sockaddr*)&their_addr, sizeof(their_addr)) == -1) 
		{
				perror("Writing datagram message error");
				closeDescriptors();
				exit(1);
		}
	}
	
	
	
	
	
	return 0;
}
