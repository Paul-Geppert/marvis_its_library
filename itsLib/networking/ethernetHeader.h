#include <linux/if_packet.h>

#include "../commonStructs.h"

void fillEthernetHeader(struct ethhdr * ethernetHeader, const socketInfo * info);
