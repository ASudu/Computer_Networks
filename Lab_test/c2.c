// Client-server for sending one message to each other
// Client2
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>           // for rand
#include <sys/select.h>     // for select

#define MAXPENDING 2
#define TIMEOUT 2
#define BUFFLEN 256
#define IDLEN 30
#define MAXENTRIES 100  // Max number of entries in file
#define PORT 8888

char* file = "id.txt";
struct sockaddr_in server_addr;
char* server_ip = "127.0.1.1";
int c_sock;
int curr_data;

int select_out;

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

packet snd_pkt, rcv_ack;

typedef struct data
{
    int offset;
    char id[IDLEN];
}data;

data id_data[MAXENTRIES];

void initialize()
{
    for(int i=0; i<MAXENTRIES; i++)
    {
        memset(id_data[i].id, '\0', IDLEN);
        id_data[i].offset = -1;
    }

    printf("[+] id data structure initialized!\n");
}

void populate_array()
{
    FILE* f = fopen(file, "r");
    int i=0,off=0;
    id_data[0].offset = 0;


    if(!f)
        error("[-] Error opening id.txt! Exiting ...\n");
    else
    {
        while(1)
        {
            char c = fgetc(f);

            // EOF
            if(c == '.')
            {
                printf("[+] File ended\n");
                break;
            }
            
            // id ended
            else if(c==',' || c=='\n')
            {
                id_data[i+1].offset = id_data[i].offset + off;
                off = 0;
                i++;
            }

            else
            {
                id_data[i].id[off] = c;
                off++;
            }

        }
    }

    printf("Number of entries: %d\n",i+1);
    printf("[+] Array of structures populated\n");

    fclose(f);
}

