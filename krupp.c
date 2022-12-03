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

#define DISPATCHER_SERVER_PORT 1072 // Change this!
#define CACHE_SERVER_PORT 1075
#define FILE_SERVER_PORT 1076
#define BUF_SIZE 256


void * processClientRequest(void * request);

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
        char response[256];
        sendClientRequest("delete fs", 1085, NULL);
        sendClientRequest("delete ms", 1086, NULL);
    }

    printf("%s\n", message);
}


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
        return -1;
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
        return -1;
    }

    // Connect to server, if we cannot connect, then exit out
    if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printf("Unable to connect to server");
    }

    // snprintf allows you to write to a buffer, think of it as a formatted print into an array
    snprintf(sendLine, sizeof(sendLine), sendCommand);

    // Write will actually write to a file (in this case a socket) which will transmit it to the server
    write(serverSocket, sendLine, strlen(sendLine));

    if (response != NULL) {
        // Now start reading from the server
        // Read will read from socket into receiveLine up to BUF_SIZE
        while ( (bytesRead = read(serverSocket, receiveLine, BUF_SIZE)) > 0) {
            receiveLine[bytesRead] = 0; // Make sure we put the null terminator at the end of the buffer
            printf("Received %d bytes from server with message: %s\n", bytesRead, receiveLine);

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

