// RECEIVER
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>       // For rand and timer

#define BUFLEN 512
#define PORT 8882

typedef struct packet1
{
    int sq_no;
}ACK_PKT;

typedef struct packet2
{
    int sq_no;
    char data[BUFLEN];
}DATA_PKT;

void error(char* msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    struct sockaddr_in server, client;
    int s_sock, i, recv_len, slen = sizeof(client);
    DATA_PKT rcv_pkt;
    ACK_PKT  ack_pkt;

    // For dropping ACKs
    srand(time(NULL));
    int r = rand()%100;       // 10% chance of packet drop
    int drop = (r != 0)?1:0;

    // Create a UDP socket
    if ((s_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        error("[-] Error creating server socket! Exiting...\n");

    // Zero out the structure
    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Bind socket to port
    if( bind(s_sock , (struct sockaddr*)&server, sizeof(server) ) == -1)
        error("[-] Error in binding server socket! Exiting...\n");
    
    int state =0;

    while(1)
    {
        switch(state)
        {
            case 0:
            {
                printf("Waiting for packet 0 from sender...\n");
                fflush(stdout);
                // Try to receive some data, this is a blocking call
                if ((recv_len = recvfrom(s_sock, &rcv_pkt, BUFLEN, 0, (struct sockaddr*) &client, &slen)) == -1)
                    error("[-] Error in receiving message with seq #0! Exiting...\n");
                
                // Do not drop
                if (!drop && rcv_pkt.sq_no==0)
                {
                    printf("[+] Packet received with seq. no. %d and Packet content is:\n %s",rcv_pkt.sq_no, rcv_pkt.data);
                    ack_pkt.sq_no = 0;
                    if  (sendto(s_sock,  &ack_pkt,  recv_len,  0,  (struct  sockaddr*)  &client, slen) == -1)
                        error("[-] Error in sending ACK 0! Exiting...\n");
                    state = 1;
                    break;
                }
            }

            case 1:
            {
                printf("Waiting for packet 1 from sender...\n");
                fflush(stdout);
                // Try to receive some data, this is a blocking call
                if ((recv_len = recvfrom(s_sock, &rcv_pkt, BUFLEN, 0, (struct sockaddr*) &client, &slen)) == -1)
                    error("[-] Error in receiving message with seq #1! Exiting...\n");
                
                // Do not drop
                if (!drop && rcv_pkt.sq_no==1)
                {
                    printf("[+] Packet received with seq. no. %d and Packet content is: %s",rcv_pkt.sq_no, rcv_pkt.data);
                    ack_pkt.sq_no = 1;
                    if  (sendto(s_sock,  &ack_pkt,  recv_len,  0,  (struct  sockaddr*)  &client, slen) == -1)
                        error("[-] Error in sending ACK 1! Exiting...\n");
                    state = 0;
                    break;
                }
            }
        }
    }

    close(s_sock);
    return 0;
}