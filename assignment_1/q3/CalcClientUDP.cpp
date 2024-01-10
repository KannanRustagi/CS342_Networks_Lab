#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
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
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0)
    {
        std::cout << "Error: Unable to create socket" << std::endl;
        exit(1);
    }

    // specify the server address and port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(serverPort));
    server_address.sin_addr.s_addr = inet_addr(serverIP);

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

        if (flag)
        {
            std::cout << "Closing the connection\n";
            break;
        }
        
        // sending message to server and checking for errors
        if (sendto(clientSocket, buffer, strlen(buffer), MSG_CONFIRM, (const sockaddr *)&server_address, sizeof(server_address)) <= 0)
        {
            std::cout << "Error: Unable to send message to server\n";
        }


        bzero(buffer, MSG_LEN);

        socklen_t len;

        // receiving message from server and checking for errors
        if (recvfrom(clientSocket, buffer, MSG_LEN, MSG_WAITALL, (sockaddr *)&server_address, &len) <= 0)
        {
            std::cout << "Error: Unable to receive message to server\n";
            break;
        }
        std::cout << "Server: " << buffer << std::endl;
    }

    //close the socket
    close(clientSocket);
    return 0;
}