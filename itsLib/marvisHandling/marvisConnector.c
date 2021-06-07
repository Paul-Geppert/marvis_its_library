#include <stdio.h>
#include <stdlib.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>

#include <stdint.h>
#include "marvisConnector.h"

#define NAME       "XML-RPC Marvis Connector"
#define VERSION    "0.1"
#define SERVER_URL "172.17.0.1:23404"

// Find documentation on http://xmlrpc-c.sourceforge.net/ and https://tldp.org/HOWTO/XML-RPC-HOWTO/index.html

// TODO: vehicle ID (sumo) + station ID

vehicleInformation * getVehicleInformation(const char * vehId)
{
    xmlrpc_env env;
    xmlrpc_value *rpcVehStatus;
    xmlrpc_double speed;
    xmlrpc_int32 isActive;
    xmlrpc_double pos_x, pos_y, pos_z;
    xmlrpc_int32 lengthInCentimeter;

    vehicleInformation * vehInfoError = malloc(sizeof(vehicleInformation));
    vehInfoError->isValid = 0;
    vehInfoError->status = 0;
    
    /* Start up the XML-RPC client library. */
    xmlrpc_client_init(XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION);
    xmlrpc_env_init(&env);

    /* Call the XML-RPC server. */
    rpcVehStatus = xmlrpc_client_call(&env, SERVER_URL,
                                      "getVehicleDetails", "(s)",
                                      vehId);

    if (env.fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n", env.fault_string, env.fault_code);
        return vehInfoError;
    }

    vehicleStatus * vehStatus = malloc(sizeof(vehicleStatus));
    vehStatus->genDeltaTime = 0;
    
    /* Parse the result value. */
    xmlrpc_decompose_value(&env, rpcVehStatus, "{s:i,s:(ddd),s:d,s:i,*}",
                           "isVehicleActive", &isActive,
                           "position3d", &pos_x, &pos_y, &pos_z,
                           "speed", &speed,
                           "lengthInCentimeter", &lengthInCentimeter
    );

    vehStatus->isActive = isActive;
    vehStatus->speed = speed;
    vehStatus->latitude = pos_x;
    vehStatus->longitude = pos_y;
    vehStatus->lengthInCentimeter = lengthInCentimeter;
    vehStatus->stationId = 123456;

    if (env.fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n", env.fault_string, env.fault_code);
        return vehInfoError;
    }
    
    /* Dispose of result value. */
    xmlrpc_DECREF(rpcVehStatus);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();

    vehicleInformation * vehInfo = vehInfoError;
    vehInfo->isValid = 1;
    vehInfo->status = vehStatus;

    return vehInfo;
}
