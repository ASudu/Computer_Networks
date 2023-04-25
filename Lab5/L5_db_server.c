#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888
#define BUFFLEN 256
#define MAXPENDING 5

int s_sock, c_sock, childpid;
struct sockaddr_in server, client;
int clen = sizeof(client);
char s_buff[BUFFLEN], r_buff[2*BUFFLEN];
char c;
FILE* f;

void error(char* msg)
{
    perror(msg);
    exit(1);
}

void delete_line(int line)
{
    f = fopen("database.txt", "rw");
    FILE* fp = fopen("temp.txt", "w");
    char buffer[BUFFLEN];
    int lineno=1;

    if(!f)
        error("[-] Error opening database file");
    else if(!fp)
        error("[-] Error opening temp file");
    // Write all lines except "line" into temp file
    while(fgets(buffer, sizeof(buffer), f) != NULL)
    {
        if(lineno != line)
        {
            fprintf(fp, "%s", buffer);
        }

        lineno++;
    }

    fclose(f);
    fclose(fp);

    // Rename temp file
    remove("database.txt");
    rename("temp.txt", "database.txt");
}

void serve_put(char* r_buff)
{
    char to_write[BUFFLEN];
    // printf("Before copy\n");
    // Have a copy to write into the file
    strcpy(to_write, r_buff);
    // printf("Copied\n");

    // To remove " p"
    to_write[strlen(to_write) - 1] = '\0';
    to_write[strlen(to_write) - 1] = '\0';
    // Extract key to check if it already exists in the database
    char* key = strtok(r_buff, " ");
    // printf("Tokenized\n");
    char line[2*BUFFLEN];
    int found = -1;
    char reference_key[BUFFLEN], reference_val[BUFFLEN];
    // Open file in both read and write mode
    f = fopen("database.txt", "r+");
    int iter = 1;

    // File open error
    if(!f)
        error("[-] Error in opening file!\n");
    else
    {
        printf("[+] File opened\n");
        // Read line by line to check if key exists
        while(fgets(line, strlen(line), f) != NULL)
        {
            char* reference_key = strtok(line, " ");

            if(!strcmp(reference_key, key))
            {
                found = 1;

                // Send that there was an error
                memset(s_buff, '\0', sizeof(s_buff));
                strcpy(s_buff, "Key already exists!");
                if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
                    error("[-] Error sending data!\n");
                else
                {
                    printf("[+] Error message sent");
                    fclose(f);
                    break;
                }
            }

            iter++;
        }

        // If key doesn't exist
        if(found == -1)
        {
            fclose(f);
            f = fopen("database.txt", "a");
            fprintf(f,"\n%s",to_write);
            printf("[+] Updated file!\n");
            fclose(f);

            // Send that entry was successfully added
            memset(s_buff, '\0', sizeof(s_buff));
            strcpy(s_buff, "OK");
            if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
                error("[-] Error sending data!\n");
        }
    }
    // printf("In serve_put()");
}

void serve_get(char* r_buff)
{
    // Extract key to check if it already exists in the database
    char* key = strtok(r_buff, " ");
    char line[2*BUFFLEN];
    int found = -1;
    int iter=1;
    char ref_key[BUFFLEN], ref_val[BUFFLEN];

    // Open file in both read and write mode
    f = fopen("database.txt", "r+");
    // fseek(f,0,SEEK_SET);
    // printf("[+] Set pointer to start!\n");

    // File open error
    if(!f)
        error("[-] Error in opening file!\n");
    else
    {
        printf("[+] File opened\n");
        // Read line by line to check if key exists
        while(fscanf(f,"%s %s\n",ref_key, ref_val) != 0)
        {
            // char* reference_key = strtok(line, " ");
            // printf("%s\n", reference_key);
            // char* val = strtok(NULL, " ");


            if(!strcmp(ref_key, key))
            {
                found = 1;

                // Send the value
                memset(s_buff, '\0', sizeof(s_buff));
                strcpy(s_buff, ref_val);
                if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
                    error("[-] Error sending data!\n");
                else
                {
                    fclose(f);
                    break;
                }
            }

            iter++;
        }

        if(iter == 1)
            printf("[-] Did not enter while!\n");
        // If key doesn't exist
        if(found == -1)
        {
            // Send that entry was not found
            memset(s_buff, '\0', sizeof(s_buff));
            strcpy(s_buff, "Key doesn't exist!");
            if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
                error("[-] Error sending data!\n");
            else
                fclose(f);
        }
    }
    // printf("In serve_get()");
}

