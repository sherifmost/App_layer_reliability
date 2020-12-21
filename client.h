// Defines the necessary functions and libraries needed for
// implementing the client.
#include "file.h"
#include "parser.h"
#include <unistd.h>
#include <errno.h>
// The includes needed for socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// chosen a random initial port larger than 1024 to prevent any required privelege constraints
#define DEFAULT_PORT "3000"
// defines the file at which the client input is located
#define INPUT_PATH "client.in"

// Functions implemented

// returns the file descriptor resulting from creating this socket
int open_socket();
// Requests a file from a server given the socket file descriptor
void request_file(int sockfd);