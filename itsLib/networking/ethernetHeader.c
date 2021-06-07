#include <net/ethernet.h>
#include <arpa/inet.h> 

#include "ethernetHeader.h"

void fillEthernetHeader(struct ethhdr * eth, const socketInfo * info) {
    eth->h_source[0] = (unsigned char)(info->ifreq_c.ifr_hwaddr.sa_data[0]);
    eth->h_source[1] = (unsigned char)(info->ifreq_c.ifr_hwaddr.sa_data[1]);
    eth->h_source[2] = (unsigned char)(info->ifreq_c.ifr_hwaddr.sa_data[2]);
    eth->h_source[3] = (unsigned char)(info->ifreq_c.ifr_hwaddr.sa_data[3]);
    eth->h_source[4] = (unsigned char)(info->ifreq_c.ifr_hwaddr.sa_data[4]);
    eth->h_source[5] = (unsigned char)(info->ifreq_c.ifr_hwaddr.sa_data[5]);

    // Send message as broadcast
    for (int i = 0; i < 6; i++)
    {
        eth->h_dest[i] = 0xff;
    }

    // Means next header will be GeoNetworking
    eth->h_proto = htons(0x8947);
}
