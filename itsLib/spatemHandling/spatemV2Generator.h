#define SPATEM_PROTOCOL_VERSION 2

#include "../../generated-v2/SPATEM.h"

#include "../commonStructs.h"

typedef struct spatemParameters
{
    int stationId;
    int intersectionId;
    int revision;
    int intersectionStatus;
    int signalGroup;
    int eventState;
    int minEndTime;
    int maxEndTime;
} spatemParameters;

generationResult * generateSpatem(spatemParameters * parameters);
int generateAndSendSpatem(const socketInfo * info, spatemParameters * parameters);

/**
 * Frees the data structure allocated for SPATEM generation
 */
void _freeSPATEMGenerationResult(generationResult *ptr);
