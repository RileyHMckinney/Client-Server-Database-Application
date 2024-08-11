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


#include "msg.h"  //for proper message structuring

void Usage(char *progname);
int Listen(char *portnum, int *sock_family);
void *HandleClient(void *client_fd_ptr);

int main(int argc, char **argv) {
    if (argc != 2) {
        Usage(argv[0]);
    }

    int sock_family;
    int listen_fd = Listen(argv[1], &sock_family);
    if (listen_fd <= 0) {
        // Failed to bind/listen to a socket. Quit with failure.
        printf("Couldn't bind to any addresses.\n");
        return EXIT_FAILURE;
    }

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd < 0) {
            if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
                continue;
            printf("Failure on accept: %s\n", strerror(errno));
            break;
        }

        // Allocate memory to store client_fd (pass it to the thread)
        int *client_fd_ptr = malloc(sizeof(int));
        if (client_fd_ptr == NULL) {
            perror("Unable to allocate memory for client file descriptor");
            close(client_fd);
            continue;
        }
        *client_fd_ptr = client_fd;

        // Create a new thread to handle the client
        pthread_t tid;
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

void Usage(char *progname) {
    printf("usage: %s port\n", progname);
    exit(EXIT_FAILURE);
}

int Listen(char *portnum, int *sock_family) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;          // Use IPv4
    hints.ai_socktype = SOCK_STREAM;    // Stream socket
    hints.ai_flags = AI_PASSIVE;        // Use wildcard address
    hints.ai_protocol = IPPROTO_TCP;    // TCP protocol

    int res = getaddrinfo(NULL, portnum, &hints, &result);
    if (res != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(res));
        return -1;
    }

    int listen_fd = -1;
    for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
        listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listen_fd == -1) {
            continue;
        }

        int optval = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            *sock_family = rp->ai_family;
            break;
        }

        close(listen_fd);
        listen_fd = -1;
    }

    freeaddrinfo(result);

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
    //cast the client_socket_ptr parameter into an integer pointer type
    int socket_fd = *(int *)client_socket_ptr;
    // Release dynamically allocated memory
    free(client_socket_ptr);

    struct msg client_msg; // Incoming message from the client
    struct msg server_response; // Response message to be sent back
    int database_fd; // File descriptor for the database

    while (1) {
        // Receive a message from the client
        ssize_t bytes_received = read(socket_fd, &client_msg, sizeof(client_msg));
        
        //if no msg received, break loop
        if (bytes_received <= 0) {
            printf("[Client disconnected or encountered an error.]\n");
            break;
        }

        // Initialize the server response
        memset(&server_response, 0, sizeof(server_response));

        // Handle different types of client requests
        switch (client_msg.type) {
            //
            case PUT:
                // Append the incoming record to the database file
                //  -if the file database.db does not exist, create one and open with write-only permissions
                database_fd = open("database.db", O_WRONLY | O_APPEND | O_CREAT, 0644);
                if (database_fd == -1) {
                    perror("Error opening the database file");
                    server_response.type = FAIL;
                } else {
                    if (write(database_fd, &client_msg.rd, sizeof(client_msg.rd)) == sizeof(client_msg.rd)) {
                        server_response.type = SUCCESS;
                    } else {
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
                server_response.type = FAIL; // Unknown message type
                break;
        }

        // Send the constructed response back to the client
        write(socket_fd, &server_response, sizeof(server_response));
    }

    // Properly close the client's socket connection
    close(socket_fd);
    return NULL;
}

