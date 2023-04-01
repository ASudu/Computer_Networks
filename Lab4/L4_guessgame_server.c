// Client-server for guessing game
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

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
    int s_sock, recvlen;
    int slen = sizeof(server);
    char r_buffer[BUFFLEN], s_buffer[BUFFLEN];

    /* CREATE UDP Socket*/
    if((s_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        error("Error in creating server socket!...Exiting\n");
    else
        printf("Server socket created!\n");
    
    /* BIND socket to a port*/
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTNO);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket
    if(bind(s_sock, (struct sockaddr*) &server, sizeof(server)) == -1)
        error("Error in binding socket!...Exiting\n");
    
    int random = 1 + rand()%6;
    srand(time(0));

    /* LISTEN for data*/
    while(1)
    {
        char snum[5];
        printf("The number is %d\n", random);
        // RECEIVE data and check what to send
        memset(r_buffer, '\0', BUFFLEN);
        if((recvlen = recvfrom(s_sock, r_buffer, BUFFLEN, 0, (struct sockaddr*) &server, &slen)) == -1 )
            error("Error in receiving data!...Exiting\n");
        else
        {
            printf("Received data from %s: %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
            memset(s_buffer, '\0', BUFFLEN);
            if(atoi(r_buffer) == random)
            {
                strcpy(s_buffer, "Guess is correct!!\n");
                random = 1 + rand()%6;
                srand(time(0));
            }

            else if(atoi(r_buffer) < random)
                strcpy(s_buffer, "Guess is smaller...\n");
            else
                strcpy(s_buffer, "Guess is larger...\n");

        }

        // SEND data
        if(sendto(s_sock, s_buffer, BUFFLEN, 0, (struct sockaddr*) &server, slen) == -1)
            error("Error in sending data!...Exiting\n");        
    }

    close(s_sock);    
    return 0;
}