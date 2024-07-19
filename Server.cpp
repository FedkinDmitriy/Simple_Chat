#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class ChatServer
{
private:
    SOCKET serverSocket;
    std::unordered_map<std::string, SOCKET> clients;

public:
    ChatServer(int port)
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        listen(serverSocket, 5);
    }

    ~ChatServer()
    {
        closesocket(serverSocket);
        WSACleanup();
    }

    void AcceptClients()
    {
        while (true)
        {
            SOCKET clientSocket;
            sockaddr_in clientAddr;
            int addrLen = sizeof(clientAddr);
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);

            std::thread clientThread([this, clientSocket] { ClientHandler(clientSocket); });
            clientThread.detach();
        }
    }

    void ClientHandler(SOCKET clientSocket)
    {
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        std::string clientName(buffer, bytesReceived);
        clients[clientName] = clientSocket;

        std::cout << "Client connected: " << clientName << std::endl;

        while (true)
        {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0)
            {
                std::cout << "Client disconnected: " << clientName << std::endl;
                clients.erase(clientName);
                closesocket(clientSocket);
                break;
            }

            std::string messageReceived(buffer, bytesReceived);
            std::cout << "Received from " << clientName << ": " << messageReceived << std::endl;

            std::string buffer_to_send = clientName + ": " + messageReceived;

            for (auto& pair : clients)
            {
                if (pair.first != clientName)
                {  // Do not send message back to the sender
                    send(pair.second, buffer_to_send.c_str(), buffer_to_send.length(), 0);
                }
            }
        }
    }
};

int main()
{
    ChatServer server(1500);
    server.AcceptClients();
}
