#include <asn_application.h>

#define PDU_Type_Ptr    NULL

asn_TYPE_descriptor_t * getPduType(int messageId, int protocolVersion);

// PDUs are defined in pdu_collection.c
extern asn_TYPE_descriptor_t *asn_pdu_collection_v1[];
extern asn_TYPE_descriptor_t *asn_pdu_collection_v2[];

