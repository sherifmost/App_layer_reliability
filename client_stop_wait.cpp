// Implements the UDP client for stop and wait
// Based on the documentation for socket programming
// To call it use the command ./my_client
#include "client.h"

// The port to which we connect
string port_number = DEFAULT_PORT;
// The IP address of the server.
string server_ip;
// The path to the requested file at the server
string file_name;

// The main loop is here
int main(int argc, char **argv)
{

    // Read the arguments from the client.in file, ordered as:
    /*
        IP address of server.
        Well-known port number of server.
        Filename to be transferred (should be a large file).
    */
    // Read the file and split it to an argument per line
    vector<string> args = split_to_words(read_file_bin(INPUT_PATH), LINE_FEED);
    server_ip = args[0];
    // Make the addition of server port optional
    if (args.size() == 2)
    {
        file_name = args[1];
    }
    else
    {
        port_number = args[1];
        file_name = args[2];
    }
    // Create a UDP socket.
    int sockfd = open_socket();
    // Keep sending the file request till an acknowledgement is received.
    // TODO: setup the timer functionality
    request_file(sockfd);
    while(1);
    // Connecting to the server
    // int server_fd = connect_to_server(server_ip, port_number);
    // obtaining all the requests from the file to send and receive with the server
    // string requests_all = read_file_bin(REQUESTS_PATH);
    // sending each request and receiving the response as specified by the pseudo code
    // finished all requests so close the connection
    // cout << "client shutting down" << endl;
    // close(server_fd);
    // return 0;
}

int open_socket()
{
    // The file descriptor that is to be returned.
    int sockfd;
    // Creating socket file descriptor for a UDP socket using IPv4 protocol
    // AF_INET: IPv4, SOCK_DGRAM: UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Failed to create a socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void request_file(int sockfd)
{
    // Build a packet to request the file
    data_packet file_request;
    file_request.seqno = 0;
    file_request.chksum = 0;
    file_request.len = sizeof(file_request.chksum) + sizeof(file_request.len) + sizeof(file_request.seqno) + file_name.length();
    fill_buffer(file_name, file_request.data, 0, file_name.length());
    // Serialize this packet to be able to send it
    char packet_buf[file_request.len];
    data_packet_to_string(file_request, packet_buf);
    // To fill server address information
    struct addrinfo hints, *servaddr;
    int error_get_addr;

    // initializing hints as specified in the requirements:
    // Local host(my IP),IP V4, UDP
    memset(&hints, 0, sizeof hints);
    // IP V4
    hints.ai_family = AF_INET;
    // Unreliable stream (UDP)
    hints.ai_socktype = SOCK_DGRAM;

    // Obtaining the server address info
    if ((error_get_addr = getaddrinfo(server_ip.c_str(), port_number.c_str(), &hints, &servaddr)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error_get_addr));
        exit(EXIT_FAILURE);
    }
    // send the file request
    if (sendto(sockfd, packet_buf, file_request.len,
                                0, servaddr->ai_addr,
                                servaddr->ai_addrlen) < 0)
    {
        fprintf(stderr, "Couldn't send the request\n");
        exit(EXIT_FAILURE);
    }
    cout << "request to file " << file_name << " was sent successfully." << endl;
}