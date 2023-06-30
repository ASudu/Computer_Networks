// Client-server for sending one message to each other
// Server
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXPENDING 2
#define BUFFLEN 256
#define PORT 8888
#define PDR 10          // drop probability 10%

// Define function for error display
void error(char* msg)
{
    perror(msg);
    exit(1);
}

typedef struct packet
{
    char data[BUFFLEN];     // payload
    int size;               // payload size
    int seq_no;             // byte sequence number
    int client_no;          // to differentiate c1 and c2
    char TYPE;              // 'd' = data 'a' = ack
}packet;

packet snd_ack, rcv_pkt;
struct sockaddr_in server_addr, client_addr1, client_addr2;
int s_sock, bind_s, listen_s, c1_sock, c2_sock;
int expect1 = 0, expect2 = 0;
FILE* f;
char* file = "list.txt";
char buff[BUFFLEN + 5];

void initialize()
{
    strcpy(snd_ack.data, "NULL");
    snd_ack.seq_no = 0;
    snd_ack.size = 0;
    snd_ack.client_no = 0;
    snd_ack.TYPE = 'a';
    
    strcpy(rcv_pkt.data, "NULL");
    rcv_pkt.seq_no = 0;
    rcv_pkt.size = 0;
    rcv_pkt.client_no = 0;
    rcv_pkt.TYPE = 'd';
}

void write_file(char* line)
{
    fprintf(f,"%s",line);
}

