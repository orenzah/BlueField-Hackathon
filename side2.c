

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
#include "lz4.h"
#include "lz4.c"




/* My Code */
#define BUFFER_SIZE 1024
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
	char buffer[BUFFER_SIZE] = {0};
	char decompBuffer[BUFFER_SIZE] = {0};
	FILE* textBook;
	/* init */
	  
	if (signal(SIGINT,  signalHandler) == SIG_ERR)
	{
		printf("\ncan't catch SIGINT\n");
	}
  
	if (argc < 3)
	{
		printf("too few arguments\n");
		exit(1);
	}
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 1)
	{
		perror("socket\n\r");
		exit(1);
	}
	printf("Socket fd: %d\n", sockfd);
	desc[curr++] = sockfd;
	{
		int reuse = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) 
		{
			perror("Setting SO_REUSEADDR error");
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
		their_addr.sin_addr.s_addr = INADDR_ANY;
		
		

		
		if(bind(sockfd, (struct sockaddr*)&their_addr, sizeof(their_addr))) 
		{
			perror("Binding datagram socket error");
			closeDescriptors();
			exit(1);
		} 
		
	}
	 
	
	printf("start receiving\n");
	while(1)
	{
		size_t bytesRead;			
		size_t s_addr = sizeof(their_addr);
		
		if ((bytesRead = recvfrom(sockfd, buffer, BUFFER_SIZE ,0, (struct sockaddr*)&their_addr, &s_addr)) == -1) 
		{
				perror("Reading datagram message error");
				closeDescriptors();
				exit(1);
		}
		printf("packet arrived\n");
		bytesRead = LZ4_decompress_fast(buffer, decompBuffer, BUFFER_SIZE);
		fwrite(decompBuffer, sizeof(char), bytesRead, stdout);
	}
	return 0;
}
