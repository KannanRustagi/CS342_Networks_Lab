#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define MSG_LEN 1024

int main(int argc, char const *argv[])
{
    // checking for valid number of arguments
    if (argc != 3)
    {
        std::cout << "Error: Invalid no. of arguments" << std::endl;
        exit(1);
    }
    // extracting hostname and server port from args
    const char *hostName = argv[1];
    const char *serverPort = argv[2];

    // decoding hostname to IP Address
    hostent *host_entry;
    if ((host_entry = gethostbyname(hostName)) == NULL)
    {
        std::cout << "Error: Unable to resolve host" << std::endl;
        exit(1);
    }
    char *serverIP = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));

    // instantiating client socket and checking for errors
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

    if (connect(clientSocket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cout << "Error: Can't connect to server" << std::endl;
        exit(0);
    }
    else
    {
        std::cout << "Connected to server" << std::endl;
    }
    char buffer[MSG_LEN] = {0};
    while (true)
    {
        bzero(buffer, MSG_LEN);
        // taking input expression from client
        printf("Enter expression of the form \"x op y\" where op is +,-,/,*,^ or \"\\exit\" to close\nClient: ");
        fgets(buffer, MSG_LEN, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (buffer[0] == 0)
            continue;

        // checking for disconnection request
        bool flag = (strcmp(buffer, "/exit") == 0);

        // sending message to server and checking for errors
        // std::cout<<send(clientSocket, buffer, strlen(buffer), 0) <<std::endl;
        if (send(clientSocket, buffer, strlen(buffer), 0) <= 0)
        {
            std::cout << "Error: Unable to send message to server" << std::endl;
            exit(1);
        }

        if (flag)
        {
            std::cout << "Closing the connection\n";
            break;
        }

        bzero(buffer, MSG_LEN);

        // receiving message from server and checking for errors
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