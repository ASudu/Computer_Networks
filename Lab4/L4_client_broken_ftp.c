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
    int c_sock, command, offset, recvlen=0;
    char rcvbuff[10];
    unsigned char buff_offset[10];
    unsigned char buff_command[2];
    
    return 0;
}