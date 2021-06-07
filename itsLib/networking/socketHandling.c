#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <string.h>

#include "./socketHandling.h"

/**
 * Open a socket for the network interface with name ifName
 * 
 * Returns:
 * - the socket file descriptor
 * - the index of the interface
 * - the MAC address of the interface
 */
socketInfo * prepareSocket(const char * ifName) {
    socketInfo * info = calloc(1, sizeof(socketInfo));

    // Open socket
    info->sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (info->sockfd == -1)
    {
        perror("error while creating socket");
        return info;
    }

    // Get the index
    memset(&info->ifreq_i, 0, sizeof(info->ifreq_i));
    strncpy(info->ifreq_i.ifr_name, ifName, IFNAMSIZ - 1); // giving name of Interface
    if ((ioctl(info->sockfd, SIOCGIFINDEX, &info->ifreq_i)) < 0)
    {
        perror("error in index ioctl reading");
        return info;
    }

    // Get the MAC address
    memset(&info->ifreq_c, 0, sizeof(info->ifreq_c));
    strncpy(info->ifreq_c.ifr_name, ifName, IFNAMSIZ - 1);  // giving name of Interface
    if ((ioctl(info->sockfd, SIOCGIFHWADDR, &info->ifreq_c)) < 0) // getting MAC Address
    {
        perror("error in SIOCGIFHWADDR ioctl reading");
        return info;
    }

    return info;
}

void closeSocket(socketInfo * ptr) {
    free(ptr);
}
