#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#define MSG_LEN 1024

int main(int argc, char const *argv[])
{
    // checking number of arguments and extracting port number
    if (argc != 2)
    {
        std::cout << "Error: Invalid no. of arguments" << std::endl;
        exit(1);
    }
    const char *serverPort = argv[1];
    
    // initialising client socket and checking for errors
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        std::cout << "Error: Unable to create socket" << std::endl;
        exit(1);
    }

    // setting server address and port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(serverPort));
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    // connecting clientsocket to server and  checking for errors
    if (connect(clientSocket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cout << "Error: Unable to connect to server" << std::endl;
        exit(1);
    }

    char buffer[MSG_LEN] = {0};

    // reading welcome message from server and checking for errors
    if (read(clientSocket, buffer, MSG_LEN) < 0)
    {
        std::cout << "Error: Unable to receive message from server" << std::endl;
        exit(1);
    }

    std::cout << buffer << std::endl;

    // running an infinite loop for continuous exchange of messages b/w client and server
    while (true)
    {
        bzero(buffer, MSG_LEN);

        //taking client message as input
        printf("Client: ");
        fgets(buffer, MSG_LEN, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        if(buffer[0]==0) continue;
        // checking for disconnection request
        bool flag = (strcmp(buffer, "/exit") == 0);

        // sending the message to server and checking for errors
        if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
        {
            std::cout << "Error: Unable to send message to server" << std::endl;
            exit(1);
        }

        // disconnecting and closing the client socket in case of disconnection request
        if (flag)
        {
            std::cout << "Closing the connection\n";
            break;
        }

        bzero(buffer, MSG_LEN);

        // reading the reply from server and checking for errors
        if (read(clientSocket, buffer, MSG_LEN) <= 0)
        {
            std::cout << "Error: Unable to receive message from server" << std::endl;
            exit(1);
        }
        std::cout << "Server: " << buffer << std::endl;
    }

    // close the socket
    close(clientSocket);
    return 0;
}