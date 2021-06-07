#include <errno.h>
#include <stdlib.h>

#include <linux/if_packet.h>
#include <net/ethernet.h>


#include "./spatemV2Generator.h"

#include "../../generated-v2/IntersectionState.h"
#include "../../generated-v2/MovementEvent.h"
#include "../../generated-v2/MovementState.h"
#include "../../generated-v2/TimeChangeDetails.h"

#include "../asnHandling/pduHelper.h"
#include "../networking/btpBHeader.h"
#include "../networking/geonetworkingHeader.h"

generationResult * generateSpatem(spatemParameters * parameters) {
    generationResult * generationError = (generationResult *) malloc(sizeof(generationResult));
    generationError->size = -1;
    generationError->buffer = NULL;

    SPATEM_t * spatem = (SPATEM_t *) calloc(1, sizeof(SPATEM_t));
    
    if (!spatem) {
        perror("calloc() failed!");
        return generationError;
    }

    spatem->header.protocolVersion = SPATEM_PROTOCOL_VERSION;
    spatem->header.messageID = ItsPduHeader__messageID_spatem;
    spatem->header.stationID = parameters->stationId;

    // Create an intersection object and add it to the intersection list
    IntersectionState_t * intersec = (IntersectionState_t *) calloc(1, sizeof(IntersectionState_t));
    asn_sequence_add(&spatem->spat.intersections.list, intersec);
    intersec->id.region = 0; // Optional
    intersec->id.id = parameters->intersectionId;
    intersec->revision = parameters->revision; // MessageCount (0..127)
    // intersec->status will contain zeros due to calloc

    // Create a movement state object for the intersection
    MovementState_t * mvState = (MovementState_t *) calloc(1, sizeof(MovementState_t));
    asn_sequence_add(&intersec->states.list, mvState);
    mvState->signalGroup = parameters->signalGroup;

    // Create a movement event object for the movement state
    MovementEvent_t * movementEvent = (MovementEvent_t *) calloc(1, sizeof(MovementEvent_t));
    asn_sequence_add(&mvState->state_time_speed.list, movementEvent);
    movementEvent->eventState = parameters->eventState;

    if (parameters->minEndTime >= 0) {
        TimeChangeDetails_t * timing = (TimeChangeDetails_t *) calloc(1, sizeof(TimeChangeDetails_t));
        timing->minEndTime = parameters->minEndTime;

        if (parameters->maxEndTime >= 0) {
            TimeMark_t * maxEndTime = (TimeMark_t *) calloc(1, sizeof(TimeMark_t));
            *maxEndTime = parameters->maxEndTime;
            timing->maxEndTime = maxEndTime;
        }
        
        movementEvent->timing = timing;
    }


    // Add generated SPATEM to output
    generationResult * generatedSpatem = generationError;
    generatedSpatem->size = sizeof(SPATEM_t);
    generatedSpatem->buffer = spatem;

    return generatedSpatem;
}

int generateAndSendSpatem(const socketInfo * info, spatemParameters * parameters)
{
    generationResult *spatemResult = generateSpatem(parameters);

    if (spatemResult->size < 0) {
        perror("Error generating SPATEM");
        return -1;
    }

    asn_encode_to_new_buffer_result_t spatemUperEncoded = asn_encode_to_new_buffer(0, ATS_UNALIGNED_BASIC_PER, getPduType(ItsPduHeader__messageID_spatem, SPATEM_PROTOCOL_VERSION), spatemResult->buffer);

    if (spatemUperEncoded.result.encoded < 0) {
        perror("Error encoding SPATEM as UPER");
        return -1;
    }

    // Preparing the sendbuffer
    void *sendbuffer = calloc(1, spatemUperEncoded.result.encoded + sizeof(btpBHeader) + SIZE_GEONET_HEADER + sizeof(struct ethhdr));

    int currentPositionInSendbuffer = 0;

    struct ethhdr *eth = (struct ethhdr *)(sendbuffer + currentPositionInSendbuffer);
    currentPositionInSendbuffer += sizeof(struct ethhdr);

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

    // ##############################################################
    // https://www.etsi.org/deliver/etsi_en/302600_302699/3026360401/01.04.01_60/en_3026360401v010401p.pdf
    // ##############################################################

    geonetworkingHeader *geonetHeader = (geonetworkingHeader *)(sendbuffer + currentPositionInSendbuffer);
    currentPositionInSendbuffer += SIZE_GEONET_HEADER;

    unsigned long payloadSize = sizeof(btpBHeader) + spatemUperEncoded.result.encoded;
    fillGeonetworkingHeader(geonetHeader, payloadSize);

    // ##############################################################
    // https://www.etsi.org/deliver/etsi_en/302600_302699/3026360501/01.02.01_60/en_3026360501v010201p.pdf
    // ##############################################################

    btpBHeader *btpHeader = (btpBHeader *)(sendbuffer + currentPositionInSendbuffer);
    currentPositionInSendbuffer += sizeof(btpBHeader);
    btpHeader->destinationPort = htons(2004); // 2001 means payload is SPATEM

    // Insert SPATEM
    memcpy(sendbuffer + currentPositionInSendbuffer, spatemUperEncoded.buffer, spatemUperEncoded.result.encoded);

    currentPositionInSendbuffer += spatemUperEncoded.result.encoded;
    

    struct sockaddr_ll sadr_ll;
    sadr_ll.sll_ifindex = info->ifreq_i.ifr_ifindex; // index of interface
    sadr_ll.sll_halen = ETH_ALEN;              // length of destination mac address
    sadr_ll.sll_addr[0] = eth->h_dest[0];
    sadr_ll.sll_addr[1] = eth->h_dest[1];
    sadr_ll.sll_addr[2] = eth->h_dest[2];
    sadr_ll.sll_addr[3] = eth->h_dest[3];
    sadr_ll.sll_addr[4] = eth->h_dest[4];
    sadr_ll.sll_addr[5] = eth->h_dest[5];

    ssize_t send_len = sendto(info->sockfd, sendbuffer, currentPositionInSendbuffer, 0, (const struct sockaddr *)&sadr_ll, sizeof(struct sockaddr_ll));
    if (send_len < 0)
    {
        printf("error in sending. sendlen: %ld | errno = %d\n", send_len, errno);
    }

    _freeSPATEMGenerationResult(spatemResult);
    free(sendbuffer);

    return send_len;
}

/**
 * Frees the data structure allocated for SPATEM generation
 */
void _freeSPATEMGenerationResult(generationResult *ptr)
{
    ASN_STRUCT_FREE(asn_DEF_SPATEM, ptr->buffer);
    free(ptr);
}
