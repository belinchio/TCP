#include <iostream>
#include <fstream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>

std::mutex mtx;

class TCPServer {

private:
    
    size_t m_port;

public:

    TCPServer(size_t port) : m_port(port) {}

    void start() {
        struct sockaddr_in serverAddr, clientAddr;
        size_t serverSocket, clientSocket;
        socklen_t clientAddrSize = sizeof(clientAddr);
        char buffer[1024];

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(m_port);
        
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to bind to port " << m_port << std::endl;
            return;
        }

        listen(serverSocket, 5);

        while (true) {
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket < 0) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            std::thread clientThread(&TCPServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }

    void handleClient(size_t clientSocket) {
        char buffer[1024];

        std::string message;
        while (true) {
            size_t bytesRead = recv(clientSocket, buffer, 1024, 0);
            if (bytesRead < 1) {
                break;
            }
            message += std::string(buffer, bytesRead);
        }

        mtx.lock();
        std::ofstream logFile("log.txt", std::ios_base::app);
        logFile << message;
        logFile.close();
        mtx.unlock();

        close(clientSocket);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "<port>\n";
        return 1;
    }

    int serverPort = std::stoi(argv[1]);

    TCPServer server(serverPort);
    server.start();
    return 0;
}