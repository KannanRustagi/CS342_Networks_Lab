#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
    int client;

    socklen_t addr_size;

    // Creating the client socket
    client = socket(PF_INET, SOCK_STREAM, 0);
    if (client < 0)
    {
        printf("Error creating socket\n");
        exit(1);
    }
    else
    {
        printf("Socket creation successful\n");
    }

    struct sockaddr_in serv_addr;
    struct sockaddr_storage cli;

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = htons(4000);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.7");

    // Connecting with the server
    int y = connect(client, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (y == -1)
    {
        printf("Error in connecting with the server\n");
        exit(1);
    }
    else
    {
        printf("Connected to the server\n");
    }

    // Buffer to store the content of a packet
    char pkt[1024];
    // Sequence number
    int seq_no = 0;
    // Character array to store acknowledgement
    char ack[6];
    int i = 0;
    // Next sequence number
    int next = 1;

    while (1)
    {
        if (next == 1)
        {
            memset(pkt, '\0', sizeof(pkt));
            i = 1;
            // Taking input from the user for the content of the packet
            printf("\nEnter message to the server: ");
            while ((pkt[i++] = getchar()) != '\n')
                ;
            pkt[i - 1] = '\0';

            pkt[0] = (char)(seq_no);
        }

        sleep(1.5);

        // Send pkt to the server
        send(client, pkt, sizeof(pkt), 0);

        // Empty ack
        bzero(ack, 6);

        // Receive ack or nack from the server
        recv(client, ack, sizeof(ack), 0);

        // If pkt acknowledged by the server, proceed to the next pkt by setting next = 1
        if (ack[0] == 'A')
        {
            printf("Acknowledgment received from the server for pkt %d - [", seq_no);
            for (int j = 1; pkt[j] != '\0'; j++)
                printf("%c", pkt[j]);
            printf("]\n");
            seq_no = seq_no == 0 ? 1 : 0;
            next = 1;
        }
        // If pkt is not acknowledged, resend the same pkt by setting next = 0
        else
        {
            next = 0;
            printf("Negative acknowledgment received from the server for pkt %d - [", seq_no);
            for (int j = 1; pkt[j] != '\0'; j++)
                printf("%c", pkt[j]);
            printf("]\nResending pkt %d - [", seq_no);
            for (int j = 1; pkt[j] != '\0'; j++)
                printf("%c", pkt[j]);
            printf("]\n");
        }

        // Check if the user entered "exit" to close the connection
        char exit[4] = {'e', 'x', 'i', 't'};
        int noExit = 0;

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
            printf("Client has successfully exited the network...\n");
            close(client);
            break;
        }
    }

    return 0;
}
