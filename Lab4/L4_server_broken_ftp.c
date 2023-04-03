// Client server for broken FTP
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

void error(char* msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in server_addr;
    char sendbuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    return 0;
}