#define PROTOCOL_VERSION 1
#define CAM_ID 2

#include <errno.h>
#include <unistd.h>

#include <net/ethernet.h>
#include <sys/time.h>

#include "./camV1Generator.h"

#include "../asnHandling/pduHelper.h"
#include "../marvisHandling/marvisConnector.h"
#include "../networking/btpBHeader.h"
#include "../networking/geonetworkingHeader.h"
#include "../networking/ethernetHeader.h"

int startCamService(const socketInfo * info, const char * vehId, int intervalSec) {
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Could not fork to CamService: errno = %d\n", errno);
        return -1;
    }

    // Parent
    if (pid > 0) {
        fprintf(stderr, "CamService has pid %ld\n", (long) pid);
        return pid;
    }

    // Convert to microseconds, because we can either use sleep or usleep
    // Sleep is to coarse  so we need usleep which is based on microseconds
    int intervalMicroSec = intervalSec * 1000 * 1000;

    struct timeval tv;
    time_t uStartTime;
    time_t uEndTime;

    while (1) {
        gettimeofday(&tv, NULL);
        uStartTime = tv.tv_sec * 1000 * 1000 + tv.tv_usec;

        generateAndSendCam(info, vehId);
        
        gettimeofday(&tv,NULL);
        uEndTime = tv.tv_sec * 1000 * 1000 + tv.tv_usec;
        
        usleep(intervalMicroSec - (uEndTime - uStartTime));
    }
}

int generateAndSendCam(const socketInfo * info, const char * vehId) {
    generationResult *camResult = generateCAM(vehId);

    if (camResult->size < 0) {
        // CAM not generated successfully -> do not send it
        perror("Error generating CAM");
        free(camResult);
        return -1;
    }

    if (camResult->size == 0) {
        fprintf(stderr, "device is not active\n");
        free(camResult);
        return 0;
    }

    int bytesSent = sendCam(info, camResult);
    _freeCAMGenerationResult(camResult);
}

generationResult * generateCAM(const char * vehId) {
    generationResult * generationError = (generationResult *) malloc(sizeof(generationResult));
    generationError->size = -1;
    generationError->buffer = NULL;

    CAM_t * cam = (CAM_t *) calloc(1, sizeof(CAM_t));
    
    if (!cam) {
        perror("calloc() failed!");
        return generationError;
    }

    vehicleInformation * vehInfo = getVehicleInformation(vehId);

    if (!vehInfo->isValid) {
        perror("Failed to request vehicle status");

        free(cam);
        _freeVehInfo(vehInfo);
    
        return generationError;
    }

    if (!vehInfo->status->isActive) {
        generationError->size = 0;

        free(cam);
        _freeVehInfo(vehInfo);
        
        return generationError;
    }

    vehicleStatus * vehStatus = vehInfo->status;

    cam->header.protocolVersion = PROTOCOL_VERSION;
    cam->header.messageID = ItsPduHeader__messageID_cam;
    cam->header.stationID = vehStatus->stationId;
    
    cam->cam.generationDeltaTime = vehStatus->genDeltaTime * GenerationDeltaTime_oneMilliSec;

    BasicContainer_t * basic = & cam->cam.camParameters.basicContainer;
    basic->stationType = StationType_passengerCar;
    basic->referencePosition.altitude.altitudeValue = AltitudeValue_unavailable;
    basic->referencePosition.altitude.altitudeConfidence = AltitudeConfidence_unavailable;
    basic->referencePosition.longitude = vehStatus->longitude;
    basic->referencePosition.latitude = vehStatus->latitude;
    basic->referencePosition.positionConfidenceEllipse.semiMajorOrientation = HeadingValue_unavailable;
    basic->referencePosition.positionConfidenceEllipse.semiMajorConfidence = SemiAxisLength_unavailable;
    basic->referencePosition.positionConfidenceEllipse.semiMinorConfidence = SemiAxisLength_unavailable;

    HighFrequencyContainer_t * hfc = & cam->cam.camParameters.highFrequencyContainer;
    hfc->present = HighFrequencyContainer_PR_basicVehicleContainerHighFrequency;

    BasicVehicleContainerHighFrequency_t * bvc = & hfc->choice.basicVehicleContainerHighFrequency;
    bvc->heading.headingValue = HeadingValue_unavailable;
    bvc->heading.headingConfidence = HeadingConfidence_unavailable;
    bvc->speed.speedValue = vehStatus->speed;
    bvc->speed.speedConfidence = SpeedConfidence_unavailable;
    bvc->driveDirection = DriveDirection_unavailable;

    bvc->vehicleLength.vehicleLengthValue = vehStatus->lengthInCentimeter;
    bvc->vehicleLength.vehicleLengthConfidenceIndication = VehicleLengthConfidenceIndication_unavailable;
    bvc->vehicleWidth = VehicleWidth_unavailable;

    bvc->longitudinalAcceleration.longitudinalAccelerationValue = LongitudinalAccelerationValue_unavailable;
    bvc->longitudinalAcceleration.longitudinalAccelerationConfidence = AccelerationConfidence_unavailable;

    bvc->curvature.curvatureValue = CurvatureValue_unavailable;
    bvc->curvature.curvatureConfidence = CurvatureConfidence_unavailable;
    bvc->curvatureCalculationMode = CurvatureCalculationMode_unavailable;
    bvc->yawRate.yawRateValue = YawRateValue_unavailable;
    bvc->yawRate.yawRateConfidence = YawRateConfidence_unavailable;

    generationResult * generatedCAM = generationError;
    generatedCAM->size = sizeof(CAM_t);
    generatedCAM->buffer = cam;

    _freeVehInfo(vehInfo);

    return generatedCAM;
}

