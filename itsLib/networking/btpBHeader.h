#include <stdint.h>

// ##############################################################
// Find documentation at
// https://www.etsi.org/deliver/etsi_en/302600_302699/3026360501/01.02.01_60/en_3026360501v010201p.pdf
// ##############################################################

typedef struct btpBHeader
{
    uint16_t destinationPort;
    uint16_t destinationPortInfo;
} btpBHeader;
