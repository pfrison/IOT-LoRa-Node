#include "mbed.h"
#include "SEN0177.h"

int printErr(Serial *pc, int status){
    if(status == SEN0177_SUCCESS)
        return 0;
    switch(status){
        case SEN0177_WARN_CHECKSUM:
            pc->printf("WARN : Packet checksum failed.\n\r");
            return 1;
        case SEN0177_ERR_GET_PACKET:
            pc->printf("ERR : Unable to identify data.\n\r");
            return -1;
    }
    pc->printf("ERR : Unknow error.\n\r");
    return -1;
}