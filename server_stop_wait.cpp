// Implements the UDP server with stop and wait functionality
// To call it use the command ./my_server_stop_wait
// Based on the tutorial for socket programming provided by Beejy
#include "server_stop_wait.h"
// Timeout in ms for sending the file request
#define TIMEOUT 150

// The variables used
string port_number = DEFAULT_PORT;
int random_seed = 1;
float probability = 0.0;
uint32_t curr_seqno = 0;
// For obtaining the client's address
struct sockaddr_in *client_addr;
socklen_t len = sizeof(client_addr);


// The main function for running the server
// Should contain an infinite loop where the listener keeps
// listening to the incoming packets
int main(int argc, char **argv)
{
    // Read the arguments from the client.in file, ordered as:
    /*
        Port number of server.
        Random generator seed value..
        Probability p of datagram loss (real number in the range [ 0.0 , 1.0].
    */
    // Read the file and split it to an argument per line
    vector<string> args = split_to_words(read_file_bin(INPUT_PATH), LINE_FEED);
    // Make the addition of server port optional
    if (args.size() != 0 && args[0].size() != 0)
    {
        port_number = args[0];
    }
    // Fill in the random generator seed and the loss probability if they are given
    if (args.size() > 1)
    {
        random_seed = stoi(args[1]);
        probability = stof(args[2]);
    }
    // initialize with the given random seed
    srand(random_seed);
    // Creating a UDP socket
    int sockfd = open_socket();
    // Receive the requested file from the client and get his address info
    memset(&client_addr, 0, sizeof(client_addr));
    string file_name = receive_file_request(sockfd);
    // Record start time
    auto start = std::chrono::high_resolution_clock::now();
    // close the socket and end if the file doesn't exist
    if (!file_exists(file_name))
    {
        cout << "file doesn't exist, shutting down..." << endl;
        send_fin(sockfd);
        close(sockfd);
        exit(0);
    }
    // Start a timeout for the socket
    set_timeout(sockfd);
    // start the main operation of sending UDP datagrams using the stop and wait technique
    // first, read the file to be sent
    string file_contents = read_file_bin(file_name);
    // Checks if a timeout occurs
    bool timeout = false;
    // A buffer to send the packet
    char packet_buf[MAX_LEN];
    // The packet being sent
    data_packet packet;
    // to detect whether we finished sending the file
    bool finished = false;
    // To determine where to start from when sending the next datagram
    int file_start = 0;
    // The main operation to be done
    while (1)
    {
        // in case we didn't timeout get next packet
        if (!timeout)
        {
            // get length of the data to be sent
            int length = min(MAX_DATA_SIZE, (int)file_contents.length() - file_start);
            packet = create_next_datagram(file_start,length, file_contents, packet_buf);
            // update the start of the file
            file_start += length;
            // Check whether we read the whole file
            if (file_start == (int)file_contents.length())
            {
                finished = true;
            }
        }
        else
        {
            cout << "timeout for packet with seqno " << packet.seqno << " retransmitting.." << endl;
        }
        // send or resend the current packet
        send_datagram(sockfd, packet, packet_buf);
        int num_rec = handle_response(sockfd);
        // Check whether or not a timeout occured
        if (num_rec < 0)
        {
            timeout = true;
        }
        else
        {
            timeout = false;
        }
        // in case we finished send a fin meessage and shut down
        if (finished && !timeout)
        {
            cout << "finished sending the file, shutting down..." << endl;
            send_fin(sockfd);
            close(sockfd);
            auto finish = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = finish - start;
            std::cout << "Elapsed time in seconds: " << elapsed.count() << " s\n";
            exit(0);
        }
        
    }
    return 0;
}

