//CS 3377 Project 3 
// Names: Jonathan Lee and Riley Mckinney
// NETIDs: jhl210002 and rhm220001

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <inttypes.h>

#include "msg.h"  //for proper message structuring

#define BUF 256

// function prototypes
void Usage(char *progname);

int LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen);

int Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd);

void PutSelect(int socket_fd);
void GetSelect(int socket_fd);

int 
main(int argc, char **argv) {
  // Check for the correct number of command line arguments being provided
  if (argc != 3) {
    Usage(argv[0]);
  }

  // Parse the port number from command line argument
  unsigned short port = 0;
  if (sscanf(argv[2], "%hu", &port) != 1) {
    Usage(argv[0]);
  }

  // Get an appropriate sockaddr structure.
  struct sockaddr_storage addr;
  size_t addrlen;
  if (!LookupName(argv[1], port, &addr, &addrlen)) {
    Usage(argv[0]);
  }

  // Connect to the remote host.
  int socket_fd;
  if (!Connect(&addr, addrlen, &socket_fd)) {
    Usage(argv[0]);
  }

 while(1){ //display menu using switch case
    printf("\nEnter your choice (1 to put, 2 to get, 0 to quit): ");
    int choice;
    scanf("%d", &choice);
    
    // Clear the input buffer
    while (fgetc(stdin) != '\n' && !feof(stdin));
    
    switch (choice) {
        case 1:         //call PutSelect() fucntion
            PutSelect(socket_fd);
            break;
        case 2:         //call GetSelect() function
            GetSelect(socket_fd);
            break;
        case 0:         //quit, and close socket
            close(socket_fd);
            return EXIT_SUCCESS;
        default:        //otherwise, loop
            break;
    }
 }
}

//for putting data into the db
void PutSelect(int socket_fd) {
    //create a msg struct to send to the server
    struct msg put_msg;
    char buffer[128]; 

    //set msg.type to PUT
    put_msg.type = PUT;

    //Prompt user for a name
    printf("Enter the name: ");
    
    
    // Use fgets() to read inputs with more than one word
    // Validates the user's input for name
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      // Remove the "\n" (new line) at the end
      buffer[strcspn(buffer, "\n")] = 0;
    }
    else {
      perror("Failed to save name input (fgets failed).");
      return;
    }
    

    //check if the name is too long to fit in the msg struct
    if(strlen(buffer) >= sizeof(put_msg.rd.name)) {
        printf("Invalid name input.\n");
        return;
    }
    strcpy(put_msg.rd.name, buffer); 

    //prompt the user for the id
    printf("Enter the id: ");
    
    if(scanf("%" PRIu32, &put_msg.rd.id) != 1) {
        printf("Invalid id input.\n");
        while (fgetc(stdin) != '\n'); 
        return;
    }
    
    // Clear the padding field
    memset(put_msg.rd.pad, 0, sizeof(put_msg.rd.pad)); 

    // Write the put_msg to the socket
    if (write(socket_fd, &put_msg, sizeof(put_msg)) != sizeof(put_msg)) {
        perror("Error writing message to server");
        return;
    }

    
    // Create a message struct to receive the server's response
    struct msg response;
    
    // Read the server response
    if (read(socket_fd, &response, sizeof(response)) != sizeof(response)) {
        perror("Error reading response from server");
        return;
    }
    
    if (response.type == SUCCESS) {
        printf("Put success.\n");
    } else {
        printf("Put failure.\n");
    }
    
    //clear buffer
    memset(buffer, 0, sizeof(buffer)); 
}


//for getting data from the db
void GetSelect(int socket_fd) {
    //create a msg struct to send to the server
    struct msg get_msg;
    
    //set msg.type to PUT
    get_msg.type = GET;
    
    //prompt the user for the id
    printf("Enter the id: ");
    
    if(scanf("%" PRIu32, &get_msg.rd.id) != 1){
        printf("Invalid id input.\n");
        return;
    }
    
    if (write(socket_fd, &get_msg, sizeof(get_msg)) != sizeof(get_msg)) {
        perror("Error writing message to server");
        return;
    }
    
    struct msg response;
    read(socket_fd, &response, sizeof(response));
    
    if (response.type == SUCCESS) {
        printf("Name: %s\nID: %" PRIu32 , response.rd.name, response.rd.id);
    } else {
        printf("Record not found.\n");
    }
    
}

// Print information surrounding the hostname port
void 
Usage(char *progname) {
  printf("usage: %s  hostname port \n", progname);
  exit(EXIT_FAILURE);
}

// Search for name in the msg.h's record array
int 
LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen) {
  struct addrinfo hints, *results;
  int retval;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // Do the lookup by invoking getaddrinfo().
  if ((retval = getaddrinfo(name, NULL, &hints, &results)) != 0) {
    printf( "getaddrinfo failed: %s", gai_strerror(retval));
    return 0;
  }

  // Set the port in the first result.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr =
            (struct sockaddr_in *) (results->ai_addr);
    v4addr->sin_port = htons(port);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr =
            (struct sockaddr_in6 *)(results->ai_addr);
    v6addr->sin6_port = htons(port);
  } else {
    printf("getaddrinfo failed to provide an IPv4 or IPv6 address \n");
    freeaddrinfo(results);
    return 0;
  }

  // Return the first result.
  assert(results != NULL);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_addrlen = results->ai_addrlen;

  // Clean up
  freeaddrinfo(results);
  return 1;
}

//Begin connecting the socket
int 
Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd) {
  // Create the socket.
  int socket_fd = socket(addr->ss_family, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("socket() failed: %s", strerror(errno));
    return 0;
  }

  // Connect the socket to the remote host.
  int res = connect(socket_fd,
                    (const struct sockaddr *)(addr),
                    addrlen);
  // Connection failed
  if (res == -1) {
    printf("connect() failed: %s", strerror(errno));
    return 0;
  }

  *ret_fd = socket_fd;
  return 1;
}