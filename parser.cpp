#include "parser.h"
// For http parsing
vector<string> split_to_lines(string request)
{
    int request_length = request.length();
    // We split by finding cr and lf
    // finding 2 cr lf after each other corresponds to the empty line
    int string_begining = 0;
    vector<string> result;
    for (int i = 1; i < request_length; i++)
    {
        // The case where a line feed and a carriage return was obtained
        if (request[i - 1] == CARRIAGE_RETURN && request[i] == LINE_FEED)
        {
            // The case where they represent the empty line
            if (string_begining == i - 1)
            {
                result.push_back(EMPTY_LINE);
                result.push_back(request.substr(i + 1, request_length - i));
                break;
            }
            // other wise push the line and advance the beggining
            result.push_back(request.substr(string_begining, i - string_begining - 1));
            string_begining = i + 1;
        }
    }
    return result;
}
vector<string> split_to_words(string line, char delim)
{
    int length = line.length();
    // We split by finding SPACE
    int string_begining = 0;
    vector<string> result;
    for (int i = 1; i < length; i++)
    {
        // The case where a line feed and a carriage return was obtained
        if (line[i] == delim)
        {
            // push the word and advance the beggining
            result.push_back(line.substr(string_begining, i - string_begining));
            string_begining = i + 1;
        }
    }
    result.push_back(line.substr(string_begining, length - string_begining));
    return result;
}
// for converting between sent format and stored format (serialization functions)
void data_packet_to_string(data_packet packet, char *result)
{
    // converting checksum into bytes
    memcpy(result, &packet.chksum, sizeof(packet.chksum));
    // converting len into bytes
    memcpy(result + sizeof(packet.chksum), &packet.len, sizeof(packet.len));
    // converting seqno into bytes
    memcpy(result + sizeof(packet.chksum) + sizeof(packet.len), &packet.seqno, sizeof(packet.seqno));
    // copying the data bytes
    memcpy(result + sizeof(packet.chksum) + sizeof(packet.len) + sizeof(packet.seqno), &packet.data, num_data_bytes(packet));
}
void ack_packet_to_string(ack_packet packet, char *result)
{
    // converting checksum into bytes
    memcpy(result, &packet.chksum, sizeof(packet.chksum));
    // converting len into bytes
    memcpy(result + sizeof(packet.chksum), &packet.len, sizeof(packet.len));
    // converting seqno into bytes
    memcpy(result + sizeof(packet.chksum) + sizeof(packet.len), &packet.ackno, sizeof(packet.ackno));
}
data_packet string_to_data_packet(char* packet){
    data_packet result;
    // obtaining checksum
    memcpy(&result.chksum, packet, sizeof(result.chksum));
    // obtaining len
    memcpy(&result.len, packet + sizeof(result.chksum), sizeof(result.len));
    // obtaining seq number
    memcpy(&result.seqno, packet + sizeof(result.chksum) + sizeof(result.len), sizeof(result.len));
    // obtaining the data
    memcpy(&result.data, packet + sizeof(result.chksum) + sizeof(result.len) + sizeof(result.seqno), num_data_bytes(result));
    // Add a null character for convinience
    result.data[num_data_bytes(result)] = STRING_END;
    return result;
}
ack_packet string_to_ack_packet(char* packet){
    ack_packet result;
    // obtaining checksum
    memcpy(&result.chksum, packet, sizeof(result.chksum));
    // obtaining len
    memcpy(&result.len, packet + sizeof(result.chksum), sizeof(result.len));
    // obtaining seq number
    memcpy(&result.ackno, packet + sizeof(result.chksum) + sizeof(result.len), sizeof(result.len));
    return result;
}
// returns number of data bytes present in a packet
int num_data_bytes(data_packet packet)
{
    return packet.len - (sizeof(packet.chksum) + sizeof(packet.len) + sizeof(packet.seqno));
}
// fills a buffer with data from a string without terminating with null
void fill_buffer(string data, char *buffer, int start,int length)
{
    for (int i = start; i < length + start; i++)
    {
        buffer[i - start] = data[i];
    }
}