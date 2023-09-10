#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd = 0; // Socket file descriptor for communication with the server
char id[2];     // Unique identifier for the client
pthread_t tid1, tid2; // Thread IDs for reading and writing messages

// Function to read messages from the server
void *read_msg(void *arg)
{
    char str[100];
    read(sockfd, str, 100);

    // Check if the received message is "exit"
    if (strcmp(str, "exit") == 0)
    {
        printf("OK Bye!\n");
        close(sockfd);
        exit(0);
    }

    // Print the message received from the server
    printf("%c says: %s\n", str[99], str + 2);

    // Create a new thread to continue reading messages
    pthread_create(&tid1, NULL, read_msg, NULL);
    return (NULL);
}

// Function to write messages to the server
void *write_msg(void *arg)
{
    char str[100];
    fflush(stdout);
    printf("Enter message: \n");
    scanf("%[^\n]%*c", str);
    str[99] = id[0];
    write(sockfd, str, 100);

    // Check if the entered message is "exit"
    if (strcmp(str, "exit") == 0)
    {
        close(sockfd);
        exit(0);
    }

    // Create a new thread to continue writing messages
    pthread_create(&tid2, NULL, write_msg, NULL);
    return (NULL);
}

int main(int argc, char *argv[])
{
    int n = 0;
    char recvBuff[1024];
    int temp;
    struct sockaddr_in serv_addr;
    char s[100];

    if (argc != 3)
    {
        printf("\n Usage: %s <ip of server> \n", argv[0]);
        return 1;
    }

    memset(recvBuff, '0', sizeof(recvBuff));

    // Create a socket for communication
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Convert the server IP address from text to binary
    printf("Server present at the IP address: %s\n", argv[1]);
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        printf("\n inet_pton error occurred\n");
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    printf("\nSuccessful connection with server has been established\n");

    // Read the unique client identifier from the server
    read(sockfd, id, 2);
    printf("My ID : %c\n", id[0]);

    // Instructions for sending messages
    printf("Please follow the given instructions for sending messages-\n");
    printf("1. To send a message to all clients connected to the server, the format of the message should be * <message>\n");
    printf("2. To send a message to a particular client, the format of the message should be <Client ID> <message>\n");
    printf("3. To exit the chat room, enter 'exit'\n");

    // Create threads for reading and writing messages
    pthread_create(&tid1, NULL, read_msg, NULL);
    pthread_create(&tid2, NULL, write_msg, NULL);

    // Keep the main thread running
    while (1);
    return 0;
}
