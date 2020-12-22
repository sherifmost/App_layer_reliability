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
// Sets the signal child handler to reap zombie children
void reap_zombies();
// A random function that returns whether or not to send a packet
bool to_be_sent();
// Returns the socket file descriptor of the UDP socket which the server created
int open_socket();
// Handles file request received from client and returns the file path also fills the client address 
string receive_file_request(int sockfd);
// Updates the sequence number for the next packet to be sent
void update_seqno();
// sends a packet indicating that the server is shutting down
void send_fin(int sockfd); 
// Creates the next datagram
data_packet create_next_datagram(int file_start,int length,string total_data,char* packet_buf);
// sends the next datagram according to the packet loss probability
void send_datagram(int sockfd, data_packet packet, char *packet_buf);
// receives the response from the client and reacts accrordingly, 
int handle_response(int sockfd); 
// Sets the timeout for the socket receive
void set_timeout(int sockfd);