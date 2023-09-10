#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#define MAX 5
int connfd[MAX] = {0};
int i, e[MAX] = {0};
pthread_t tid2;
pthread_t tid[MAX];

// Function to read messages from clients
void *read_msg(void *arg)
{
    int k = *(int *)arg;
    int l;
    char str[100];
    read(connfd[k], str, 100);

    // Check if the received message is "exit"
    if (strcmp(str, "exit") == 0)
    {
        printf("Client %c is Offline\n", (char)(65 + k));
        close(connfd[k]);
        return (NULL);
    }

    // Check if the message is intended for all clients
    if (str[0] == '*')
    {
        for (l = 0; l < i; l++)
            if (l != k)
                write(connfd[l], str, 100);

        printf("%c says: %s\n", str[99], str);

        // Create a new thread to continue reading messages
        pthread_create(&tid[k], NULL, read_msg, (void *)&k);
    }
    else // Message is intended for a specific client
    {
        for (l = 0; l < i; l++)
            if (l == (str[0] - 'A'))
                write(connfd[l], str, 100);

        // Create a new thread to continue reading messages
        pthread_create(&tid[k], NULL, read_msg, (void *)&k);
    }
    return (NULL);
}

// Function to send messages to clients
void *write_msg(void *arg)
{
    int l;
    char str[100];
    fflush(stdin);
    scanf("%[^\n]%*c", str);
    str[99] = 'S';

    // Send the message to all clients
    for (l = 0; l < i; l++)
        write(connfd[l], str, 100);

    // Check if the entered message is "exit" to exit the program
    if (strcmp(str, "exit") == 0)
        exit(0);

    // Create a new thread to continue writing messages
    pthread_create(&tid2, NULL, write_msg, NULL);
    return (NULL);
}

int main(int argc, char *argv[])
{
    int listenfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];
    int temp;
    time_t ticks;
    char C_name[MAX][2];
    int j[MAX];

    // Create a socket for server communication
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the server address
    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // Start listening for incoming client connections
    listen(listenfd, 10);

    // Initialize client names and thread IDs
    for (i = 0; i < MAX; ++i)
    {
        C_name[i][0] = (char)(65 + i); // Client names A, B, C, ...
        C_name[i][1] = '\n';           // Newline character for printing
        j[i] = i;                      // Index used for thread creation
    }

    printf("My ID : S\n");

    // Create a thread for writing messages from the server
    pthread_create(&tid2, NULL, write_msg, NULL);

    // Accept connections from clients and create threads for each client
    for (i = 0; i < MAX; i++)
    {
        connfd[i] = accept(listenfd, (struct sockaddr *)NULL, NULL);

        // Send the client its name (A, B, C, ...) upon successful connection
        write(connfd[i], C_name[i], 2);

        printf("Successful connection with Client %c has been established\n", C_name[i][0]);

        // Create a thread for reading messages from this client
        pthread_create(&tid[i], NULL, read_msg, (void *)(j + i));
    }
}
