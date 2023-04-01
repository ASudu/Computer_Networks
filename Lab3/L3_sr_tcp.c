// Client-server for sending one message to each other
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXPENDING 5
#define BUFFLEN 32

// Define function for error display
void error(char* msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    char buffer[BUFFLEN];

    /* Create TCP Socket*/
    // Note: AF_* and PF_* are used interchangeably
    // One is Address Family (AF) and the other is Protocol Family (PF)
    int s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s_sock < 0)
        error("Error in socket creation!....Exiting\n");
    else
        printf("Server socket creation successful!\n");

    /* BIND the soket to an address*/
    // Create local address structure
    // Note: sockaddr_in is the structure specific to IPv4
    // sockaddr is structure for any protocol
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned!\n");

    // Bind the socket to the address structure
    int temp = bind(s_sock, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(temp < 0)
        error("Error in socket binding!....Exiting\n");
    else
        printf("Server socket binding successful!\n");

    /* LISTEN for possible connections*/
    int temp1 = listen(s_sock, MAXPENDING);
    if(temp1 < 0)
        error("Error in socket listen!....Exiting\n");
    else
        printf("Server socket listening!\n");

    /* ACCEPT a few TCP connections depending on the basndwidth*/
    int client_len = sizeof(client_addr);
    int c_sock = accept(s_sock, (struct sockaddr*) &client_addr, &client_len);
    if(c_sock < 0)
        error("Error in client socket!....Exiting\n");
    else
        printf("Handling client!\n");
    
    /* RECEIVE data*/
    int recvd = recv(c_sock, buffer, BUFFLEN, 0);
    int x;
    // Convert the float in string buffer to float
    // and store in int variable so that we get the integer part
    if(recvd < 0)
        error("Error in receiving data!....Exiting\n");
    else
    {
        x = atof(buffer);
        printf("Received data!\n");
    }   

    /* SEND data*/
    // Clear the buffer
    memset(buffer, 0, BUFFLEN);
    // Since we want the ceil of the number received
    x = x+1;
    // Store as string in the buffer
    sprintf(buffer, "%d", x); 
    int sent = send(c_sock, buffer, BUFFLEN, 0);
    if(sent != BUFFLEN)
        error("Error in sending data!....Exiting\n");
    else
        printf("Sent %d bytes of data!\n", sent);

    /* CLOSE connection*/
    close(s_sock);
    close(c_sock);

    return 0;
}