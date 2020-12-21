// Implements the UDP server with stop and wait functionality
// To call it use the command ./my_server_stop_wait
// Based on the tutorial for socket programming provided by Beejy
#include "server.h"

// The variables used
string port_number = DEFAULT_PORT;
int random_seed = 1;
float probability = 0.0;

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
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    string file_name = receive_file_request(sockfd, &client_addr);









    // // In case a port number was specified we initialize it
    // // Server is called by ./my_server port_number
    // if (argc > 1)
    // {
    //     port_number = argv[1];
    // }
    // cout << "Server running on port " << port_number << endl;
    // // file descriptor of the socket at which this server is listening
    // int listen_fd = get_server_fd(port_number);
    // if (listen_fd == -1)
    // {
    //     fprintf(stderr, "error occured in obtainnig the server fd.\n");
    //     exit(1);
    // }
    // // initialize first the signal handler to reap te zombie processes
    reap_zombies();
    // cout << "Server started listening on localhost at port number " << port_number << endl;
    // cout << "waiting for connections...." << endl;
    // handle_connections(listen_fd);
    return 0;
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
    
    return sockfd;
}
// Handles incoming file request from client and fills his address
string receive_file_request(int sockfd, struct sockaddr_in *client_addr)
{
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
    socklen_t len = sizeof(client_addr);
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

// Create a socket and obtain the file descriptor of the socket////////////////////////////////////////
int temp()
{
    // Initialized with -1 to indicate errors.
    int listen_sockfd = -1;
    // The address info structs
    // hints: the hints provided to obtain the address info of the server
    struct addrinfo hints, *servinfo, *it;
    int error_get_addr;
    // for the workaround
    int yes = 1;
    // initializing hints as specified in the requirements:
    // Local host(my IP),IP V4, TCP
    memset(&hints, 0, sizeof hints);
    // IP V4
    hints.ai_family = AF_INET;
    // Reliable stream (TCP)
    hints.ai_socktype = SOCK_STREAM;
    // Use my IP (Local host)
    hints.ai_flags = AI_PASSIVE;

    // Obtaining the server address info
    if ((error_get_addr = getaddrinfo(NULL, port_number.c_str(), &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error_get_addr));
        return -1;
    }
    // loop through all the results and bind to the first socket we can
    for (it = servinfo; it != NULL; it = it->ai_next)
    {
        // Try obtaining the socket and returning its fd
        // returns -1 for error
        if ((listen_sockfd = socket(it->ai_family, it->ai_socktype,
                                    it->ai_protocol)) == -1)
        {
            // prints a message corresponding to value of errno
            perror("In calling function get_server_fd, server: socket");
            continue;
        }
        // A workaround described by Beejy to overcome the case where address is returned to be in use
        if (setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("in calling function get_server_fd, setsockopt");
            // The program exits as port is used
            exit(1);
        }
        // Trying to bind the socket to the port
        if (bind(listen_sockfd, it->ai_addr, it->ai_addrlen) == -1)
        {
            close(listen_sockfd);
            perror("in calling function get_server_fd, server: bind");
            // Try the next socket
            continue;
        }
        // If suitable socket found break
        break;
    }
    // Freeing the structure as it is not needed any more
    freeaddrinfo(servinfo);
    // If no suitable socket was found
    if (it == NULL)
    {
        fprintf(stderr, "server: failed to find a suitable socket to bind\n");
        exit(1);
    }
    // Try listening on this socket
    if (listen(listen_sockfd, BACKLOG) == -1)
    {
        perror("In calling function get_server_fd, listen");
        exit(1);
    }
    return listen_sockfd;
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

// Non-persistent at first then would be updated to be persistent by use of select()
void handle_connections(int listen_fd)
{
    // The fd of the client trying to connect
    int client_fd;
    // keeps the client's address information
    struct sockaddr_storage client_addr;
    socklen_t sin_size = sizeof client_addr;
    while (1)
    {
        // Listener (parent) process would accept a connection if one exists
        // Then it would delegate the hanlding to a child process
        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (client_fd == -1)
        {
            perror("In handle_connections, accept");
            continue;
        }
        // To obtain the correct byte ordering (endian)
        // consider IPV4 only
        char s[INET_ADDRSTRLEN];
        if (client_addr.ss_family == AF_INET)
        {
            inet_ntop(AF_INET, &(((struct sockaddr_in *)&client_addr)->sin_addr),
                      s, sizeof s);
            cout << "Accepted connection from: " << s << endl;
        }
        else
        {
            cout << "server supports only IPV4" << endl;
            continue;
        }
        // Creating a child process to delegate the work with the accepted connection to it
        if (!fork())
        { // this is the child worker process
            // child doesn't need the listener
            close(listen_fd);
            // Receiving the client request to process it
            int num_received;
            char buf[MAX_DATA_SIZE];
            if ((num_received = recv(client_fd, buf, MAX_DATA_SIZE - 1, 0)) == -1)
            {
                perror("recv");
                exit(1);
            }
            // assuring that the buffer ends with a null character
            buf[num_received] = '\0';
            // parsing the received request to handle it
            // get and post requests are handled
            string request_string = string(buf);
            cout << "received request from " << s << " :" << endl
                 << request_string << endl;
            // handling the request
            // http_request request = string_to_request(request_string);
            // handle_request(request, client_fd);
            close(client_fd);
            // Exit the child process for now
            exit(0);
        }
        // parent doesn't need this for now connection is non-persistent
        close(client_fd);
    }
}

// To simulate packet drop in a random manner
bool to_be_sent()
{
    float rand_num = (float)rand() / RAND_MAX;
    return rand_num > probability;
}