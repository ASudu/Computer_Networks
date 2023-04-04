#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define PORT 8888
#define BUFFLEN 256

int c_sock, key;
struct sockaddr_in client;
int clen = sizeof(client);
char* server_ip = "172.18.76.160";
char key_buff[2*BUFFLEN], val_buff[BUFFLEN], ch;

void error(char* msg)
/* This function is called to print error and close program*/
{
    perror(msg);
    exit(1);
}

void get()
/* This function gets the val_buff corresponding to the key_buff passed*/
{
    memset(key_buff, '\0', sizeof(key_buff));
    printf("Enter the key to find: \n");
    scanf("%d", &key);

    sprintf(key_buff, "%d", key);
    // Adding this to help server identify the command
    strcat(key_buff, " g");

    // Send the key
    if(send(c_sock, key_buff, sizeof(key_buff), 0) < 0)
        error("[-] Error in sending data (get)!\n");
    
    // Receive the corresponding value
    memset(val_buff, '\0', sizeof(val_buff));
    if(recv(c_sock, val_buff, sizeof(val_buff), 0) < 0)
        error("[-] Error in receiving response (get)!\n");
    else
    {
        printf("[+] Received the response (get)!\n");
        // Error if key could not be found
        if(!strcmp(val_buff, "ERROR"))
            printf("[-] Key does not exist in database!\n");
        else
        {
            printf("The value corresponding to %d: ", key);
            puts(val_buff);
        }

    }
}

void put()
/* This function adds a key_buff-val_buff pair in existing database
provided the key_buff doesn't already exists*/
{
    memset(key_buff, '\0', sizeof(key_buff));
    memset(val_buff, '\0', sizeof(val_buff));
    printf("Enter the key to add: \n");
    scanf("%d", &key);
    printf("Enter the corresponding value to add: \n");
    gets(val_buff);

    sprintf(key_buff, "%d", key);
    // Adding a space at end to separate key and value
    strcat(key_buff, " ");
    // Adding the value
    strcat(key_buff, val_buff);
    // Adding this to help server identify the command
    strcat(key_buff, " p");

    // Send the key
    if(send(c_sock, key_buff, sizeof(key_buff), 0) < 0)
        error("[-] Error in sending data (put)!\n");
    
    // Receive the corresponding value
    memset(val_buff, '\0', sizeof(val_buff));
    if(recv(c_sock, val_buff, sizeof(val_buff), 0) < 0)
        error("[-] Error in receiving response (put)!\n");
    else
    {
        printf("[+] Received the response (put)!\n");
        // If "OK" not received then error
        if(strcmp(val_buff, "OK"))
            printf("[-] %s\n",val_buff);
        else
            printf("[+] %s\n",val_buff);

    }
}

void delete()
/* This function deletes the key_buff-val_buff pair corresponding
to the key_buff if it exists*/
{
    memset(key_buff, '\0', sizeof(key_buff));
    printf("Enter the key to delete: \n");
    scanf("%d", &key);

    sprintf(key_buff, "%d", key);
    // Adding this to help server identify the command
    strcat(key_buff, " d");

    // Send the key
    if(send(c_sock, key_buff, sizeof(key_buff), 0) < 0)
        error("[-] Error in sending data (del)!\n");
    
    // Receive the corresponding value
    memset(val_buff, '\0', sizeof(val_buff));
    if(recv(c_sock, val_buff, sizeof(val_buff), 0) < 0)
        error("[-] Error in receiving response (del)!\n");
    else
    {
        printf("[+] Received the response (del)!\n");
        // If "OK" not received then error
        if(strcmp(val_buff, "OK"))
            printf("[-] %s\n",val_buff);
        else
            printf("[+] %s\n",val_buff);

    }
}

void quit()
/* This function exits the program and terminates the connection*/
{
    // Flush Buffer
    memset(key_buff, '\0', sizeof(key_buff));
    // Send Bye
    strcpy(key_buff, "Bye");
    if(send(c_sock, key_buff, sizeof(key_buff), 0) < 0)
        error("[-] Error in sending close request (quit)!\n");
    else
    {
        // Flush Buffer
        memset(val_buff, '\0', sizeof(val_buff));
        // Wait to receive response
        if(recv(c_sock, val_buff, sizeof(val_buff), 0) < 0)
            error("[-] Error in receiving response (quit)!\n");
        else
            puts(key_buff);

        printf("[+] Connection successfully closed (quit)!\n");
        close(c_sock);
        exit(0);
    }
}

int main()
/* The driver program*/
{
    /* CREATE TCP socket*/
    if((c_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        error("[-] Error in creating client socket!\n");
    else
        printf("[+] Client socket created!\n");
    
    /* CONNECT request*/
    memset(&client, 0, clen);
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = inet_addr(server_ip);

    if(connect(c_sock, (struct sockaddr*) &client, clen) < 0)
        error("[-] Error in connecting to server!\n");
    else
        printf("[+] Connected to server with IP: %s\n", inet_ntoa(client.sin_addr));
    
    /* SEND and RECEIVE data*/
    while(1)
    {
        printf("Enter one of the following options:\n");
        printf("p - put key_buff val_buff\n g - get key_buff\n d - del key_buff\n q - Quit");
        scanf("%c", &ch);
        ch = tolower(ch);

        switch(ch)
        {
            case 'p':
                put();
                break;
            case 'g':
                get();
                break;
            case 'd':
                delete();
                break;
            case 'q':
                quit();
                break;
            default:
                printf("[-] Invalid choice! Try again...\n");
                break;

        }
    }

    return 0;
}