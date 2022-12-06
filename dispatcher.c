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

#define DISPATCHER_SERVER_PORT 1073 // Change this!
#define CACHE_SERVER_PORT 1078
#define FILE_SERVER_PORT 1079
#define BUF_SIZE 256

int serverSocket;

char** tokenizeInput(char *buffer) {
    char **inputTokens = malloc(2 * sizeof(char*));
    char *token;

    token = strtok(buffer, " ");
    inputTokens[0] = malloc(strlen(token) + 1);
    strcpy(inputTokens[0], token);

    token = strtok(NULL, "");
    inputTokens[1] = malloc(strlen(token) + 1);
    strcpy(inputTokens[1], token);

    return inputTokens;
}
/*
int main(int argc, char *argv[]) {
    char buffer[256];
    char **inputTokens;
    char *operation, *restOfInput;
    char message[256];

    printf("Enter input: ");
    fgets(buffer, 256, stdin);

    buffer[strlen(buffer) - 1] = '\0';

    inputTokens = tokenizeInput(buffer);

    operation = inputTokens[0];
    restOfInput = inputTokens[1];

    if (strcmp(operation, "save") == 0) {
        snprintf(message, sizeof(message), "write %s", restOfInput);
    } else if (strcmp(operation, "read") == 0) {
        snprintf(message, sizeof(message), "load %s", restOfInput);
    } else if (strcmp(operation, "delete") == 0) {
        snprintf(message, sizeof(message), "delete %s", restOfInput);
    }

    printf("%s\n", message);
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Client as a function

void sendClientRequest(char *sendCommand, int port, char *response) {
    int serverSocket, bytesRead;
    // These are the buffers to talk back and forth with the server
    char sendLine[BUF_SIZE];
    char receiveLine[BUF_SIZE];

    // Create socket to server
    if ( (serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Unable to create socket\n");
        return;
    }

    // Setup server connection
    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress)); // Ensure address is blank

    // Setup the type of connection and where the server is to connect to
    serverAddress.sin_family = AF_INET; // AF_INET - talk over a network, could be a local socket
    serverAddress.sin_port   = htons(port); // Conver to network byte order

    // Try to convert character representation of IP to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        printf("Unable to convert IP for server address\n");
        return;
    }

    // Connect to server, if we cannot connect, then exit out
    if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printf("Unable to connect to server");
    }

    // Write will actually write to a file (in this case a socket) which will transmit it to the server
    write(serverSocket, sendCommand, strlen(sendCommand));

    if (response != NULL) {
        // Now start reading from the server
        // Read will read from socket into receiveLine up to BUF_SIZE
        while ( (bytesRead = read(serverSocket, receiveLine, BUF_SIZE)) > 0) {
            receiveLine[bytesRead] = 0; // Make sure we put the null terminator at the end of the buffer
            printf("Received %d bytes from server with message: %s\n", bytesRead, receiveLine);
            //response = malloc(256);
            strcpy(response, receiveLine);

            // Got response, get out of here
            break;
        }
    }

    // Close the server socket
    close(serverSocket);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Server Code:


//We need to make sure we close the connection on signal received, otherwise we have to wait
//for server to timeout.
void closeConnection() {
    printf("\nClosing Connection with file descriptor: %d \n", serverSocket);
    close(serverSocket);
    exit(1);
}

// Create a separate method for

void * processClientRequest(void * request) {
    int connectionToClient = *(int *)request;
    char receiveLine[BUF_SIZE];
    char sendLine[BUF_SIZE];
    char cacheMessage[BUF_SIZE];
    char fileServerMessage[BUF_SIZE];
    char **inputTokens;
    char *operation, *restOfInput, *message;

    char s[] = "save";
    char r[] = "read";
    char d[] = "delete";

    int bytesReadFromClient = 0;
    // Read the request that the client has
    while ( (bytesReadFromClient = read(connectionToClient, receiveLine, BUF_SIZE)) > 0) {
        // Need to put a NULL string terminator at end
        receiveLine[bytesReadFromClient] = 0;

        // Show what client sent
        printf("Received: %s\n", receiveLine);

        // Tokenize the input
        inputTokens = tokenizeInput(receiveLine);
        operation = inputTokens[0];
        restOfInput = inputTokens[1];


        if (strcmp(operation, s) == 0) {
            snprintf(fileServerMessage, sizeof(fileServerMessage), "write %s", restOfInput);
            sendClientRequest(fileServerMessage, FILE_SERVER_PORT, NULL);
        } else if (strcmp(operation, r) == 0) {
            char response[256] = "";
            snprintf(cacheMessage, sizeof(cacheMessage), "load %s", restOfInput);
            snprintf(fileServerMessage, sizeof(fileServerMessage), "read %s", restOfInput);
            sendClientRequest(cacheMessage, CACHE_SERVER_PORT, response);
            if (strcmp(response, "0:\n") == 0) {
                printf("sending to fs...");
                sendClientRequest(fileServerMessage, FILE_SERVER_PORT, response);
                sendClientRequest(response, DISPATCHER_SERVER_PORT, NULL);
            } else if (strcmp(response, "0:\n") != 0) {
                printf("sending to network...");
                sendClientRequest(response, DISPATCHER_SERVER_PORT, NULL);
            }
            printf("response was: %s", response);
        } else if (strcmp(operation, d) == 0) {
            snprintf(fileServerMessage, sizeof(fileServerMessage), "rm %s", restOfInput);
            snprintf(cacheMessage, sizeof(cacheMessage), "delete %s", restOfInput);
            sendClientRequest(fileServerMessage, FILE_SERVER_PORT, NULL);
            sendClientRequest(cacheMessage, CACHE_SERVER_PORT, NULL);
        }
        printf("skipped sending :(");
    }

    // Print text out to buffer, and then write it to client (connfd)
    snprintf(sendLine, sizeof(sendLine), "true");

    printf("Sending %s\n", sendLine);
    write(connectionToClient, sendLine, strlen(sendLine));

    // Zero out the receive line so we do not get artifacts from before
    bzero(&receiveLine, sizeof(receiveLine));
    close(connectionToClient);
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
    serverAddress.sin_port        = htons(DISPATCHER_SERVER_PORT);

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
        //Accept connection (this is blocking)
        //2nd parameter you can specify connection
        //3rd parameter is for socket length
        connectionToClient = accept(serverSocket, (struct sockaddr *) NULL, NULL);

        // Kick off a thread to process request
        pthread_t someThread;
        pthread_create(&someThread, NULL, processClientRequest, (void *)&connectionToClient);

    }
}