int main()
{

    // Initialize
    initialize();
    populate_array();

    // for(int i=0; i<MAXENTRIES; i++)
    // {
    //     printf("Data: %s        Offset:%d\n",id_data[i].id,id_data[i].offset);
    // }

    /* CREATE TCP socket*/
    c_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(c_sock < 0)
        error("[-] Error creating the socket!....Exiting\n");
    else
        printf("[+] Client socket successfully created!\n");
    
    /* CONNECT to a server*/
    // Populate server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    // Use "ip addr show eth0" to get the IP of the system
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connect
    int temp = connect(c_sock, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(temp < 0)
        error("[-] Error connecting to server! Exiting ...\n");
    else
        printf("[+] Successfully connected to server with IP: %s\n", server_ip);


    int state = 0;
    curr_data = 0;

    while(1)
    {
        switch(state)
        {
            case 0:{     // Send first
                // Populate send packet
                // if non null entries exist
                if(id_data[curr_data].offset >= 0)
                {
                    strcpy(snd_pkt.data, id_data[curr_data].id);
                    snd_pkt.seq_no = id_data[curr_data].offset;
                    snd_pkt.size = strlen(snd_pkt.data);
                    snd_pkt.client_no = 2;      // since c2
                    snd_pkt.TYPE = 'd';
                }

                // All non-null entries are done
                else
                {
                    strcpy(snd_pkt.data, "done");
                    snd_pkt.seq_no = id_data[curr_data].offset;
                    snd_pkt.size = strlen(snd_pkt.data);
                    snd_pkt.client_no = 2;      // since c2
                    snd_pkt.TYPE = 'd';

                    // Send
                    if(send(c_sock, &snd_pkt, sizeof(snd_pkt), 0) < 0)
                        error("[-] Error in sending packet! Exiting ...\n");
                    printf("[+] All names done\n");
                    close(c_sock);
                    exit(0);
                    break;
                }

                // Send
                if(send(c_sock, &snd_pkt, sizeof(snd_pkt), 0) < 0)
                    error("[-] Error in sending packet! Exiting ...\n");
                else
                {
                    printf("[+] SENT PKT: Seq. No. = %d, Size = %d, Client No. = %d!\n", snd_pkt.seq_no, snd_pkt.size, snd_pkt.client_no);
                    state = 1;
                    break;
                }
                break;
            }
            case 1:{     // Wait for ACK

                /***************** TIMER ****************/
                struct timeval tv;
                tv.tv_sec = TIMEOUT;
                tv.tv_usec = 0;
                fd_set rcvset;
                FD_ZERO(&rcvset);
                FD_SET(c_sock,&rcvset);

                // Monitor the sockets 0 to (c_sock + 1) - 1
                if(select_out = select(c_sock + 1, &rcvset, NULL, NULL, &tv) < 0)
                    error("[-] Error in select call! Exiting...\n");
                
                // Timeout => retransmit
                else if(select_out == 0)
                {
                    if(send(c_sock,&snd_pkt,sizeof(snd_pkt),0) < 0)
                        error("[-] Error in sending packet! Exiting...\n");

                    printf("[+] RE-TRANSMIT PKT: Seq. No. = %d, Size = %d, Client no. = %d\n",snd_pkt.seq_no,snd_pkt.size,snd_pkt.client_no);
                    state=1;
                    break;
                }
                /***************** TIMER ****************/

                // Socket readable
                else
                {
                    if(recv(c_sock,&rcv_ack,sizeof(rcv_ack),0) < 0)
                        error("[-] Error receiving ack! Exiting ...\n");
                    
                    if(rcv_ack.seq_no==-1){
                        error("Done");
                    }                         
                    // Seq number is same as sent offset
                    // This is packet dedicated for client 2
                    if(rcv_ack.client_no == 2 && rcv_ack.seq_no == id_data[curr_data].offset)
                    {
                        printf("[+] RCVD ACK: Seq. No. %d\n",rcv_ack.seq_no);
                        state = 2;
                        curr_data++;
                        break;
                    }
                }                
                break;
            }
            case 2:{     // Send first
                // Populate send packet
                // if non null entries exist
                if(id_data[curr_data].offset >= 0)
                {
                    strcpy(snd_pkt.data, id_data[curr_data].id);
                    snd_pkt.seq_no = id_data[curr_data].offset;
                    snd_pkt.size = strlen(snd_pkt.data);
                    snd_pkt.client_no = 2;      // since c2
                    snd_pkt.TYPE = 'd';
                }

                // All non-null entries are done
                else
                {
                    strcpy(snd_pkt.data, "done");
                    snd_pkt.seq_no = id_data[curr_data].offset;
                    snd_pkt.size = strlen(snd_pkt.data);
                    snd_pkt.client_no = 2;      // since c2
                    snd_pkt.TYPE = 'd';

                    // Send
                    if(send(c_sock, &snd_pkt, sizeof(snd_pkt), 0) < 0)
                        error("[-] Error in sending packet! Exiting ...\n");
                    printf("[+] All ids done\n");
                    close(c_sock);
                    exit(0);
                    break;
                }
                
                // Send
                if(send(c_sock, &snd_pkt, sizeof(snd_pkt), 0) < 0)
                    error("[-] Error in sending packet! Exiting ...\n");
                else
                {
                    printf("[+] SENT PKT: Seq. No. = %d, Size = %d, Client No. = %d!\n", snd_pkt.seq_no, snd_pkt.size, snd_pkt.client_no);
                    state = 3;
                    break;
                }
                break;
            }
            case 3:{     // Wait for ACK

                /***************** TIMER ****************/
                struct timeval tv;
                tv.tv_sec = TIMEOUT;
                tv.tv_usec = 0;
                fd_set rcvset;
                FD_ZERO(&rcvset);
                FD_SET(c_sock,&rcvset);

                // Monitor the sockets 0 to (c_sock + 1) - 1
                if(select_out = select(c_sock + 1, &rcvset, NULL, NULL, &tv) < 0)
                    error("[-] Error in select call! Exiting...\n");
                
                // Timeout => retransmit
                if(select_out == 0)
                {
                    if(send(c_sock,&snd_pkt,sizeof(snd_pkt),0) < 0)
                        error("[-] Error in sending packet! Exiting...\n");
                    
                    printf("[+] RE-TRANSMIT PKT: Seq. No. = %d, Size = %d, Client no. = %d\n",snd_pkt.seq_no,snd_pkt.size,snd_pkt.client_no);
                    state=3;
                    break;
                }
                /***************** TIMER ****************/

                // Socket readable
                else
                {
                    if(recv(c_sock,&rcv_ack,sizeof(rcv_ack),0) < 0)
                        error("[-] Error receiving ack! Exiting ...\n");
                    
                    
                    // Seq number is same as sent offset
                    if(rcv_ack.seq_no==-1){
                        error("Done");
                    }       
                    // This is packet dedicated for client 2
                    if(rcv_ack.client_no == 2 && rcv_ack.seq_no == id_data[curr_data].offset)
                    {
                        printf("[+] RCVD ACK: Seq. No. %d\n",rcv_ack.seq_no);
                        state = 0;
                        curr_data++;
                        break;
                    }
                }
                break;
            }
        }
    }

    close(c_sock);
    return 0;
}