// Client-Server for echo server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFLEN 512
#define PORTNO 8888

void error(char* msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    struct sockaddr_in server;
    int s_sock, recv_len;
    int slen = sizeof(server);
    char buffer[BUFFLEN];
    char* server_ip = "172.18.76.160";

    /* CREATE UDP Socket*/
    if((s_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        error("Error in creating server socket!....Exiting\n");
    else
        printf("Server socket created!\n");
    
    /* BIND socket to port*/
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTNO);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(s_sock, (struct sockaddr*) &server, sizeof(server)) == -1)
        error("Error in binding socket!....Exiting\n");
    
    /* LISTEN for data*/
    while(1)
    {
        printf("Waiting for data...");
        fflush(stdout);

        // Receive data
        memset(buffer, '\0', sizeof(buffer));
        if((recv_len = recvfrom(s_sock, buffer, BUFFLEN, 0, (struct sockaddr*) &server, &slen)) == -1)
            error("Error in receiving data!....Exiting\n");

        // Print details of connected peer
        printf("Received packet from %s: %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
        printf("Data: %s\n", buffer);

        // Reply to client with same data
        if(sendto(s_sock, buffer, BUFFLEN, 0, (struct sockaddr*) &server, slen) == -1)
            error("Error in echoing data!....Exiting\n");
        
    }

    /* CLOSE connection*/
    close(s_sock);
    return 0;
}