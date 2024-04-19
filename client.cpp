#include <iostream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>

class TCPClient {

private:
    std::string m_clientName;
    size_t m_serverPort;
    size_t m_connectionPeriod;

public:
    TCPClient(const std::string& clientName, size_t serverPort, size_t connectionPeriod)
            : m_clientName(clientName), m_serverPort(serverPort), m_connectionPeriod(connectionPeriod) {}
    
    void start() {
        struct sockaddr_in serverAddr;
        char buffer[1024];

        size_t clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(m_serverPort);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            return;
        }

        while (true) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S");
            ss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count() << "]" << m_clientName << "\n";

            std::string message = ss.str();
            if (send(clientSocket, message.c_str(), message.size(), 0) == -1) {
                std::cerr << "Failed to send message to server" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(m_connectionPeriod));
        }

        close(clientSocket);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    std::string clientName = argv[1];
    size_t serverPort = std::stoi(argv[2]);
    size_t connectionPeriod = std::stoi(argv[3]);

    TCPClient client(clientName, serverPort,connectionPeriod);
    client.start();
    return 0;
}