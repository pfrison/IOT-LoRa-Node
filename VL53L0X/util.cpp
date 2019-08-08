#include "mbed.h"
#include "VL53L0X.h"

int printErr(Serial *pc, int status){
    if(status == VL53L0X_SUCCESS)
        return 0;
    switch(status){
        case VL53L0X_ERR_READ:
            pc->printf("ERR : Erreur de lecture.\n\r");
            return -1;
        case VL53L0X_ERR_WRITE:
            pc->printf("ERR : Erreur d'ecriture.\n\r");
            return -1;
        case VL53L0X_ERR_CALIBRATION_TIMEOUT:
            pc->printf("ERR : Calibration timeout.\n\r");
            return -1;
        case VL53L0X_ERR_SPAD_TIMEOUT:
            pc->printf("ERR : Timeout while waiting for SPAD info.\n\r");
            return -1;
        case VL53L0X_ERR_BUDGET_TOO_LOW:
            pc->printf("ERR : Timeing bugdet too low.\n\r");
            return -1;
        case VL53L0X_ERR_REQ_TIMEOUT_TOO_BIG:
            pc->printf("ERR : Requested timeout is too big.\n\r");
            return -1;
        case VL53L0X_ERR_VCSEL_PERIOD:
            pc->printf("ERR : Wrong VCSEL period.\n\r");
            return -1;
        case VL53L0X_ERR_VCSEL_TYPE:
            pc->printf("ERR : Wrong VCSEL type.\n\r");
            return -1;
        case VL53L0X_ERR_INTERRUPT_TIMEOUT:
            pc->printf("ERR : Interrupt order timeout.\n\r");
            return -1;
        case VL53L0X_ERR_START_RANGE_TIMEOUT:
            pc->printf("ERR : Start measurement order timeout.\n\r");
            return -1;
    }
    pc->printf("ERR : erreur inconnue.\n\r");
    return -1;
}