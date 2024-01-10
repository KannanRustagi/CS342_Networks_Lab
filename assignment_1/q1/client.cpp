#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "base64.hpp"
#define MSG_LEN 1024

int main(int argc, char const *argv[])
{
    // checking number of arguments and extracting port number and server IP address
    if (argc != 3)
    {
        std::cout << "Error: Invalid no. of arguments" << std::endl;
        exit(1);
    }
    const char *serverIP = argv[1];
    const char *serverPort = argv[2];

    //instantiating socket and checking for errors
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        std::cout << "Error: Unable to create socket" << std::endl;
        exit(1);
    }

    // specify the server address and port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(serverPort));
    inet_pton(AF_INET, serverIP, &server_address.sin_addr);

    // connecting client to server and checking for errors
    if (connect(clientSocket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cout << "Error: Can't connect to server" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "Successfully connected to server" << std::endl;
    }

    char buffer[MSG_LEN] = {0};

    // running infinite loop for continuous exchange of msg b/w server and client
    while (true)
    {
        bzero(buffer, MSG_LEN);
        
        //take input from client
        std::cout << "Client: ";
        fgets(buffer, MSG_LEN, stdin);
        bool flag = buffer[0] == '3';

        //encode to base64
        strcpy(buffer, base64::to_base64(buffer).c_str());

        //send msg to server and check for errors
        if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
        {
            std::cout << "Error: Unable to send message to server" << std::endl;
            exit(1);
        }

        // close the connection in case of msg of type 3
        if (flag)
        {
            std::cout << "Closing the connection" << std::endl;
            break;
        }

        bzero(buffer, MSG_LEN);

        // read response from server and check for errors
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