int main()
{
    
    f = fopen(file,"w+");

    /* Create TCP Socket*/
    s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s_sock < 0)
        error("[-] Error in creating server socket! Exiting ...\n");
    else
        printf("[+] Server socket created!\n");

    /* BIND the soket to an address*/
    // Create local address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address: %s\n",inet_ntoa(server_addr.sin_addr));

    // Bind the socket to the address structure
    bind_s = bind(s_sock, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(bind_s < 0)
        error("[-] Error in binding socket! Exiting ...\n");
    else
        printf("[+] Server socket binding successful!\n");
    
    /* LISTEN for possible connections*/
    listen_s = listen(s_sock, MAXPENDING);
    if(listen_s < 0)
        error("[-] Error in socket listen! Exiting ...\n");
    else
        printf("[+] Server socket listening!\n");
    
    /* ACCEPT 2 TCP connections for c1 and c2*/
    // For c1
    int client_len1 = sizeof(client_addr1);
    c1_sock = accept(s_sock, (struct sockaddr*) &client_addr1, &client_len1);
    if(c1_sock < 0)
        error("[-] Error in accepting client c1! Exiting ...\n");
    else
    {
        printf("[+] Connection established with c1:\n");
        printf("[+] IP address: %s Port: %d\n", inet_ntoa(client_addr1.sin_addr), ntohs(client_addr1.sin_port));
    }

    // For c2
    int client_len2 = sizeof(client_addr2);
    c2_sock = accept(s_sock, (struct sockaddr*) &client_addr2, &client_len2);
    if(c2_sock < 0)
        error("[-] Error in accepting client c2! Exiting ...\n");
    else
    {
        printf("[+] Connection established with c2:\n");
        printf("[+] IP address: %s Port: %d\n", inet_ntoa(client_addr2.sin_addr), ntohs(client_addr2.sin_port));
    }

    /* SEND and RECEIVE*/
    int state = 0;

    while(1)
    {
        switch (state)
        {
            // Wait for data from client1
            case 0:{
                // Receive
                if(recv(c1_sock,&rcv_pkt, sizeof(rcv_pkt),0) < 0)
                    error("[-] Error receiving data from client1! Exiting ...\n");
                
                
                // As expected
                if(rcv_pkt.client_no == 1 && rcv_pkt.seq_no == expect1)
                {
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 1;
                    expect1 += rcv_pkt.size;

                    printf("[+] RCVD PKT: Seq. No. = %d, Size = %d, Client no. = %d\n",rcv_pkt.seq_no,rcv_pkt.size,rcv_pkt.client_no);

                    if(send(c1_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    
                    if(!strcmp(rcv_pkt.data, "done"))
                    {
                        
                        printf("[+] Operations done! Bye ...\n");
                        write_file(".");
                        /* CLOSE the connections*/
                        close(s_sock);
                        close(c1_sock);
                        close(c2_sock);
                        fclose(f);              // close the file
                        exit(0);
                    }

                    // Write in file
                    if(ftell(f))
                        write_file(",");
                    write_file(rcv_pkt.data);
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    state = 1;
                    break;
                }
                
                // For retransmission ack
                else if(rcv_pkt.client_no == 1 && rcv_pkt.seq_no < expect1 && rcv_pkt.seq_no >= 0)
                {
                    printf("Out of order: %d\n", rcv_pkt.seq_no);
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 1;
                    expect1 += rcv_pkt.size;

                    if(send(c1_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    break;
                }

                // Done
                else if(rcv_pkt.client_no == 1 && rcv_pkt.seq_no < 0)
                {
                    printf("Negg: %s\n", rcv_pkt.data);
                    write_file(".");
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 1;
                    expect1 += rcv_pkt.size;

                    if(send(c1_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);

                    /* CLOSE the connections*/
                    close(s_sock);
                    close(c1_sock);
                    close(c2_sock);
                    fclose(f);              // close the file
                    exit(0);
                }
                break;
            }
            // Wait for data from client2
            case 1:{
                // Receive
                if(recv(c2_sock,&rcv_pkt, sizeof(rcv_pkt),0) < 0)
                    error("[-] Error receiving data from client2! Exiting ...\n");
                

                // As expected
                if(rcv_pkt.client_no == 2 && rcv_pkt.seq_no == expect2)
                {
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 2;
                    expect2 += rcv_pkt.size;

                    printf("[+] RCVD PKT: Seq. No. = %d, Size = %d, Client no. = %d\n",rcv_pkt.seq_no,rcv_pkt.size,rcv_pkt.client_no);

                    if(send(c2_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client2! Exiting ...\n");
                    
                    if(!strcmp(rcv_pkt.data, "done"))
                    {
                        
                        printf("[+] Operations done! Bye ...\n");
                        write_file(".");
                        /* CLOSE the connections*/
                        close(s_sock);
                        close(c1_sock);
                        close(c2_sock);
                        fclose(f);              // close the file
                        exit(0);
                    }

                    // Write in file
                    if(ftell(f))
                        write_file(",");
                    write_file(rcv_pkt.data);
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    state = 2;
                    break;
                }
                
                // For retransmission ack
                else if(rcv_pkt.client_no == 2 && rcv_pkt.seq_no < expect2 && rcv_pkt.seq_no >= 0)
                {
                    printf("Out of order: %d\n", rcv_pkt.seq_no);
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 2;
                    expect2 += rcv_pkt.size;

                    if(send(c2_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client2! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    break;
                }

                // Done
                else if(rcv_pkt.client_no == 2 && rcv_pkt.seq_no < 0)
                {
                    printf("Negg: %s\n", rcv_pkt.data);
                    write_file(".");
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 2;
                    expect1 += rcv_pkt.size;

                    if(send(c2_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);

                    /* CLOSE the connections*/
                    close(s_sock);
                    close(c1_sock);
                    close(c2_sock);
                    fclose(f);              // close the file
                    exit(0);
                }
                break;
            }
            // Wait for data from client1
            case 2:{
                // Receive
                if(recv(c1_sock,&rcv_pkt, sizeof(rcv_pkt),0) < 0)
                    error("[-] Error receiving data from client1! Exiting ...\n");
                
                // As expected
                if(rcv_pkt.client_no == 1 && rcv_pkt.seq_no == expect1)
                {
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 1;
                    expect1 += rcv_pkt.size;

                    printf("[+] RCVD PKT: Seq. No. = %d, Size = %d, Client no. = %d\n",rcv_pkt.seq_no,rcv_pkt.size,rcv_pkt.client_no);

                    if(send(c1_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    
                    if(!strcmp(rcv_pkt.data, "done"))
                    {
                        
                        printf("[+] Operations done! Bye ...\n");
                        write_file(".");
                        /* CLOSE the connections*/
                        close(s_sock);
                        close(c1_sock);
                        close(c2_sock);
                        fclose(f);              // close the file
                        exit(0);
                    }
                    
                    // Write in file
                    if(ftell(f))
                        write_file(",");
                    write_file(rcv_pkt.data);
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    state = 3;
                    break;
                }
                
                // For retransmission ack
                else if(rcv_pkt.client_no == 1 && rcv_pkt.seq_no < expect1 && rcv_pkt.seq_no >= 0)
                {
                    printf("Out of order: %d\n", rcv_pkt.seq_no);
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 1;
                    expect1 += rcv_pkt.size;

                    if(send(c1_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    break;
                }

                // Done
                else if(rcv_pkt.client_no == 1 && rcv_pkt.seq_no < 0)
                {
                    printf("Negg: %s\n", rcv_pkt.data);
                    write_file(".");
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 1;
                    expect1 += rcv_pkt.size;

                    if(send(c1_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);

                    /* CLOSE the connections*/
                    close(s_sock);
                    close(c1_sock);
                    close(c2_sock);
                    fclose(f);              // close the file
                    exit(0);
                }
                break;
            }
            // Wait for data from client2
            case 3:{
                // Receive
                if(recv(c2_sock,&rcv_pkt, sizeof(rcv_pkt),0) < 0)
                    error("[-] Error receiving data from client2! Exiting ...\n");
                
                // As expected
                if(rcv_pkt.client_no == 2 && rcv_pkt.seq_no == expect2)
                {
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 2;
                    expect2 += rcv_pkt.size;

                    printf("[+] RCVD PKT: Seq. No. = %d, Size = %d, Client no. = %d\n",rcv_pkt.seq_no,rcv_pkt.size,rcv_pkt.client_no);

                    if(send(c2_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client2! Exiting ...\n");
                    
                    if(!strcmp(rcv_pkt.data, "done"))
                    {
                        
                        printf("[+] Operations done! Bye ...\n");
                        write_file(".");
                        /* CLOSE the connections*/
                        close(s_sock);
                        close(c1_sock);
                        close(c2_sock);
                        fclose(f);              // close the file
                        exit(0);
                    }
                    
                    // Write in file
                    if(ftell(f))
                        write_file(",");
                    write_file(rcv_pkt.data);
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    state = 0;
                    break;
                }
                
                // For retransmission ack
                else if(rcv_pkt.client_no == 2 && rcv_pkt.seq_no < expect2 && rcv_pkt.seq_no >= 0)
                {
                    printf("Out of order: %d\n", rcv_pkt.seq_no);
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 2;
                    expect2 += rcv_pkt.size;

                    if(send(c2_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client2! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);
                    break;
                }

                // Done
                else if(rcv_pkt.client_no == 2 && rcv_pkt.seq_no < 0)
                {
                    printf("Negg: %s\n", rcv_pkt.data);
                    write_file(".");
                    snd_ack.seq_no = rcv_pkt.seq_no;
                    snd_ack.TYPE = 'a';
                    snd_ack.client_no = 2;
                    expect1 += rcv_pkt.size;

                    if(send(c2_sock,&snd_ack,sizeof(snd_ack),0) < 0)
                        error("[-] Error in sending ack to client1! Exiting ...\n");
                    printf("[+] SENT ACK: Seq. No. = %d, Client No. = %d\n",snd_ack.seq_no,snd_ack.client_no);

                    /* CLOSE the connections*/
                    close(s_sock);
                    close(c1_sock);
                    close(c2_sock);
                    fclose(f);              // close the file
                    exit(0);
                }
                break;
            }
        }
    }

    /* CLOSE the connections*/
    close(s_sock);
    close(c1_sock);
    close(c2_sock);
    fclose(f);              // close the file
    return 0;
}
