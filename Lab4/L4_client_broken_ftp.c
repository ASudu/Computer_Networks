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
    char* server_ip = "172.18.76.160";
    char rcvbuff[256];
    // Buffer to send file offset value
    unsigned char buff_offset[10];
    // Buffer to send complete file (0) or partial file (1)
    unsigned char buff_command[2];

    memset(rcvbuff, '\0', sizeof(rcvbuff));
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));

    /* CREATE TCP socket*/
    if((c_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Error in creating client socket!....Exiting\n");
    else
        printf("Client socket created!\n");
    
    /* Populate address structure*/
    server.sin_family = AF_INET;
    server.sin_port = htons(5001);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* CONNECT attempt*/
    if(connect(c_sock, (struct sockaddr*) &server, sizeof(server)) < 0)
        error("Error in connecting to server!....Exiting\n");
    
    /* Create file where data will be stored*/
    FILE* fp;
    fp = fopen("dest_file.txt", "ab");
    if(!fp)
        error("Error opening the file!....Exiting\n");
    // Go to end of file
    fseek(fp, 0, SEEK_END);
    // Get file size
    offset = ftell(fp);
    fclose(fp);

    fp = fopen("dest_file.txt", "ab");
    if(!fp)
        error("Error opening the file!....Exiting\n");
    
    printf("Enter (0) to get complete file, (1) to specify offset, (2) calculate offset value from local file: \n");
    scanf("%d", &command);
    sprintf(buff_command, "%d", command);
    write(c_sock, buff_command, 2);

    if(command == 1 || command == 2)
    {
        if(command == 1)
        {
            printf("Enter the value of file offset: \n");
            scanf("%d", &offset);

        }

        // Else we have offset value already
        sprintf(buff_offset, "%d", offset);
        write(c_sock, buff_offset, 10);
    }

    /* RECEIVE data in chunks of 256 bytes*/
    while((recvlen = read(c_sock, rcvbuff, 256)) > 0)
    {
        printf("Bytes received: %d\n", recvlen);
        fwrite(rcvbuff, 1, recvlen, fp);
        memset(rcvbuff, '\0', sizeof(rcvbuff));
    }
    
    if(recvlen < 0)
        error("Error in reading file!....Exiting\n");

    return 0;
}