#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class ChatClient
{
private:
    SOCKET clientSocket;
    sockaddr_in serverAddr;

public:
    ChatClient(const std::string& serverIp, int port)
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
    }

    ~ChatClient()
    {
        closesocket(clientSocket);
        WSACleanup();
    }

    void Connect()
    {
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Connection failed." << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Connected to the server. Welcome to the chat!" << std::endl;
    }

    void SendMessage(const std::string& message)
    {
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void ReceiveMessages()
    {
        char buffer[1024];
        while (true)
        {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived == SOCKET_ERROR || bytesReceived == 0)
            {
                std::cout << "Disconnected from server." << std::endl;
                break;
            }
            std::cout << std::string(buffer, bytesReceived) << std::endl;
        }
    }

    void Run()
    {
        std::thread receiveThread([this] { ReceiveMessages(); });
        receiveThread.detach();

        std::string input;
        std::string username;
        std::cout << "Enter username:\n";
        std::getline(std::cin, username);
        SendMessage(username);
        while (true)
        {
            std::getline(std::cin, input);
            SendMessage(input);
            if (input == "exit")
            {
                break;
            }
        }
    }
};

int main()
{
    std::string serverIp;
    std::cout << "Enter server IP address: ";
    std::cin >> serverIp;

    ChatClient client(serverIp, 1500);
    client.Connect();
    client.Run();
}
