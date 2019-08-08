#include "GTPA013.h"
#include <math.h>

// ----- CONSTRUCTORS -----

/**
 * GTPA013 initialisation.
 */
GTPA013::GTPA013(Serial *serial) {
    _serial = serial;
}

// ----- PRIVATE METHODS -----

/**
 * Capture a whole packet from start
 */
int GTPA013::readPacket(unsigned char *packet, int length){
    //detect start "$GPGGA"
    int stage = 0;
    int attemptsLeft = length;
    int i=0;
    while(stage != 6){
        attemptsLeft--;
        if(attemptsLeft == 0)
            return GTPA013_ERR_PACKET_TIMEOUT;
        
        packet[i] = (unsigned char) _serial->getc();
        switch(packet[i]){
            case '$':
                if(stage == 0) stage++; else stage = 0;
                break;
            case 'G':
                if(stage == 1 || stage == 3 || stage == 4) stage++; else stage = 0;
                break;
            case 'P':
                if(stage == 2) stage++; else stage = 0;
                break;
            case 'A':
                if(stage == 5) stage++; else stage = 0;
                break;
            default:
                stage = 0;
        }
        
        i++;
    }
    packet[0] = '$';
    packet[1] = 'G';
    packet[2] = 'P';
    packet[3] = 'G';
    packet[4] = 'G';
    packet[5] = 'A';
    
    // record while detecting for a new start "$GPGGA"
    stage = 0;
    int stageStart = -1;
    for(int i=6; i<length; i++){
        packet[i] = (unsigned char) _serial->getc();
        switch(packet[i]){
            case '$':
                if(stage == 0) { stage++; stageStart = i; } else stage = 0;
                break;
            case 'G':
                if(stage == 1 || stage == 3 || stage == 4) stage++; else stage = 0;
                break;
            case 'P':
                if(stage == 2) stage++; else stage = 0;
                break;
            case 'A':
                if(stage == 5) stage++; else stage = 0;
                break;
            default:
                stage = 0;
        }
        if(stage == 6){ // 2nd start detected -> remove excess (+ "\n") and stop
            for(int j=stageStart-1; j<stageStart+7; j++)
                packet[j] = '\0';
            break;
        }
    }
    if(stage != 6)
        return GTPA013_WARN_INCOMPLETE_PACKET;
    return GTPA013_SUCCESS;
}

/**
 * Capture a whole packet and divide it in this NMEA sequences [GGA, GSA, GSV, RMC, VTG]
 */
int GTPA013::getNMEAs(unsigned char *NMEA, int max_length){
    unsigned char packet[max_length*3];
    int status = readPacket(packet, max_length*3);
    if(status != GTPA013_SUCCESS)
        return status;
    
    for(int i=0; i<max_length*5; i++){
        NMEA[i] = '\0';
    }
    int i = 0;
    int j = -1;
    int index = 0;
    int step = 0;
    status = GTPA013_SUCCESS;
    while(packet[i] != '\0'){
        if(packet[i] == '\n' || j == -1){
            if(j != -1)
                i++; // skip "\n"
            step = 0;
            i += 3; // skip "$GP"
            NMEA[(index*max_length) + j] = '\0';
            j = 0;
            
            // detect NMEA
            if(step == 0 && packet[i] == 'G'){ // GGA, GSA, GSV
                step = 1;
                i++;
                if(packet[i] == 'G'){ // GGA
                    i += 3; // skip "GA,"
                    index = 0;
                }else{ // GSA GSV
                    i++;
                    if(packet[i] == 'A'){ // GSA
                        i += 2; // skip "A,"
                        index = 1;
                    }else{ // GSV
                        i += 2; // skip "V,"
                        index = 2;
                    }
                }
            }else if(step == 0 && packet[i] == 'R'){ // RMC
                index = 3;
                step = 1;
                i += 4; // skip "RMC,"
            }else if(step == 0 && packet[i] == 'V'){ // VTG
                index = 4;
                step = 1;
                i += 4; // skip "VTG,"
            }else{
                step = 0;
            }
        }else{
            if(step == 0)
                status = GTPA013_WARN_UNKNOW_SYNTAX;
            else{
                NMEA[(index*max_length) + j] = packet[i];
                j++;
            }
            i++;
        }
    }
    NMEA[(index*max_length) + j] = '\0';
    return status;
}

// ----- PUBLIC METHODS -----

/**
 * Give the location or an error if null. Units are in degrees [°North, °East]
 */
int GTPA013::getLocation(float *degrees){
    // get GGA
    unsigned char NMEA[5][200];
    int status = getNMEAs((unsigned char*) NMEA, 200);
    if(status != GTPA013_SUCCESS)
        return status;
    unsigned char GGA[200];
    for(int i=0; i<200; i++)
        GGA[i] = NMEA[0][i];
    
    // get latitude, N/S indicator, longitude and E/W indicator
    float latitude = -1;
    bool north = true;
    float longitude = -1;
    bool east = true;
    int stage = 0;
    int i = 0;
    while(stage != 5){
        if(GGA[i] == ',')
            stage++;
        else if(stage == 1){ // ddmm.mmmm
            latitude = 0;
            // dd
            for(int j=0; j<2; j++)
                latitude += (GGA[i+j] - '0') * pow((float) 10, 1-j);
            // mm.mmmm
            for(int j=2; j<9; j++){
                if(GGA[i+j] == '.')
                    continue;
                latitude += (GGA[i+j] - '0') * pow((float) 10, 3-j) / 60;
            }
            i += 8;
        }else if(stage == 2)
            north = GGA[i] == 'N';
        else if(stage == 3){ // dddmm.mmmm
            longitude = 0;
            // ddd
            for(int j=0; j<3; j++)
                longitude += (GGA[i+j] - '0') * pow((float) 10, 2-j);
            // mm.mmmm
            for(int j=3; j<10; j++){
                if(GGA[i+j] == '.')
                    continue;
                longitude += (GGA[i+j] - '0') * pow((float) 10, 4-j) / 60;
            }
            i += 9;
        }else if(stage == 4)
            east = GGA[i] == 'E';
        i++;
    }
    if(latitude == -1 || longitude == -1)
        return GTPA013_WARN_LOCATION_UNAVAILABLE;
    
    degrees[0] = north ? latitude : -latitude;
    degrees[1] = east ? longitude : -longitude;
    return GTPA013_SUCCESS;
}

/**
 * Init sensor : configure serial
 */
void GTPA013::init(){
    // configure serial
    _serial->baud(GTPA013_SERIAL_BAUD);
    _serial->format(GTPA013_SERIAL_BITS, GTPA013_SERIAL_PARITY, GTPA013_SERIAL_STOP);
}