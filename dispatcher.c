//Copy of serverWithPopupThread.c

//TODO: Add client as a function  [Assigned to: Alex]
//TODO: Add main code from pseudocode [Assigned to: Zach]
//TODO: Add saveFile function [Assigned to: Zach]
//TODO: Add readFile function [Assigned to: Christian]
//TODO: Add deleteFile function [Assigned to: Alex]
//TODO: Figure out Netcat [Assigned to: Christian]


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#define SERVER_PORT 1072 // Change this!
#define BUF_SIZE 256

// We make this a global so that we can refer to it in our signal handler
int serverSocket;

/*
 We need to make sure we close the connection on signal received, otherwise we have to wait
 for server to timeout.
 */
void closeConnection() {
    printf("\nClosing Connection with file descriptor: %d \n", serverSocket);
    close(serverSocket);
    exit(1);
}

// Create a separat emethod for

void * processClientRequest(void * request) {
    int connectionToClient = *(int *)request;
    char receiveLine[BUF_SIZE];
    char sendLine[BUF_SIZE];

    int bytesReadFromClient = 0;
    // Read the request that the client has
    while ( (bytesReadFromClient = read(connectionToClient, receiveLine, BUF_SIZE)) > 0) {
        // Need to put a NULL string terminator at end
        receiveLine[bytesReadFromClient] = 0;

        // Show what client sent
        printf("Received: %s\n", receiveLine);

        // Print text out to buffer, and then write it to client (connfd)
        snprintf(sendLine, sizeof(sendLine), "true");

        printf("%Sending s\n", sendLine);
        write(connectionToClient, sendLine, strlen(sendLine));

        // Zero out the receive line so we do not get artifacts from before
        bzero(&receiveLine, sizeof(receiveLine));
        close(connectionToClient);
    }
}



int main(int argc, char *argv[]) {
    int connectionToClient, bytesReadFromClient;

    // Create a server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family      = AF_INET;

    // INADDR_ANY means we will listen to any address
    // htonl and htons converts address/ports to network formats
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port        = htons(SERVER_PORT);

    // Bind to port
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("Unable to bind to port just yet, perhaps the connection has to be timed out\n");
        exit(-1);
    }

    // Before we listen, register for Ctrl+C being sent so we can close our connection
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = closeConnection;
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // Listen and queue up to 10 connections
    listen(serverSocket, 10);

    while (1) {
        /*
         Accept connection (this is blocking)
         2nd parameter you can specify connection
         3rd parameter is for socket length
         */
        connectionToClient = accept(serverSocket, (struct sockaddr *) NULL, NULL);

        // Kick off a thread to process request
        pthread_t someThread;
        pthread_create(&someThread, NULL, processClientRequest, (void *)&connectionToClient);

    }
}

//Client Code:

//int main(int argc, char *argv[]) {
//    int serverSocket, bytesRead;
//
//    // These are the buffers to talk back and forth with the server
//    char sendLine[BUF_SIZE];
//    char receiveLine[BUF_SIZE];
//
//    // Create socket to server
//    if ( (serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//        printf("Unable to create socket\n");
//        return -1;
//    }
//
//    // Setup server connection
//    struct sockaddr_in serverAddress;
//    bzero(&serverAddress, sizeof(serverAddress)); // Ensure address is blank
//
//    // Setup the type of connection and where the server is to connect to
//    serverAddress.sin_family = AF_INET; // AF_INET - talk over a network, could be a local socket
//    serverAddress.sin_port   = htons(SERVER_PORT); // Conver to network byte order
//
//    // Try to convert character representation of IP to binary
//    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
//        printf("Unable to convert IP for server address\n");
//        return -1;
//    }
//
//    // Connect to server, if we cannot connect, then exit out
//    if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
//        printf("Unable to connect to server");
//    }
//
//    // snprintf allows you to write to a buffer, think of it as a formatted print into an array
//    snprintf(sendLine, sizeof(sendLine), "Hey Frodo!");
//
//    // Write will actually write to a file (in this case a socket) which will transmit it to the server
//    write(serverSocket, sendLine, strlen(sendLine));
//
//    // Now start reading from the server
//    // Read will read from socket into receiveLine up to BUF_SIZE
//    while ( (bytesRead = read(serverSocket, receiveLine, BUF_SIZE)) > 0) {
//        receiveLine[bytesRead] = 0; // Make sure we put the null terminator at the end of the buffer
//        printf("Received %d bytes from server with message: %s\n", bytesRead, receiveLine);
//
//        // Got response, get out of here
//        break;
//    }
//
//    // Close the server socket
//    close(serverSocket);
//}
