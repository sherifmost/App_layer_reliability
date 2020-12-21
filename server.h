// Defines the necessary functions and libraries needed for 
// implementing the multithreaded server.
#include "parser.h"
#include "file.h"

#include <unistd.h>
#include <errno.h>
// The includes needed for socket programming
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// The includes needed for multiprocessing
#include <sys/wait.h>
#include <signal.h>
// chosen a random initial port larger than 1024 to prevent any required privelege constraints
#define DEFAULT_PORT "3000"
// file used to enter the arguments
#define INPUT_PATH "server.in"
#define BACKLOG 100 // how many pending connections queue will hold

// Functions implemented

// A handler for children processes that just clears zombie processes without blocking.
void sigchild_handler(int s);
// A random function that returns whether or not to send a packet
bool to_be_sent();
// Returns the socket file descriptor of the UDP socket which the server created
int open_socket();
// Handles file request received from client and returns the file path also fills the client address 
string receive_file_request(int sockfd,struct sockaddr_in *client_addr);
// Sets the signal child handler to reap zombie children
void reap_zombies();
// The main infinite loop where the server is handling the incoming connections and requests
void handle_connections(int listen_fd);
// obtains the timeout value and changes it according to number of active connections
float change_timeout(int current_timeout,int number_connections);