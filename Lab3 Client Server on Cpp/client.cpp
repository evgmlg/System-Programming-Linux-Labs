#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

const int TCP_PORT = 12345;
const int UDP_PORT = 12346;
const std::string FILE_PATH = "input.txt";

bool running = true;

void signalHandler(int signal) {
    running = false;
}

int TCPfuncClient()
{   
    // Set up signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    // Open the input file
    std::ifstream inputFile(FILE_PATH, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Failed to open input file." << std::endl;
        return 1;
    }

    // Create TCP socket
    int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1) {
        std::cerr << "Failed to create TCP socket." << std::endl;
        return 1;
    }

    // Set server address for TCP connection
    sockaddr_in tcpServerAddress{};
    tcpServerAddress.sin_family = AF_INET;
    tcpServerAddress.sin_port = htons(TCP_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &(tcpServerAddress.sin_addr)) <= 0) {
        std::cerr << "Invalid TCP server address." << std::endl;
        return 1;
    }

    // Connect to TCP server
    if (connect(tcpSocket, reinterpret_cast<sockaddr*>(&tcpServerAddress), sizeof(tcpServerAddress)) == -1) {
        std::cerr << "Failed to connect to TCP server." << std::endl;
        return 1;
    }

    // Read data from file and send it to the server
    while (running && !inputFile.eof()) {
        char buffer[1024];
        inputFile.read(buffer, sizeof(buffer));
        ssize_t bytesRead = inputFile.gcount();
        if (bytesRead > 0) {
            if (send(tcpSocket, buffer, bytesRead, 0) == -1) {
                std::cerr << "Failed to send data via TCP." << std::endl;
                return 1;
            }

            ssize_t bytesRead1= read(tcpSocket, buffer, sizeof(buffer));
            if (bytesRead1 > 0) {
                std::string message(buffer, bytesRead1);
                if (message == "pong") {
                    std::cerr << "Server send: " << message << std::endl;
                }
            }
        }
    }

    // Close connections
    close(tcpSocket);
    inputFile.close();    
}

int UDPfuncClient()
{   
    // Set up signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);
    
    // Open the input file
    std::ifstream inputFile(FILE_PATH, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Failed to open input file." << std::endl;
        return 1;
    }
    
    
    // Create UDP socket
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return 1;
    }
    
    // Set server address for UDP connection
    sockaddr_in udpServerAddress{};
    udpServerAddress.sin_family = AF_INET;
    udpServerAddress.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &(udpServerAddress.sin_addr)) <= 0) {
        std::cerr << "Invalid UDP server address." << std::endl;
        return 1;
    }

     if (connect(udpSocket, reinterpret_cast<sockaddr*>(&udpServerAddress), sizeof(udpServerAddress)) == -1) {
        std::cerr << "Failed to connect to UDP server." << std::endl;
        return 1;
    }

    // Read data from file and send it to the server
    while (running && !inputFile.eof()) {
        char buffer[1024];
        inputFile.read(buffer, sizeof(buffer));
        ssize_t bytesRead = inputFile.gcount();
        
        if (bytesRead > 0) {
            
            if (sendto(udpSocket, buffer, bytesRead, 0, reinterpret_cast<sockaddr*>(&udpServerAddress), sizeof(udpServerAddress)) == -1) {
                std::cerr << "Failed to send data via UDP." << std::endl;
                return 1;
            }

            char buffer[1024];
            sockaddr_in clientAddress{};
            socklen_t clientAddressLength = sizeof(clientAddress);
            
            ssize_t bytesRead2 = recvfrom(udpSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLength);
            
            if (bytesRead2 > 0) {
                std::string message(buffer, bytesRead2);
                if (message == "pong") {
                    std::cerr << "Server send: " << message << std::endl;
                }
            }
        }
    }

    // Close connections
    close(udpSocket);
    inputFile.close();
}

int main() {
 
    char b;
    std::cout << "1.Type 't' for use TCP.\n2.Type 'u' for use UDP.\n4.Press Ctrl+C to exit." << std::endl;
    std::cin >> b;

    std::cout << "Run client" << std::endl;

    switch (b)
    {
    case 't':
        TCPfuncClient();
        break;
    case 'u':
        UDPfuncClient();
        break;
    default:
        std::cout << "You type a wrong symbol!" << std::endl;
        break;
    }

    std::cout << "Client stopped." << std::endl;

    return 0;
}
