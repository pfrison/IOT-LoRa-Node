#include "mbed.h"
#include "BNO055.h"

int printErr(Serial *pc, int status){
    if(status == BNO055_SUCCESS)
        return 0;
    switch(status){
        case BNO055_ERR_WRITE:
            pc->printf("ERR : Erreur d'ecriture.\n\r");
            return -1;
        case BNO055_ERR_READ:
            pc->printf("ERR : Erreur de lecture.\n\r");
            return -1;
        case BNO055_WARN_FORBIDDEN:
            pc->printf("WARN : Registre en lecture seule.\n\r");
            return 1;
        case BNO055_ERR_WRONG_CONFIG:
            pc->printf("ERR : Mauvaise configuration.\n\r");
            return -1;
        case BNO055_ERR_WRONG_UNIT:
            pc->printf("ERR : Mauvaise selection des unites de mesure.\n\r");
            return -1;
        case BNO055_WARN_NOT_CALIBRATED:
            pc->printf("WARN : Capteur non calibre.\n\r");
            return 1;
    }
    pc->printf("ERR : erreur inconnue.\n\r");
    return -1;
}