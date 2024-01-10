#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#define MSG_LEN 1024

// function to calculate result from expression string
double evaluate(std::string expression)
{
    while (expression.size() && expression.back() == ' ')
        expression.pop_back();
    if (expression.size() == 0)
    {
        return INT_MIN;
    }
    int i = 0;
    while (expression[i] == ' ')
        i++;

    double x = 1, y = 1;
    if (expression[i] == '-')
    {
        x = -1;
        i++;
    }
    std::string temp = "";

    while (expression[i] != ' ')
    {
        if (!(isdigit(expression[i]) || expression[i] == '.'))
        {
            return INT_MIN;
        }
        temp += expression[i];
        i++;
    }

    x *= stod(temp);

    i++;
    char op = expression[i];
    i += 2;

    if (expression[i] == '-')
    {
        y = -1;
        i++;
    }
    temp = "";

    while (i < expression.size())
    {
        if (!(isdigit(expression[i]) || expression[i] == '.'))
        {
            return INT_MIN;
        }
        temp += expression[i];
        i++;
    }

    y *= stod(temp);

    switch (op)
    {
    case '+':
        return x + y;
        break;
    case '-':
        return x - y;
        break;
    case '*':
        return x * y;
        break;
    case '/':
        return x / y;
        break;
    case '^':
        return pow(x, y);
        break;
    default:
        return INT_MIN;
        break;
    }
}

int main(int argc, char const *argv[])
{
    // checking for valid number of arguments
    if (argc != 2)
    {
        std::cout << "Error: Invalid no. of arguments" << std::endl;
        exit(1);
    }

    // extracting port number from args
    const char *serverPort = argv[1];

    // instantiating server socket and checking for errors
    int serverFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverFd < 0)
    {
        perror("Socket Failure");
        exit(0);
    }

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(atoi(serverPort));

    // binding server socket to address and checking for errors
    if (bind(serverFd, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Bind Failure");
        exit(0);
    }
    else
    {
        std::cout << "Server listening on port " << serverPort << std::endl;
    }

    int clientSocket;
    sockaddr_in clientAddress;
    socklen_t addrlen = sizeof(clientAddress);

    char buffer[MSG_LEN] = {0};

    // running a infinite for continuous exchange of messages b/w clients and server
    while (true)
    {

        bzero(buffer, MSG_LEN);

        // reading msg from client and checking for error
        if (recvfrom(serverFd, buffer, MSG_LEN, MSG_WAITALL, (sockaddr *)&clientAddress, &addrlen) < 0)
        {
            std::cout << "Error: Unable to receive message from client" << std::endl;
            exit(1);
        }

        // calculating result from input expression
        double ans = evaluate(std::string(buffer));
        std::string res = std::to_string(ans);

        std::cout << "Recieved from client: " << inet_ntoa(clientAddress.sin_addr) << ":" << clientAddress.sin_port << ": " << buffer << "\n";
        bzero(buffer, MSG_LEN);

        // checking for invalid input from client
        if (ans != INT_MIN)
            strcpy(buffer, res.c_str());
        else
            strcpy(buffer, "Invalid input");

        // sending result to client and checking for errors
        if (sendto(serverFd, buffer, strlen(buffer), MSG_CONFIRM, (sockaddr *)&clientAddress, addrlen) < 0)
        {
            std::cout << "Error: Unable to send message to client" << std::endl;
            exit(1);
        }

        std::cout << "Sending to client: " << ans << std::endl;
    }

    // closing the server socket
    close(serverFd);
    return 0;
}