#include <net/if.h>
#include <stdlib.h>

#ifndef COMMON_STRUCTS_INCLUDED
#define COMMON_STRUCTS_INCLUDED

typedef struct vehicleStatus
{
    int isActive;
    uint16_t genDeltaTime;
    int stationId;
    double longitude;
    double latitude;
    double speed;
    int lengthInCentimeter;
} vehicleStatus;

typedef struct generationResult
{
    int size;
    void * buffer;
} generationResult;

typedef struct {
    int isValid;
    vehicleStatus * status;
} vehicleInformation;

typedef struct socketInfo {
    int sockfd;
    struct ifreq ifreq_i;
    struct ifreq ifreq_c;
} socketInfo;

#endif
