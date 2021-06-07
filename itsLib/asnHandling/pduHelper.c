#include "pduHelper.h"

asn_TYPE_descriptor_t * getPduType(int messageId, int protocolVersion) {
    asn_TYPE_descriptor_t **pdu = 0;
    asn_TYPE_descriptor_t *pduType = PDU_Type_Ptr;

    if (protocolVersion == 1) {
        pdu = asn_pdu_collection_v1;
    }
    else if (protocolVersion == 2) {
        pdu = asn_pdu_collection_v2;
    }

    if (pdu == 0 || messageId < 0 || messageId > 255) {
        return pduType;
    }

    pduType = pdu[messageId];

    return pduType;
}
