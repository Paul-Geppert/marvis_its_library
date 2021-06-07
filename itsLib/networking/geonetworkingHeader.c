#include "./geonetworkingHeader.h"

void fillGeonetworkingHeader(geonetworkingHeader * geonetHeader, unsigned long payloadSize) {
    geonetHeader->basicHeader.versionAndNh ^= 0x00; // Version
    geonetHeader->basicHeader.versionAndNh ^= 0x01; // Next Header is Common Header
    geonetHeader->basicHeader.lt ^= 0x18;   // Time (t * 4 to skip time multiplier)
    geonetHeader->basicHeader.lt ^= 0x02;   // Time multiplier:   0->50ms   1->1s   2->10s   3->100s
    geonetHeader->basicHeader.rhl ^= 0x1;   // Remaining hops

    geonetHeader->commonHeader.nhAndReserved ^= 0x20;   // Next Header:   0->ANY   1->BTP-A   2->BTP-B   3->IPv6
    geonetHeader->commonHeader.htAndHst ^= 0x50;    // Type of geonetworking 0x50 -> SHB (Single Hop Broadcast)
    geonetHeader->commonHeader.tc ^= 0x03;  // Traffic class (for priority)
    geonetHeader->commonHeader.pl ^= htons(payloadSize);    // Length of payload
    geonetHeader->commonHeader.mhl ^= 0x1;  // Maximum hop limit
}
