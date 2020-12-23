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
// The includes needed for multiprocessing
#include <sys/wait.h>
#include <signal.h>
// chosen a random initial port larger than 1024 to prevent any required privelege constraints
#define DEFAULT_PORT "3000"
// Path to the input file to obtain the arguments
#define INPUT_PATH "client.in"
// path to keep the requested file
#define CLIENT_PATH "client/"

// Functions implemented

// returns the file descriptor resulting from creating this socket
int open_socket();
// Requests a file from a server given the socket file descriptor
void request_file(int sockfd);
// A handler for children processes that just clears zombie processes without blocking.
void sigchild_handler(int s);
// Sets the signal child handler to reap zombie children
void reap_zombies();
// Handle a response from the server for the received packet returns false if the received was a fin message
bool handle_response(int sockfd);
// Update the expected packet seqno
void update_expected_no();
// send an ack packet with given ackno
void send_ack(int sockfd,int ackno);// Defines the necessary functions and libraries needed for
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
// The includes needed for multiprocessing
#include <sys/wait.h>
#include <signal.h>
// chosen a random initial port larger than 1024 to prevent any required privelege constraints
#define DEFAULT_PORT "3000"
// Path to the input file to obtain the arguments
#define INPUT_PATH "client.in"
// path to keep the requested file
#define CLIENT_PATH "client/"

// Functions implemented

// returns the file descriptor resulting from creating this socket
int open_socket();
// Requests a file from a server given the socket file descriptor
void request_file(int sockfd);
// A handler for children processes that just clears zombie processes without blocking.
void sigchild_handler(int s);
// Sets the signal child handler to reap zombie children
void reap_zombies();
// Handle a response from the server for the received packet returns false if the received was a fin message
bool handle_response(int sockfd);
// Update the expected packet seqno
void update_expected_no();
// send an ack packet with given ackno
void send_ack(int sockfd,int ackno);