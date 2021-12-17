#include <fstream>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>

int myServer() {

    // Creating a socket
    int clientSocket;
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        std::cerr << "Can't create socket!" << std::endl;
        return -1;
    }

    // Bind the socket to a IP / port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
        std::cerr << "Can't bind to IP/port" << std::endl;
        return -2;
    }
    std::cout << "listening..." << std::endl;
    
    // Mark the socket listening in
    if (listen(listening, SOMAXCONN) == -1) {
        std::cerr << "Can't listen" << std::endl;
        return -3;
    }

    // Accept the call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    if (clientSocket == -1) {
        std::cerr << "Problem with client connecting!" << std::endl;
        return -4;
    }

    // Close the listening socket
    close(listening);
    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);
    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    if (result) {
        std::cout << host << " connected on " << svc << std::endl;
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        std::cout << host << " connected on " << ntohs(client.sin_port) << std::endl;
    }

    // While receving display message
    int bufSize{ 16384 };
    char buf[bufSize]{'\0'};
    bool gotFileSize = false;
    std::string received{};
    std::string fileName{};
    // Clear the buffer
    memset(buf, 0, bufSize);
    // Wait for a message
    int byteRecv = recv(clientSocket, buf, bufSize, 0);
    while (byteRecv){

        if (byteRecv == -1) {
            std::cerr << "There was a connection issue" << std::endl;
            break;
        }
        if (byteRecv == 0) {
            std::cout << "The client disconnected" << std::endl;
            break;
        }
        if (std::string(buf) == "end of file" && !fileName.empty()) {
            // TODO: change path to receive files
            std::string path { "/home/code/Imaqliq-TZ/" };
            std::ofstream ofs(path + fileName);
            ofs << received;
            received.clear();
            std::cout << "received file: " << fileName << std::endl;
        } else {
            received += std::string(buf, 0, byteRecv);
        }
        // Get File name
        if (fileName.empty()) {
            fileName = std::string(buf);
            fileName = "Received_" + fileName;
            received = "";
        }
        
        memset(buf, 0, bufSize);
        byteRecv = recv(clientSocket, buf, bufSize, 0);
    }
    close(clientSocket);
    return 0;
}

void termHandler(int signum) {
    
}

void hupHandler(int signum) {

}

static void makeDaemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGTERM, termHandler);
    signal(SIGHUP, hupHandler);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/etc/init.d/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}

int main() {
    makeDaemon();
    
    while (1)
    {
        myServer();
        syslog (LOG_NOTICE, "First daemon started.");
        sleep (3);
        break;
    }
   
    syslog (LOG_NOTICE, "First daemon terminated.");
    closelog();
    
    return EXIT_SUCCESS;
}    
