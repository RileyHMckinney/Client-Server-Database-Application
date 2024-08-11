//CS 3377 Project 3 
// Names: Jonathan Lee and Riley Mckinney
// NETIDs: jhl210002 and rhm220001

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>  // For O_WRONLY, O_APPEND, O_CREAT, O_RDONLY
#include "msg.h"   // For proper message structuring


// function prototypes
void Usage(char *progname);
int Listen(char *portnum, int *sock_family);
void *HandleClient(void *client_fd_ptr);

int main(int argc, char **argv) {
    if (argc != 2) {
        Usage(argv[0]);
    }
    
    // Start listening on the port number
    int sock_family;
    int listen_fd = Listen(argv[1], &sock_family);
    if (listen_fd <= 0) {
        // Failed to bind/listen to a socket. Quit with failure.
        printf("Couldn't bind to any addresses on port %s.\n", argv[1]);
        return EXIT_FAILURE;
    }

    // Open the database file and truncate it to zero length
    int db_fd = open("database.db", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (db_fd == -1) {
        perror("Failed to initialize the database file");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // keep repeating unless client_fd is invalid
    while (1) {
        struct sockaddr_storage client_addr; 
        socklen_t client_addr_len = sizeof(client_addr);
        // Accept the client connection
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        // Invalid client_fd
        if (client_fd < 0) {
            // Error handling when accepting client connections
            if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
                continue;
            printf("Failure on accept: %s\n", strerror(errno));
            break;
        }

        // Allocate memory to store client_fd (pass it to the thread)
        int *client_fd_ptr = malloc(sizeof(int));
        // Check if memory allocation was successful
        if (client_fd_ptr == NULL) {
            perror("Unable to allocate memory for client file descriptor");
            close(client_fd);
            continue;
        }
        *client_fd_ptr = client_fd;

        // Create a new thread to handle the client
        pthread_t tid;
        // Check if the thread creation was successful
        if (pthread_create(&tid, NULL, HandleClient, client_fd_ptr) != 0) {
            perror("Failed to create thread");
            close(client_fd);
            free(client_fd_ptr);
            continue;
        }

        // Detach the thread, allowing it to clean up after itself
        pthread_detach(tid);
    }

    // Close socket
    close(listen_fd);
    return EXIT_SUCCESS;
}

// Prints out the usage port number
void Usage(char *progname) {
    printf("usage: %s port\n", progname);
    exit(EXIT_FAILURE);
}

// Carries out the listening command
int Listen(char *portnum, int *sock_family) {
    // Initializes the address information hints 
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;          // Use IPv4
    hints.ai_socktype = SOCK_STREAM;    // Stream socket
    hints.ai_flags = AI_PASSIVE;        // Use wildcard address
    hints.ai_protocol = IPPROTO_TCP;    // TCP protocol

    // Gets address information of the port number given
    int res = getaddrinfo(NULL, portnum, &hints, &result);
    
    // Checks if getaddrinfo was successful
    if (res != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(res));
        return -1;
    }

    // Initially set listen_fd to be -1
    int listen_fd = -1;
    // Go through the available address info
    for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
        // create the socket 
        listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        // if socket creation fails continu
        if (listen_fd == -1) {
            continue;
        }

        int optval = 1;
        // Allow the socket to reuse the address
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
      
        // Bind the socket to the address
        if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            *sock_family = rp->ai_family;
            break;
        }

        close(listen_fd);
        listen_fd = -1;
    }

    freeaddrinfo(result);

    // Check if the bind and listening of the socket is correctly functioning 
    if (listen_fd == -1) {
        return listen_fd;
    }
  
    if (listen(listen_fd, SOMAXCONN) != 0) {
        printf("Failed to mark socket as listening: %s\n", strerror(errno));
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

// Thread function responsible for processing client requests
void *HandleClient(void *client_socket_ptr) {
    // Cast the client_socket_ptr parameter into an integer pointer type
    int socket_fd = *(int *)client_socket_ptr;
    // Release dynamically allocated memory
    free(client_socket_ptr);

    struct msg client_msg;       // Incoming message from the client
    struct msg server_response;  // Response message to be sent back
    int database_fd;             // File descriptor for the database

    while (1) {
        // Receive a message from the client
        ssize_t bytes_received = read(socket_fd, &client_msg, sizeof(client_msg));

        // If no message is received, break the loop
        if (bytes_received <= 0) {
            printf("[Client disconnected or encountered an error.]\n");
            break;
        }

        // Initialize the server response
        memset(&server_response, 0, sizeof(server_response));

        // Handle different types of client requests
        switch (client_msg.type) {
            case PUT:
                // Append the incoming record to the database file
                // If the file database.db does not exist, create it and open with write-only permissions
                database_fd = open("database.db", O_WRONLY | O_APPEND | O_CREAT, 0644);
                if (database_fd == -1) {
                    perror("Error opening the database file");
                    server_response.type = FAIL;
                    close(database_fd);
                } else {
                    ssize_t bytes_written = write(database_fd, &client_msg.rd, sizeof(client_msg.rd));
                    if (bytes_written == sizeof(client_msg.rd)) {
                        server_response.type = SUCCESS;
                    } else {
                        perror("Error writing record to database");
                        server_response.type = FAIL;
                    }
                    close(database_fd);
                }
                break;

            case GET:
                // Retrieve the specified record based on the provided ID
                database_fd = open("database.db", O_RDONLY);
                if (database_fd == -1) {
                    perror("Error accessing the database file");
                    server_response.type = FAIL;
                    close(database_fd);
                } else {
                    struct record record_buffer;
                    int record_found = 0;

                    // Scan through all records to find the matching one
                    while (read(database_fd, &record_buffer, sizeof(record_buffer)) == sizeof(record_buffer)) {
                        if (record_buffer.id == client_msg.rd.id) {
                            record_found = 1;
                            server_response.type = SUCCESS;
                            memcpy(&server_response.rd, &record_buffer, sizeof(record_buffer));
                            break;
                        }
                    }

                    if (!record_found) {
                        server_response.type = FAIL;
                    }
                    close(database_fd);
                }
                break;

            default:
                server_response.type = FAIL;  // Unknown message type
                break;
        }

        // Send the constructed response back to the client
        ssize_t bytes_written = write(socket_fd, &server_response, sizeof(server_response));
        if (bytes_written != sizeof(server_response)) {
            perror("Error sending response to client");
        }
    }


    // Properly close the client's socket connection
    close(socket_fd);
    return NULL;
}
