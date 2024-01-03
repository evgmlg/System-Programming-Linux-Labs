#define DEFAULT_SOURCE
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

const int TCP_PORT = 12345;
const int UDP_PORT = 12346;

bool running = true;

void signalHandler(int signal) {
    running = false;
}

int TCPfunc()
{
    // Create TCP socket
    int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1) {
        std::cerr << "Failed to create TCP socket." << std::endl;
        return 1;
    }

    // Bind TCP socket
    sockaddr_in tcpServerAddress{};
    tcpServerAddress.sin_family = AF_INET;
    tcpServerAddress.sin_port = htons(TCP_PORT);
    tcpServerAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(tcpSocket, reinterpret_cast<sockaddr*>(&tcpServerAddress), sizeof(tcpServerAddress)) == -1) {
        std::cerr << "Failed to bind TCP socket." << std::endl;
        return 1;
    }

    // Listen for incoming TCP connections
    if (listen(tcpSocket, 5) == -1) {
        std::cerr << "Failed to listen on TCP socket." << std::endl;
        return 1;
    }

    while (running) {
        // Check for TCP connection
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(tcpSocket, &readSet);

        timeval timeout{};
        timeout.tv_sec = 1;

        if (select(tcpSocket + 1, &readSet, nullptr, nullptr, &timeout) > 0) {
            int tcpClientSocket = accept(tcpSocket, nullptr, nullptr);
            if (tcpClientSocket == -1) {
                std::cerr << "Failed to accept TCP connection." << std::endl;
                continue;
            }

            char buffer[1024];
            ssize_t bytesRead = read(tcpClientSocket, buffer, sizeof(buffer));
            if (bytesRead > 0) {
                std::string message(buffer, bytesRead);
                std::cout << "TCP message received: " << message << std::endl;
                if (message == "ping") {
                    send(tcpClientSocket, "pong", 4, 0);
                }
            }

            close(tcpClientSocket);
        }
    }

    // Clean up
    close(tcpSocket);
}

int UDPfunc()
{
    // Create UDP socket
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return 1;
    }

    // Bind UDP socket
    sockaddr_in udpServerAddress{};
    udpServerAddress.sin_family = AF_INET;
    udpServerAddress.sin_port = htons(UDP_PORT);
    udpServerAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(udpSocket, reinterpret_cast<sockaddr*>(&udpServerAddress), sizeof(udpServerAddress)) == -1) {
        std::cerr << "Failed to bind UDP socket." << std::endl;
        return 1;
    }

    while (running) {
        // Check for UDP data
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(udpSocket, &readSet);

        timeval timeout{};
        timeout.tv_sec = 1;

        if (select(udpSocket + 1, &readSet, nullptr, nullptr, &timeout) > 0) {
            char buffer[1024];
            sockaddr_in clientAddress{};
            socklen_t clientAddressLength = sizeof(clientAddress);
            
            ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLength);
            if (bytesRead > 0) {
                std::string message(buffer, bytesRead);
                std::cout << "UDP message received from " << inet_ntoa(clientAddress.sin_addr) << ": " << message << std::endl;
                if (message == "ping") {
                    sendto(udpSocket, "pong", 4, 0, reinterpret_cast<sockaddr*>(&clientAddress), clientAddressLength);
                }
            }
        }
    }

    // Clean up
    close(udpSocket);
}

int main() {
    // Set up signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    std::cout << "Server is running. Press Ctrl+C to exit." << std::endl;

    std::thread t1(TCPfunc);
    std::thread t2(UDPfunc);

    t1.join();
    t2.join();
    std::cout << "Server stopped." << std::endl;

    return 0;
}
