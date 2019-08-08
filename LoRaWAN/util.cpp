#include "mbed.h"
#include "LoRaWAN.h"

int printErr(Serial *pc, int status){
    if(status == LORA_SUCCESS)
        return 0;
    switch(status){
        case LORA_WARN_TX_TIMEOUT:
            pc->printf("WARN : Tx timeout.\n\r");
            return 1;
        case LORA_WARN_RX_TIMEOUT:
            pc->printf("WARN : Rx timeout.\n\r");
            return 1;
        case LORA_ERR_RX_ERROR:
            pc->printf("ERR : Rx error.\n\r");
            return -1;
        case LORA_ERR_RADIO_INIT:
            pc->printf("ERR : Unable to init LoRa.\n\r");
            return -1;
        case LORA_ERR_RADIO_BUSSY:
            pc->printf("ERR : Transmission already in progress.\n\r");
            return -1;
        case LORA_ERR_ASYNC_DISABLED:
            pc->printf("ERR : Asynchronous transmissions disabled at setup.\n\r");
            return -1;
    }
    pc->printf("ERR : Unknow error.\n\r");
    return -1;
}