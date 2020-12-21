#include "common.h"
// The maximum data size obtainable in a packet to make the packet has a max lenght of 512 (in bytes) 
#define MAX_DATA_SIZE 504
// Maximum values for range checkings
#define MAX_CHKSUM 65535
#define MAX_LEN 512
#define MAX_SEQNO 4294967295
#define MAX_ACKNO 4294967295  
// structures representing the packets
// Data only packet
typedef struct
{
    /* Header */
    uint16_t chksum; // May be used in the future for bonus
    uint16_t len; // Total length of the packet
    uint32_t seqno; // The sequence number of the packet for ordering and preventing duplications
    /* Data part */
    char data[MAX_DATA_SIZE + 1];
} data_packet;
// Ack only packet (8 bytes)
typedef struct
{
    /* Header */
    uint16_t chksum; // May be used in the future for bonus
    uint16_t len; // Total length of the packet
    // In case of stop and wait ack is for the acknowledged packet.
    // In case of selective repeat, it has the seqno of expected packet to be received next.
    uint32_t ackno; // The acknowledge number of the acknowledged packet for preventing duplications or loss
} ack_packet;