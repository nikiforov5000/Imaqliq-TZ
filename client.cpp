#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <thread>



int myClient() {
        // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return 1;
    }

    // Create a hint for the server
    int port = 54000;
    std::string ipAddress{ "127.0.0.1" };
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1) {
        return 1;
    }
    char buf[4096];
    std::string userInput{};
    std::string endOfFile{ "end of file" };
    while (true) {
        // Enter file name
        std::cout << "Enter file name to send > ";
        std::getline(std::cin, userInput);

        // Exit on EXIT
        if (userInput == "EXIT") {
            break;
        }

        // Opend the file
        std::ifstream ifs(userInput);
        std::string line, sendBuf;
        if (!ifs.is_open()) {
            std::cout << "No such filename:" << userInput << std::endl;
            continue;
        }
        while (std::getline(ifs, line)) { sendBuf += line; }
        
        // Send file name
        int sendRes = send(sock, userInput.c_str(), userInput.length() + 1, 0);
        if (sendRes == -1) {
            std::cout << "Could not send to server!" << std::endl;
            return -1;
        }

        // Send data to the server
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sendRes = send(sock, sendBuf.c_str(), sendBuf.length() + 1, 0);
        if (sendRes == -1) { 
            std::cout << "Could not send to server!" << std::endl;
            return -1;
        }

        // Send "end of file"
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        sendRes = send(sock, endOfFile.c_str(), endOfFile.length() + 1, 0);
        if (sendRes == -1) {
            std::cout << "Could not send to server!" << std::endl;
            return -1;
        }
    }
    close(sock);
    return 0;
}

int main() {
    myClient();
}
/* 
    

clear
make server
make client
./server

clear
./client



 */