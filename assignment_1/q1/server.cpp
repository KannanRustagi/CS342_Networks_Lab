#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "base64.hpp"
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

    // instantiating socket and checking for errrors
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        perror("Socket Failure");
        exit(1);
    }
    pid_t childpid;

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(atoi(serverPort));

    // binding socket to server address and checking for errors
    if (bind(serverFd, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Bind Failure");
        exit(1);
    }
    else
    {
        std::cout << "Server listening on port " << serverPort << std::endl;
    }

    int clientSocket;
    sockaddr_in clientAddress;
    socklen_t addrlen = sizeof(clientAddress);

    // listening for client connection on socket and checking for errors
    if (listen(serverFd, 5) < 0)
    {
        perror("Listen Failure");
        exit(1);
    }

    // running an infinite loop to accept multiple clients
    while (true)
    {
        // accepting connection request from client and checking for errors
        clientSocket = accept(serverFd, (sockaddr *)&clientAddress, &addrlen);
        if (clientSocket < 0)
        {
            std::cout << "Error accepting client" << std::endl;
            exit(1);
        }

        // extracting details of the client connected
        std::string clientIp = std::string(inet_ntoa(clientAddress.sin_addr));
        int clientPort = clientAddress.sin_port;

        std::cout << "New client connected at " << clientIp << ":" << clientPort << "\n";

        // creating a new process using for everytime a new client connects
        if ((childpid = fork()) == 0)
        {
            close(serverFd);
            char buffer[MSG_LEN] = {0};

            // running a infinite for continuous exchange of messages b/w client and server
            while (true)
            {
                bzero(buffer, MSG_LEN);

                // reading msg from client and checking for error
                if (read(clientSocket, buffer, MSG_LEN) < 0)
                {
                    std::cout << "Error: Unable to receive message from client" << std::endl;
                    exit(1);
                }

                // decoding from base64
                strcpy(buffer, base64::from_base64(buffer).c_str());

                // check for disconnection request from client
                if (buffer[0] == '3')
                {
                    std::cout << "Client disconnected at " << clientIp << ":" << clientPort << "\n";
                    close(clientSocket);
                    exit(1);
                }

                std::cout << "Client Message: " << buffer << "\n";
                const char *msg = "received message from client\n";

                // sending result to client and checking for errors
                if (send(clientSocket, msg, strlen(msg), 0) < 0)
                {
                    std::cout << "Error: Unable to send message to client" << std::endl;
                    exit(1);
                }
            }
        }
    }
    close(serverFd);
    return 0;
}