void serve_del(char* r_buff)
{
    // Extract key to check if it already exists in the database
    char* key = strtok(r_buff, " ");
    char line[BUFFLEN];
    int lineno = 1, found = -1;
    char reference_key[BUFFLEN], reference_val[BUFFLEN];

    // Open file in both read and write mode
    FILE* f = fopen("database.txt", "r+");

    // File open error
    if(!f)
        error("[-] Error in opening file!\n");
    else
    {
        // Read line by line to check if key exists
        while(fscanf(f,"%s %s\n", reference_key, reference_val) != 0)
        {
            printf("Line num: %d\n", lineno);
            if(!strcmp(reference_key, key))
            {
                found = 1;
                fclose(f);

                // Send the value
                delete_line(lineno);
                memset(s_buff, '\0', sizeof(s_buff));
                strcpy(s_buff, "OK");
                if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
                    error("[-] Error sending data!\n");
                else
                    break;
            }

            lineno++;
        }

        // If key doesn't exist
        if(found == -1)
        {
            // Send that entry was not found
            memset(s_buff, '\0', sizeof(s_buff));
            strcpy(s_buff, "Key doesn't exist!");
            if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
                error("[-] Error sending data!\n");
        }
    }
    // printf("In serve_del()");
}

int main()
{    
    /* CREATE TCP socket*/
    if((s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        error("[-] Error in creating server socket!\n");
    else
        printf("[+] Server socket created!\n");
    
    /* BIND the socket*/
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(s_sock, (struct sockaddr*) &server, sizeof(server))< 0)
        error("[-] Error in binding socket!\n");

    /* LISTEN*/
    if(listen(s_sock, MAXPENDING) < 0)
        error("[-] Error in listening!\n");
    else
        printf("[+] Socket listening!\n");
    
    /* HANDLE clients*/
    while(1)
    {
        if((c_sock = accept(s_sock, (struct sockaddr*) &client, &clen)) < 0)
            error("[-] Error in accepting connection request!\n");
        else
            printf("[+] Connection established with %s: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        
        if((childpid = fork()) == 0)
        {
            close(s_sock);
        
            while(1)
            {
                /* RECEIEVE data*/
                memset(r_buff, '\0', sizeof(r_buff));
                if(recv(c_sock, r_buff, sizeof(r_buff), 0) < 0)
                    error("[-] Error in receiving data!\n");
                else
                {
                    printf("[+] Received message from %s: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    int len = strlen(r_buff);
                    printf("Length: %d\n", len);

                    // Serve request based on the received message
                    // Case 1: Close request
                    if(!strcmp(r_buff, "Bye"))
                    {
                        memset(s_buff, '\0', sizeof(s_buff));
                        strcpy(s_buff, "Goodbye");

                        if(send(c_sock, s_buff, sizeof(s_buff), 0) < 0)
                            error("[-] Error in sending data (quit)!\n");
                        else
                        {
                            printf("[+] Disconnected from %s: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                            close(c_sock);
                            break;
                        }
                    }

                    else
                    {
                        c = r_buff[len - 1];
                        printf("Your choice was: %c\n", c);
                        switch(c)
                        {
                            // Case 2: Put request
                            case 'p':
                                // printf("put\n");
                                serve_put(r_buff);
                                // printf("put\n");
                                break;
                            // Case 3: Get request
                            case 'g':
                                printf("get\n");
                                serve_get(r_buff);
                                printf("put\n");
                                break;
                            // Case 4: Delete request
                            case 'd':
                                printf("del\n");
                                serve_del(r_buff);
                                printf("put\n");
                                break;
                            default:
                                printf("None matches!\n");
                                break;
                        }
                    }


                }

            }
        }
    }

    // Close connection
    close(s_sock);
    close(c_sock);
    return 0;
}