/*
Sends a CAM message encoded as UPER via the socket specified in `info`.
Returns the number of bytes sent. Negative numbers indicate errors. 
*/
int sendCam(const socketInfo * info, generationResult * camResult) {
    //
    // Send the CAM message by following these steps:
    // * encode CAM as UPER
    // * prepare a sendbuffer which will be constructed/filled as follwed:
    // [Ethernet header | Geonetworking Header | BTP-B Header | CAM message UPER encoded]
    // * Send buffer to socket specified in `info`
    //

    asn_encode_to_new_buffer_result_t camUperEncoded = asn_encode_to_new_buffer(0, ATS_UNALIGNED_BASIC_PER, getPduType(CAM_ID, PROTOCOL_VERSION), camResult->buffer);

    // Prepare the sendbuffer
    void *sendbuffer = calloc(1, camUperEncoded.result.encoded + sizeof(btpBHeader) + SIZE_GEONET_HEADER + sizeof(struct ethhdr));

    int currentPositionInSendbuffer = 0;

    // Ethernet

    struct ethhdr *eth = (struct ethhdr *)(sendbuffer + currentPositionInSendbuffer);
    currentPositionInSendbuffer += sizeof(struct ethhdr);

    fillEthernetHeader(eth, info);

    // Geonetworking

    geonetworkingHeader *geonetHeader = (geonetworkingHeader *)(sendbuffer + currentPositionInSendbuffer);
    currentPositionInSendbuffer += SIZE_GEONET_HEADER;

    unsigned long payloadSize = sizeof(btpBHeader) + camUperEncoded.result.encoded;
    fillGeonetworkingHeader(geonetHeader, payloadSize);

    // BTP-B

    btpBHeader *btpHeader = (btpBHeader *)(sendbuffer + currentPositionInSendbuffer);
    currentPositionInSendbuffer += sizeof(btpBHeader);
    btpHeader->destinationPort = htons(2001); // 2001 means payload is CAM

    // Insert CAM

    memcpy(sendbuffer + currentPositionInSendbuffer, camUperEncoded.buffer, camUperEncoded.result.encoded);
    currentPositionInSendbuffer += camUperEncoded.result.encoded;

    // Send CAM message
    struct sockaddr_ll sadr_ll;
    sadr_ll.sll_ifindex = info->ifreq_i.ifr_ifindex;    // index of interface
    sadr_ll.sll_halen = ETH_ALEN;                       // length of destination mac address
    sadr_ll.sll_addr[0] = eth->h_dest[0];
    sadr_ll.sll_addr[1] = eth->h_dest[1];
    sadr_ll.sll_addr[2] = eth->h_dest[2];
    sadr_ll.sll_addr[3] = eth->h_dest[3];
    sadr_ll.sll_addr[4] = eth->h_dest[4];
    sadr_ll.sll_addr[5] = eth->h_dest[5];

    ssize_t send_len = sendto(info->sockfd, sendbuffer, currentPositionInSendbuffer, 0, (const struct sockaddr *)&sadr_ll, sizeof(struct sockaddr_ll));
    if (send_len < 0)
    {
        fprintf(stderr, "error in sending. sendlen: %ld | errno = %d\n", send_len, errno);
    }

    free(sendbuffer);

    return send_len;
};

/**
 * Frees the data structure allocated for CAM generation
 */
void _freeCAMGenerationResult(generationResult *ptr)
{
    ASN_STRUCT_FREE(asn_DEF_CAM, ptr->buffer);
    free(ptr);
}

void _freeVehInfo(vehicleInformation * vehInfo) {
    if (vehInfo->status) {
        free(vehInfo->status);
    }
    free(vehInfo);
}
