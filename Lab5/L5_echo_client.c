#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888
#define BUFFLEN 1024

void error(char* msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    int c_sock;
    struct sockaddr_in client;
    char* server_ip = "172.18.76.160";
    int clen = sizeof(client);
    int rcvlen;
    char s_buff[BUFFLEN], r_buff[BUFFLEN];

    /* CREATE socket*/
    if((c_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        error("[-] Error creating client socket!\n");
    else
        printf("[+] Client socket created!\n");

    /* ESTABLISH connection*/
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = inet_addr(server_ip);

    if(connect(c_sock, (struct sockaddr*) &client, sizeof(client)) < 0)
        error("[-] Error connecting to server!\n");
    else
        printf("[+] Connected to server with IP %s\n", inet_ntoa(client.sin_addr));
    

    /* SEND and RECEICE data*/
    while(1)
    {
        memset(s_buff, '\0', sizeof(s_buff));
        printf("Enter message (\"bye\" or \"exit\" to end): ");
        gets(s_buff);

        // Send data
        if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
            error("[-] Error in sending message!\n");
        else
            if(!strcmp(s_buff, "bye") || !strcmp(s_buff, "exit"))
                break;
        
        // Receive data
        memset(r_buff, '\0', sizeof(r_buff));
        if((rcvlen = recv(c_sock, r_buff, sizeof(r_buff), 0)) < 0)
            error("[-] Error in receiving data!\n");
        else
            printf("%s\n", r_buff);
    }

    /* CLOSE connection*/
    close(c_sock);
    printf("[+] Connection closed!\n");
    
    return 0;
}