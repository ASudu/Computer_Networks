#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888
#define BUFFLEN 1024
#define MAXPENDING 5

void error(char* msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    int s_sock, c_sock, childpid, rcvlen;
    char buff[BUFFLEN];
    struct sockaddr_in server, client;
    int clen = sizeof(client);

    /* CREATE socket*/
    if((s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        error("[-] Error in creating server socket!\n");
    else
        printf("[+] Server socket created!\n");
    
    /* BIND socket*/
    // Populate address structure
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    // Bind
    int bind_s = bind(s_sock, (struct sockaddr*) &server, sizeof(server));

    /* LISTEN*/
    if(listen(s_sock, MAXPENDING) < 0)
        error("[-] Error in Listening!\n");
    else
        printf("[+] Server listening!\n");
    
    /* HANDLE multiple clients*/
    while(1)
    {
        // Accept connection
        if((c_sock = accept(s_sock, (struct sockaddr*) &client, &clen)) < 0)
            error("[-] Error in accepting connection request!\n");
        else
            printf("[+] Connection established with %s: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        
        // Fork
        if((childpid = fork()) == 0)
        {
            close(s_sock);

            while(1)
            {
                // Receive data
                memset(buff, '\0', BUFFLEN);
                rcvlen = recv(c_sock, buff, BUFFLEN, 0);
                if(!strcmp(buff, "exit") || !strcmp(buff, "bye"))
                {
                    int portno = ntohs(client.sin_port);
                    char* addr = inet_ntoa(client.sin_addr);
                    printf("[+] Disconnected from %s: %d!\n", addr, portno);
                    close(c_sock);
                    break;
                }

                else
                {
                    printf("Client: %s\n", buff);
                    send(c_sock, buff, BUFFLEN, 0);
                }
                    
                    
            }
        }

        // CLOSE
        close(c_sock);

    }
    return 0;
}