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
            std::cout << int(expression[i]) << "\n";
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
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        perror("Socket Failure");
        exit(0);
    }

    pid_t childpid;

    sockaddr_in serverAddress;
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

    // listening on server and checking for errors
    if (listen(serverFd, 5) < 0)
    {
        std::cout << "Error: Unable to listen for client connection request" << std::endl;
        exit(1);
    }

    while (true)
    {
        // accepting incoming client connection requests and checking for errors
        clientSocket = accept(serverFd, (sockaddr *)&clientAddress, &addrlen);
        if (clientSocket < 0)
        {
            std::cout << "Error: Unable to accept client connection request" << std::endl;
            exit(1);
        }

        // creating a new process using for everytime a new client connects
        if ((childpid = fork()) == 0)
        {
            close(serverFd);

            // extracting details of the client connected to server
            std::string clientIp = std::string(inet_ntoa(clientAddress.sin_addr));
            int clientPort = clientAddress.sin_port;
            std::cout << "New client connected at " << clientIp << ":" << clientPort << "\n";

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

                // calculating result from input expression
                double ans = evaluate(std::string(buffer));
                std::string res = std::to_string(ans);

                // checking for disconnection request from client
                if (strcmp(buffer, "/exit") == 0)
                {
                    std::cout << "client disconnected at " << clientIp << ":" << clientPort << "\n";
                    close(clientSocket);
                    exit(0);
                }

                std::cout << "Recieved from client: " << buffer << "\n";
                bzero(buffer, MSG_LEN);

                // checking for invalid input from client
                if (ans != INT_MIN)
                    strcpy(buffer, res.c_str());
                else
                    strcpy(buffer, "Invalid input");

                // sending result to client and checking for errors
                if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
                {
                    std::cout << "Error: Unable to send message to client" << std::endl;
                    exit(1);
                }
                std::cout << "Sending to client: " << ans << "\n";
            }
        }
    }
    //closing the server socket
    close(serverFd);
    return 0;
}