#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888
#define BUFFLEN 256

int s_sock, c_sock;
struct sockaddr_in server, client;
