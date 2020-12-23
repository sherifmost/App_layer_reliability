// Implements the UDP client for stop and wait
// Based on the documentation for socket programming
// To call it use the command ./my_client
#include "client.h"
// Timeout in ms for sending the file request (defined to be a second)
#define TIMEOUT 1000

// The port to which we connect
string port_number = DEFAULT_PORT;
// The IP address of the server.
string server_ip;
// The path to the requested file at the server
string file_name;
// To fill server address information
struct addrinfo *servaddr;
// The current expected packet seqno
uint32_t expected_no = 1;
// defining the timeout time in micro seconds
int timeout = 1000 * TIMEOUT;
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
    // Reap zombie processes
    reap_zombies();
    // Create a UDP socket.
    int sockfd = open_socket();
    // Keep sending the file request till an acknowledgement is received.
    // Creating a child process to delegate the sending to for timer functionality
    int requester_pid = fork();
    if (requester_pid == 0)
    {
        while (1)
        {
            request_file(sockfd);
            usleep(timeout);
            cout << "Request for file timed out, resending..." << endl;
        }
    }
    // Receive a response from the server and stop the process resending the request
    // when the first response is received
    bool first_response = true;
    while (1)
    {
        bool keep_con = handle_response(sockfd);
        if (first_response)
        {
            kill(requester_pid, SIGKILL);
            first_response = false;
        }
        if (!keep_con)
        {
            exit(0);
        }
    }
    return 0;
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
    int error_get_addr;
    // hints to obtain the address info
    struct addrinfo hints;
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

    return sockfd;
}

void request_file(int sockfd)
{
    // Build a packet to request the file
    data_packet file_request;
    // initialize with seqno max number so that when it is sent again the server understands it as a NAK
    file_request.seqno = MAX_SEQNO;
    file_request.chksum = 0;
    file_request.len = sizeof(file_request.chksum) + sizeof(file_request.len) + sizeof(file_request.seqno) + file_name.length();
    fill_buffer(file_name, file_request.data, 0, file_name.length());
    // Serialize this packet to be able to send it
    char packet_buf[file_request.len];
    data_packet_to_string(file_request, packet_buf);
    cout << "request to file " << file_name << " was sent successfully." << endl;
    // send the file request
    if (sendto(sockfd, packet_buf, file_request.len,
               0, servaddr->ai_addr,
               servaddr->ai_addrlen) < 0)
    {
        fprintf(stderr, "Couldn't send the request\n");
        exit(EXIT_FAILURE);
    }
}

// handle messages received from the server
bool handle_response(int sockfd)
{
    // receive the response in a buffer
    char buffer[MAX_LEN];
    int num_rec = recvfrom(sockfd, (char *)buffer, MAX_LEN,
                           0, servaddr->ai_addr,
                           &servaddr->ai_addrlen);
    if (num_rec < 0)
    {
        perror("error in receiving the message.");
        exit(EXIT_FAILURE);
    }
    // in case the received is a FIN packet (AKA Ack packet from the server) stop the connection and end the client
    if (num_rec == 8)
    {
        cout << "Closing the connection" << endl;
        close(sockfd);
        cout << "Client shutting down" << endl;
        return false;
    }
    // copy the meaningfull part and parse it
    char packet[num_rec];
    memcpy(packet, buffer, num_rec);
    data_packet file_packet = string_to_data_packet(packet);
    cout << "Received file data seqno: " << file_packet.seqno << endl;
    // Send an ack packet for this recieved packet
    send_ack(sockfd,expected_no);
    // Append the recieved file data to the file in order if it is the one expected
    if (file_packet.seqno == expected_no -1)
    {
        write_file(CLIENT_PATH + file_name, file_packet.data);
        update_expected_no();
    }
    return true;
} 

void update_expected_no()
{
    expected_no++;
}

// To send a fin packet (AKA Ack packet)
void send_fin(int sockfd)
{
    // Build a fin packet to request the file
    ack_packet fin;
    fin.ackno = 0;
    fin.chksum = 0;
    fin.len = 8;
    // Serialize this packet to be able to send it
    char packet_buf[fin.len];
    ack_packet_to_string(fin, packet_buf);
    // send the fin message
    if (sendto(sockfd, packet_buf, fin.len,
               0, servaddr->ai_addr,
               servaddr->ai_addrlen) < 0)
    {
        fprintf(stderr, "Couldn't send the request\n");
        exit(EXIT_FAILURE);
    }
    cout << "Response to finish was sent to server" << endl;
}
// send an ack packet with given ack number
void send_ack(int sockfd, int ackno)
{
    // create the ack packet
    ack_packet packet;
    packet.ackno = ackno;
    packet.chksum = 0;
    packet.len = 8;
    // serialize the ack packet
    char packet_buf[packet.len];
    ack_packet_to_string(packet, packet_buf);
    cout << "Ack to ackno " << ackno << " was sent successfully." << endl;
    // send the ack packet
    if (sendto(sockfd, packet_buf, packet.len,
               0, servaddr->ai_addr,
               servaddr->ai_addrlen) < 0)
    {
        fprintf(stderr, "Couldn't send the request\n");
        exit(EXIT_FAILURE);
    }
}
// A handler to handle zombie children
// n this way the parent wait for any child processes (pid = -1)
// and while there are zombie process (waitpid() return value is >0)
// it keep looping on calling wait.
void sigchild_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

void reap_zombies()
{
    struct sigaction s;
    // reap all dead processes using the implemented sigchild handler
    s.sa_handler = sigchild_handler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &s, NULL) == -1)
    {
        perror("when calling reap_zmbies, sigaction");
        exit(1);
    }
}