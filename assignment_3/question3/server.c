#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

// For simulating how NACK functions, we will send a NACK for every nth packet received
#define n 3

int main()
{
    int server, x, newsock;
    char buffer[1024];

    socklen_t addr_size;

    // Create a socket
    server = socket(PF_INET, SOCK_STREAM, 0);

    if (server < 0)
    {
        printf("Error creating socket...\n");
        exit(1);
    }
    else
    {
        printf("Socket created successfully...\n");
    }

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = htons(4000);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.7");

    // Bind the socket to the server address
    if (bind(server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
    {
        printf("Socket bound successfully...\n");
    }
    else
    {
        printf("Unable to bind server... An error occurred\n");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server, 5) == 0)
    {
        printf("Server listening...\n");
    }
    else
    {
        printf("Error in listening\n");
        exit(1);
    }

    struct sockaddr_in client;
    int len = sizeof(client);

    // Accept incoming connections
    newsock = accept(server, (struct sockaddr *)&client, &len);

    if (newsock == -1)
    {
        printf("Error in connecting with client\n");
        exit(1);
    }
    else
    {
        printf("Connection with client accepted\n");
    }

    // Buffer to store the content of a packet
    char pkt[1024];
    // Sequence number of a packet
    int seq_no = 0;
    // Buffer to send that contains either ACK/NACK
    char ack[6];
    int i = 0;
    // Count of packets received by the server
    int curr_pkts = 1;

    while (1)
    {   
        
        memset(ack, '\0', sizeof(ack));
        memset(pkt, '\0', sizeof(pkt));

        // Receive a packet from the client
        recv(newsock, pkt, sizeof(pkt), 0);

        // If the server receives the packet of the expected sequence number
        if (pkt[0] == (char)(seq_no) && curr_pkts % n != 0)
        {
            ack[0] = 'A';
            ack[1] = 'C';
            ack[2] = 'K';
            ack[3] = (char)(seq_no);
            ack[4] = '\0';

            printf("Packet received from client contains the data: ");
            for (int j = 1; pkt[j] != '\0'; j++)
                printf("%c", pkt[j]);
            printf("\n");

            // Update the sequence number for the next iteration
            seq_no = seq_no == 0 ? 1 : 0;
        }

        // If the server does not receive the packet of the expected sequence number
        else
        {
            ack[0] = 'N';
            ack[1] = 'A';
            ack[2] = 'C';
            ack[3] = 'K';
            ack[4] = (char)(seq_no);
            ack[5] = '\0';
        }

        sleep(1);

        // Send ACK/NACK back to the client
        send(newsock, ack, sizeof(ack), 0);
        curr_pkts++;

        // Check if the user entered "exit" to close the connection
        int noExit = 0;
        char exit[4] = {'e', 'x', 'i', 't'};

        for (int j = 1; j < 5; j++)
        {
            if (pkt[j] != exit[j - 1])
            {
                noExit = 1;
                break;
            }
        }

        if (noExit == 0)
        {
            printf("Server has successfully exited the network...\n");
            close(server);
            break;
        }
    }

    return 0;
}
