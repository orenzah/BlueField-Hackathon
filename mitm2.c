

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
#include <net/if.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#include <time.h>

#include <netinet/udp.h>	//Provides declarations for udp header
#include <netinet/ip.h>	//Provides declarations for ip header

struct udp_header
{
	/*
	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t  zeroes;
	uint8_t  proto;
	uint16_t udplength;
	* */
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t length;
	uint16_t chksum;
	
} typedef udp_header;
/*
	Generic checksum calculation function
*/

unsigned short csum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}
unsigned checksum(void *buffer, size_t len, unsigned int seed)
{
      unsigned char *buf = (unsigned char *)buffer;
      size_t i;

      for (i = 0; i < len; ++i)
            seed += (unsigned int)(*buf++);
      return seed;
}
/* My Code */
#define BUFFER_SIZE 2048
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
	int						forwfd;
	struct 	sockaddr_in 	their_addr; /* connector's address information */
	
	char buffer[BUFFER_SIZE] = {0};
	char compBuffer[BUFFER_SIZE] = {0};
	FILE* textBook;
	int interface_number;
	struct ifreq ifrIncoming = {0};
	struct ifreq ifrOutgoing = {0};
	struct iphdr *iph;// = (struct iphdr *) datagram;
	struct udphdr *udph;
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
	if ((sockfd = socket(AF_PACKET, SOCK_RAW,IPPROTO_UDP)) < 1)
	{
		perror("socket\n\r");
		exit(1);
	}
	if ((forwfd = socket(AF_PACKET, SOCK_RAW,IPPROTO_UDP)) < 1)
	{
		perror("socket\n\r");
		exit(1);
	}
	printf("Incoming Socket fd: %d\n", sockfd);
	printf("Outgoing Socket fd: %d\n", forwfd);
	desc[curr++] = sockfd;
	strncpy(ifrIncoming.ifr_name, argv[3], sizeof(ifrIncoming.ifr_name));
	strncpy(ifrOutgoing.ifr_name, argv[4], sizeof(ifrOutgoing.ifr_name));
	
	//fprintf(stderr,ifr.ifr_name, sizeof(ifr.ifr_name), opt);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifrIncoming) < 0)
	{
		perror("ioctl(): error incoming");
		closeDescriptors();
		exit(1);		
	}
	if (ioctl(forwfd, SIOCGIFINDEX, &ifrOutgoing) < 0)
	{
		perror("ioctl(): error outgoing");
		closeDescriptors();
		exit(1);		
	}
	// Switch to PROMISC mode
    struct packet_mreq sock_params;
    memset(&sock_params, 0, sizeof(sock_params));
    sock_params.mr_type = PACKET_MR_PROMISC;
    sock_params.mr_ifindex = ifrIncoming.ifr_ifindex;
    int set_promisc = setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (void *)&sock_params, sizeof(sock_params));
    if (set_promisc == -1) 
    {
        printf("Can't enable promisc mode\n");
        return -1;
    }
    struct sockaddr_ll bind_Incoming;
    struct sockaddr_ll bind_Outgoing;
    
    memset(&bind_Incoming, 0, sizeof(bind_Incoming));
    memset(&bind_Outgoing, 0, sizeof(bind_Outgoing));

    bind_Outgoing.sll_family = bind_Incoming.sll_family = AF_PACKET;
    bind_Outgoing.sll_protocol = bind_Incoming.sll_protocol = htons(ETH_P_ALL);
    bind_Incoming.sll_ifindex = ifrIncoming.ifr_ifindex;
    bind_Outgoing.sll_ifindex = ifrOutgoing.ifr_ifindex;
    
    int bind_result = bind(sockfd, (struct sockaddr *)&bind_Incoming, sizeof(bind_Incoming));
    if (bind_result == -1) 
    {
        printf("Can't bind to AF_PACKET incoming socket\n");
        return -1;
    }
    bind_result = bind(forwfd, (struct sockaddr *)&bind_Outgoing, sizeof(bind_Outgoing));
    if (bind_result == -1) 
    {
        printf("Can't bind to AF_PACKET outgoing socket\n");
        return -1;
    }

	{
		int port;
		int ttl = 4; /* max = 255 */
		sscanf(argv[2], "%d", &port);
		printf("Port %d\n", port);
		their_addr.sin_family = AF_INET;
		their_addr.sin_port = htons(port);
		their_addr.sin_addr.s_addr = inet_addr(argv[1]);
		

			
	}
	 
	
	int cnt = 13;
	struct ethhdr *eh;
	udp_header* udpHeader;
	
	eh = (struct ethhdr*) buffer;
	

	while(1)
	{
		size_t bytesRead = 0;			
		size_t bytesSent = 0;			
		size_t bytesComp = 0;	
		size_t s_addr = sizeof(their_addr);
		size_t headersSize		= 42;

		if ((bytesRead = recv(sockfd, buffer, BUFFER_SIZE ,0)) == -1) 
		{
				perror("Reading datagram message error");
				closeDescriptors();
				exit(1);
		}

		
		
		bytesComp = LZ4_compress_default(buffer + headersSize, headersSize+compBuffer, BUFFER_SIZE/2, BUFFER_SIZE/2);
		
		{
			double  ratio = bytesComp / (double)(BUFFER_SIZE/2);		    
			printf("Ratio: %5.2lf\%\n", (1-ratio)*100);
		}
		memcpy(compBuffer, buffer, headersSize);
		iph = (struct iphdr *)(compBuffer + 14);
		udph = (struct udphdr*)(compBuffer + 14 + 20);
		udph->check = 0;
		iph->check  = 0;
		udph->len = htons(8 + bytesComp);
		iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + bytesComp);
		size_t packet_len = bytesComp + 20 + 8;
		packet_len += (bytesComp + 20 + 8)%2;
		iph->check = csum((uint16_t*)(compBuffer + 14), 20);
		printf("total length: %d\n", packet_len);
		if ((bytesSent = send(forwfd, compBuffer, bytesRead + 1024-bytesComp ,0)) == -1) 
		{
				perror("Reading datagram message error");
				closeDescriptors();
				exit(1);
		}
		bzero(compBuffer,BUFFER_SIZE);
		
		//fwrite(buffer + headersSize,sizeof(char), bytesRead-udpHeaderSize - headersSize, stdout);
	}
	return 0;
}

