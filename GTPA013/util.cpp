#include "mbed.h"
#include "GTPA013.h"

int printErr(Serial *pc, int status){
    if(status == GTPA013_SUCCESS)
        return 0;
    switch(status){
        case GTPA013_WARN_INCOMPLETE_PACKET:
            pc->printf("WARN : Packet incomplete.\n\r");
            return 1;
        case GTPA013_ERR_PACKET_TIMEOUT:
            pc->printf("ERR : Packet Timeout.\n\r");
            return -1;
        case GTPA013_WARN_UNKNOW_SYNTAX:
            pc->printf("WARN : NMEA syntaxes not recognized.\n\r");
            return 1;
        case GTPA013_WARN_LOCATION_UNAVAILABLE:
            pc->printf("WARN : Location unavailable.\n\r");
            return -1;
    }
    pc->printf("ERR : Unknow error.\n\r");
    return -1;
}