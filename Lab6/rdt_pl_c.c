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
    struct sockaddr_in client;
    int c_sock, i, clen=sizeof(client);
    char buf[BUFLEN], message[BUFLEN];
    char* server_ip = "127.0.1.1";
    DATA_PKT send_pkt,rcv_ack;

    // For dropping data packets
    srand(time(NULL));
    int r = rand()%100;       // 10% chance of packet drop
    int drop = (r != 0)?1:0;
    
    if ( (c_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        error("[-] Error creating client socket! Exiting...\n");
    
    memset((char *) &client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = inet_addr(server_ip);
    
    int state = 0;
    while(1)
    {
        switch(state)
        {
            case 0:     // wait for sending packet with seq. no. 0
                printf("Enter message 0: ");
                fgets(send_pkt.data,sizeof(send_pkt),stdin);
                send_pkt.sq_no = 0;
                if  (sendto(c_sock,  &send_pkt,  sizeof(send_pkt),  0,  (struct sockaddr *) &client, clen)==-1)
                   error("[-] Error in sending message with seq #0! Exiting...\n");
                state = 1;
                break;
            
            case 1:     // Waiting for ACK 0
                if (recvfrom(c_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *) &client, &clen) == -1)
                    error("[-] Error in receiving ACK 0! Exiting...\n");
                
                // Do not drop
                if (!drop && rcv_ack.sq_no==0)
                {  
                    printf("[+] Received ack seq. no. %d\n",rcv_ack.sq_no);
                    state = 2;
                    break;
                }
            case 2:     //wait for sending packet with seq. no. 1
                printf("Enter message 1: ");
                fgets(send_pkt.data,sizeof(send_pkt),stdin);
                send_pkt.sq_no = 1;
                if  (sendto(c_sock,  &send_pkt,  sizeof(send_pkt)  ,  0  ,  (struct sockaddr *) &client, clen)==-1)
                    error("[-] Error in sending message with seq #1! Exiting...\n");
                state = 3;
                break;
            case 3:     //waiting for ACK 1
                if(recvfrom(c_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr*) &client, &clen) == -1)
                    error("[-] Error in receiving ACK 1! Exiting...\n");
                
                // Do not drop
                if (!drop && rcv_ack.sq_no==1)
                {
                    printf("[+] Received ack seq. no. %d\n",rcv_ack.sq_no);
                    state = 0;
                    break;
                }
        }
    }

    close(c_sock);    
    return 0;
}