#define PROTOCOL_VERSION 1

#include "../../generated-v1/CAM.h"

#include "../commonStructs.h"

int startCamService(const socketInfo * info, const char * vehId, int intervalSec);

int generateAndSendCam(const socketInfo * info, const char * vehId);
generationResult * generateCAM(const char * vehId);
int sendCam(const socketInfo * info, generationResult * camResult);


void _freeCAMGenerationResult(generationResult *ptr);
void _freeVehInfo(vehicleInformation * vehInfo);
