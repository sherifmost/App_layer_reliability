#include "udp.h"
// This function takes a request string and splits it into components (line + headers + data)
vector<string> split_to_lines(string request);
// This function takes the string and splits it into words
vector<string> split_to_words(string line,char delim);
// This function changes a data packet to a string
void data_packet_to_string(data_packet packet,char* string);
// This function changes an ack packet to a string
void ack_packet_to_string(ack_packet packet,char* string);
// This function changes a string to a data packet
data_packet string_to_data_packet(char* packet);
// This function changes a string to an ack packet
ack_packet string_to_ack_packet(char* packet);
// gets the number of bytes of data
int num_data_bytes(data_packet packet);
// fills a character pointer with a string according to the given length and beginning
void fill_buffer(string data, char* buffer,int start,int length);