int open_socket()
{
    int sockfd;
    // for the workaround
    int yes = 1;
    // Creating socket file descriptor
    // AF_INET: IPv4, SOCK_DGRAM: UDP.
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    // A workaround described by Beejy to overcome the case where address is returned to be in use
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int)) == -1)
    {
        perror("in calling function open_socket, setsockopt as port is used");
        // The program exits as port is used
        exit(1);
    }
    struct addrinfo hints, *server_addr;
    int error_get_addr;
    // initializing hints as specified in the requirements:
    // IP V4, UDP
    memset(&hints, 0, sizeof hints);
    // IP V4
    hints.ai_family = AF_INET;
    // Unreliable stream (UDP)
    hints.ai_socktype = SOCK_DGRAM;
    // Use my IP (Local host)
    hints.ai_flags = AI_PASSIVE;
    // Obtaining the server address info
    if ((error_get_addr = getaddrinfo(NULL, port_number.c_str(), &hints, &server_addr)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error_get_addr));
        exit(EXIT_FAILURE);
    }
    // Bind the socket with the server address
    if (bind(sockfd, server_addr->ai_addr,
             server_addr->ai_addrlen) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Handles incoming file request from client and fills his address
string receive_file_request(int sockfd)
{
    // receive the request in a buffer
    char buffer[MAX_LEN];
    int num_rec = recvfrom(sockfd, (char *)buffer, MAX_LEN,
                           0, (struct sockaddr *)&client_addr,
                           &len);
    if (num_rec < 0)
    {
        perror("error in receiving the message.");
        exit(EXIT_FAILURE);
    }
    // copy the meaningfull part and parse it
    char packet[num_rec];
    memcpy(packet, buffer, num_rec);
    data_packet file_request = string_to_data_packet(packet);
    cout << "Request to file " << file_request.data << " received, seqno: " << file_request.seqno << endl;
    return file_request.data;
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

// To simulate packet drop in a random manner
bool to_be_sent()
{
    float rand_num = (float)rand() / RAND_MAX;
    return rand_num > probability;
}

void update_seqno()
{
    curr_seqno = 1 - curr_seqno;
}

void send_datagram(int sockfd, data_packet packet, char *packet_buf)
{

    // check whether we should drop the packet or not
    if (to_be_sent())
    {
        cout << "file content sent with length " << packet.len << " and seqno " << packet.seqno << endl;
        if (sendto(sockfd, packet_buf, packet.len,
                   0, (struct sockaddr *)&client_addr,
                   len) < 0)
        {
            fprintf(stderr, "Couldn't send the request\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // drop the packet by not sending it
        cout << "packet dropped seqno " << packet.seqno << endl;
    }
}

int handle_response(int sockfd)
{
    // receive the ack in a buffer
    char buffer[MAX_LEN];
    int num_rec = recvfrom(sockfd, (char *)buffer, MAX_LEN,
                           0, (struct sockaddr *)&client_addr,
                           &len);
    // This would indicate a timeout
    if (num_rec <= 0)
        return num_rec;
    // copy the meaningfull part and parse it
    char packet_buf[num_rec];
    memcpy(packet_buf, buffer, num_rec);
    ack_packet packet = string_to_ack_packet(packet_buf);
    // check whether this is a duplicate ack (NAK), if so do nothing else update the seqno and kill the child
    if (packet.ackno != curr_seqno)
    {
        cout << "Duplicate ack received for seqno " << packet.ackno << endl;
        return -1;
    }
    else
    {
        cout << "Ack received for packet with seqno " << packet.ackno << endl;
        // go to the next state
        update_seqno();
    }
    return num_rec;
}

data_packet create_next_datagram(int file_start,int length, string total_data, char *packet_buf)
{
    // create the datagram to be sent
    data_packet packet;
    packet.seqno = curr_seqno;
    packet.chksum = 0;
    packet.len = length + 8;
    // copy content to the packet buffer
    fill_buffer(total_data, packet.data, file_start, length);
    // Serialize this packet to be able to send it
    data_packet_to_string(packet, packet_buf);
    return packet;
}

void set_timeout(int sockfd)
{
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT / 1000;
    timeout.tv_usec = (TIMEOUT % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0)
        perror("setsockopt timeout failed\n");
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
               0, (struct sockaddr *)&client_addr,
               len) < 0)
    {
        fprintf(stderr, "Couldn't send the request\n");
        exit(EXIT_FAILURE);
    }
    cout << "Request to finish was sent to client" << endl;
}