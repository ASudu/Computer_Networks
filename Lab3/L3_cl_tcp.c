// Client-server for sending one message to each other
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

    /* CREATE TCP socket*/
    int c_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(c_sock < 0)
        error("Error creating the socket!....Exiting\n");
    else
        printf("Client socket successfully created!\n");
    
    /* CONNECT to a server*/
    // Populate server address structure
    struct sockaddr_in server_addr;
    char* server_ip = "172.18.76.160";
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    // Use "ip addr show eth0" to get the IP of the system
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    printf("Server address assigned!\n");

    // Connect
    int temp = connect(c_sock, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(temp < 0)
        error("Error in connecting to server!....Exiting\n");
    else
        printf("Successfully connected to server with IP: %s\n", server_ip);
    
    /* SEND data*/
    float x;
    printf("ENTER A REAL NUMBER: ");
    scanf("%f", &x);
    // Convert float to string
    gcvt(x, BUFFLEN, buffer);
    int sent = send(c_sock, buffer, BUFFLEN, 0);
    if(sent != BUFFLEN)
        error("Error in sending data!....Exiting\n");
    else
        printf("Sent %d bytes of data!\n", sent);
    
    /* RECEIVE data*/
    // Clear buffer
    memset(buffer, 0, BUFFLEN);
    int recvd = recv(c_sock, buffer, BUFFLEN, 0);
    if(recvd < 0)
        error("Error in receiving data!....Exiting\n");
    else
        printf("Server: %s\n", buffer);

    /* CLOSE connection*/
    close(c_sock); 

    return 0;
}