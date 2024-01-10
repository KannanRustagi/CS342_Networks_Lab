#include <iostream>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#define MSG_LEN 1024

int main(int argc, char *argv[])
{
    // checking for correct number of arguments
    if (argc != 2)
    {
        std::cout << "Invalid number of arguments" << std::endl;
        exit(1);
    }

    // extracting port from args
    char *serverPort = argv[1];

    int masterSocket, newSocket, clientSocket[50] = {0}, maxClients = 50;
    int maxSd;
    struct sockaddr_in address;
    const char *msg = "connected successfully to server";
    char buffer[MSG_LEN];

    // set of socket descriptors
    fd_set socketSet;

    // create a master socket
    if ((masterSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "Error: Unable to create socket" << std::endl;
        exit(1);
    }

    address.sin_port = htons(atoi(serverPort));
    address.sin_family = PF_INET;
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(masterSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cout << "Error: Unable to bind to address provided" << std::endl;
        exit(1);
    }

    std::cout << "Server listening on port " << serverPort << std::endl;

    // listening on server and checking for errors
    if (listen(masterSocket, 5) < 0)
    {
        std::cout << "Error: Unable to listen" << std::endl;
        exit(1);
    }

    // accept the incoming connection
    int addrlen = sizeof(address);

    while (1)
    {
        // reset socket set
        FD_ZERO(&socketSet);

        // add master socket to set
        FD_SET(masterSocket, &socketSet);
        maxSd = masterSocket;

        // add child sockets to set
        for (int i = 0; i < maxClients; i++)
        {
            // if valid socket descriptor then add to read list
            if (clientSocket[i] > 0)
            {
                FD_SET(clientSocket[i], &socketSet);
            }

            // store the max value of socket descriptors
            maxSd = std::max(maxSd, clientSocket[i]);
        }

        // wait for activity on any socket
        int activity = select(maxSd + 1, &socketSet, NULL, NULL, NULL);

        if (activity < 0)
        {
            std::cout << "select error" << std::endl;
        }

        // check for disturbance on master socket
        if (FD_ISSET(masterSocket, &socketSet))
        {
            if ((newSocket = accept(masterSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                std::cout << "Error: Unable to accept incoming client connection request" << std::endl;
                exit(1);
            }

            // print new client details
            std::cout << "New client connected at " << inet_ntoa(address.sin_addr) << ":" << address.sin_port << std::endl;

            // send connection success message
            if (send(newSocket, msg, strlen(msg), 0) < 0)
            {
                std::cout << "Error: Unable to send message to client" << std::endl;
                exit(1);
            }

            // add new socket to array of sockets
            for (int i = 0; i < maxClients; i++)
            {
                if (clientSocket[i] == 0)
                {
                    clientSocket[i] = newSocket;
                    std::cout << "Adding to list of clients as " << i << std::endl;
                    break;
                }
            }
        }

        for (int i = 0; i < maxClients; i++)
        {

            if (FD_ISSET(clientSocket[i], &socketSet))
            {

                bzero(buffer, MSG_LEN);
                int recv = read(clientSocket[i], buffer, MSG_LEN);
                // if unable to recv message from client
                if (recv < 0)
                {
                    std::cout << "Error: Unable to receive message from client" << std::endl;
                    exit(1);
                }
                // if some client got disconnected
                else if (recv == 0)
                {
                    getpeername(clientSocket[i], (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    std::cout << "Client disconnected at " << inet_ntoa(address.sin_addr) << ":" << address.sin_port << std::endl;
                    clientSocket[i] = 0;
                    close(clientSocket[i]);
                }
                // if recv any message from client other than disconnection
                else if (strcmp(buffer, "/exit") != 0)
                {
                    std::cout << "client " << i << ": " << buffer << "\n";
                    bzero(buffer, MSG_LEN);
                    std::cout << "server: ";
                    fgets(buffer, MSG_LEN, stdin);
                    buffer[strcspn(buffer, "\n")] = 0;
                    if (send(clientSocket[i], buffer, strlen(buffer), 0) < 0)
                    {
                        std::cout << "Error: Unable to send message to client" << std::endl;
                        exit(1);
                    }
                }
            }
        }
    }

    close(masterSocket);
    return 0;
}
