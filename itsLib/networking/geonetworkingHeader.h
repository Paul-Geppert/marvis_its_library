#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>

// ##############################################################
// Find documentation at
// https://www.etsi.org/deliver/etsi_en/302600_302699/3026360401/01.04.01_60/en_3026360401v010401p.pdf
// ##############################################################

// manually set size of geonetworkin header, as sizeof() will return a too large value
#define SIZE_GEONET_HEADER 40

typedef struct
{
    struct {
        uint8_t versionAndNh;
        uint8_t reserved;
        uint8_t lt;
        uint8_t rhl;
    } basicHeader;
    struct {
        uint8_t nhAndReserved;
        uint8_t htAndHst;
        uint8_t tc;
        uint8_t flags;
        uint16_t pl;
        uint8_t mhl;
        uint8_t reserved;
    } commonHeader;
    struct {
        uint64_t part1;
        uint64_t part2;
        uint64_t part3;
    } sopv;
    struct {
        uint32_t part1;
    } media;
} geonetworkingHeader;

void fillGeonetworkingHeader(geonetworkingHeader * geonetHeader, unsigned long payloadSize);
