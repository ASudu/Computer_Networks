// Client-Server for echo server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    struct sockaddr_in client;
    int c_sock;
    int clen = sizeof(client);
    char r_buffer[BUFFLEN], s_buffer[BUFFLEN];
    char* server_ip = "172.18.76.160";

    /* CREATE UDP Socket*/
    if((c_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        error("Error in creating client socket!....Exiting\n");
    else
        printf("Client socket successfully created\n");

    /* ESTABLISH connection*/
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(PORTNO);
    client.sin_addr.s_addr = inet_addr(server_ip);

    /* SEND and RECEIVE data*/
    while(1)
    {
        memset(s_buffer, '\0', sizeof(s_buffer));
        printf("Enter message: ");
        gets(s_buffer);

        // Send message
        if(sendto(c_sock, s_buffer, BUFFLEN, 0, (struct sockaddr*) &client, sizeof(client)) == -1)
            error("Error in sending data!....Exiting\n");
        
        // Get reply and print it
        memset(r_buffer, '\0', sizeof(r_buffer));
        if(recvfrom(c_sock, r_buffer, sizeof(r_buffer), 0, (struct sockaddr*) &client, &clen) == -1)
            error("Error in receiving data!....Exiting\n");
        else
            puts(r_buffer);        
    }

    /* CLOSE connection*/
    close(c_sock);

    
    return 0;
}