#include "SEN0177.h"


// ----- CONSTRUCTORS -----

/**
 * SEN0177 initialisation.
 */
SEN0177::SEN0177(Serial *serial, DigitalOut *reset) {
    _serial = serial;
    _reset = reset;
}

// ----- PRIVATE METHODS -----

/**
 * Check packet (with checksum)
 */
int SEN0177::checkPacket(unsigned char packet[SEN0177_SERIAL_PACKET_LENGTH]){
    // calculate checksum
    int check16bits = 0;
    for(int i=0; i<SEN0177_SERIAL_PACKET_LENGTH - 2; i++)
        check16bits = (check16bits + packet[i]) & 0xFFFF;
    
    // compare
    int recievedCheck = (packet[SEN0177_SERIAL_PACKET_LENGTH - 2] << 8)
            + packet[SEN0177_SERIAL_PACKET_LENGTH - 1];
    if(check16bits == recievedCheck)
        return SEN0177_SUCCESS;
    return SEN0177_WARN_CHECKSUM;
}

/**
 * Capture a whole packet from start
 */
int SEN0177::readPacket(unsigned char packet[SEN0177_SERIAL_PACKET_LENGTH]){
    // detect starts char
    bool start1 = false;
    bool start2 = false;
    int triesLeft = 1000;
    while(!start1 || !start2){
        if(triesLeft == 0)
            return SEN0177_ERR_GET_PACKET;
        triesLeft--;
        
        char c = (char) _serial->getc();
        if(c == SEN0177_SERIAL_START_1){
            start1 = true;
        } else if(start1 && c == SEN0177_SERIAL_START_2) {
            start2 = true;
        } else {
            start1 = false;
            start2 = false;
        }
    }
    
    packet[0] = SEN0177_SERIAL_START_1;
    packet[1] = SEN0177_SERIAL_START_2;
    
    // record sequence
    
    for(int i=2; i<SEN0177_SERIAL_PACKET_LENGTH; i++)
        packet[i] = (char) _serial->getc();
    
    // check packet
    return checkPacket(packet);
}

// ----- PUBLIC METHODS -----

/**
 * Init sensor : activate reset and configure serial format
 */
void SEN0177::init(){
    // reset
    *_reset = 0;
    wait(0.5);
    *_reset = 1;
    wait(1);
    
    // configure serial
    _serial->baud(SEN0177_SERIAL_BAUD);
    _serial->format(SEN0177_SERIAL_BITS, SEN0177_SERIAL_PARITY, SEN0177_SERIAL_STOP);
}

/**
 * Recieve a packet and strip usefull data
 * 
 * Data (16 bits ints):
 *  - from 0 to 3 : particle concentration [PM1.0, PM2.5, PM10]
 *  - from 4 to 8 : number of paticles [>0.3, >0.5, >1, >2.5, >5, >10] in um
 *
 * PM : particulate matter
 * the number represent he size of the particles 2.5 = 2.5 um (micro meter)
 */
static int conv8to16bits(char a, char b){ return (a << 8) + b; }
int SEN0177::getData(int data[SEN0177_SERIAL_DATA_LENGTH]){
    // get packet
    unsigned char packet[SEN0177_SERIAL_PACKET_LENGTH];
    int status = readPacket(packet);
    if(status != SEN0177_SUCCESS
            && status != SEN0177_WARN_CHECKSUM)
        return status;
    
    // particles concentrations
    data[0] = conv8to16bits(packet[4], packet[5]);
    data[1] = conv8to16bits(packet[6], packet[7]);
    data[2] = conv8to16bits(packet[8], packet[9]);
    
    // number of particles
    data[3] = conv8to16bits(packet[16], packet[17]);
    data[4] = conv8to16bits(packet[18], packet[19]);
    data[5] = conv8to16bits(packet[20], packet[21]);
    data[6] = conv8to16bits(packet[22], packet[23]);
    data[7] = conv8to16bits(packet[24], packet[25]);
    data[8] = conv8to16bits(packet[26], packet[27]);
    
    return status;
}

/**
 * Get particle concentration in an array : [PM1.0, PM2.5, PM10] in ug/m^3
 * 
 * PM : particulate matter
 * the number represent he size of the particles 2.5 = 2.5 um (micro meter)
 */
int SEN0177::getConcentration(int concentrations[SEN0177_ARRAY_CONCENTR_LENGTH]){
    int data[SEN0177_SERIAL_DATA_LENGTH];
    int status = getData(data);
    if(status != SEN0177_SUCCESS
            && status != SEN0177_WARN_CHECKSUM)
        return status;
    
    concentrations[0] = data[0];
    concentrations[1] = data[1];
    concentrations[2] = data[2];
    
    return status;
}

/**
 * Get the number of particle above a diameter in an array :
 * [>0.3, >0.5, >1, >2.5, >5, >10] in um
 */
int SEN0177::getNumOfParticles(int numOfParticles[SEN0177_ARRAY_NUM_PART_LENGTH]){
    int data[SEN0177_SERIAL_DATA_LENGTH];
    int status = getData(data);
    if(status != SEN0177_SUCCESS
            && status != SEN0177_WARN_CHECKSUM)
        return status;
    
    numOfParticles[0] = data[3];
    numOfParticles[1] = data[4];
    numOfParticles[2] = data[5];
    numOfParticles[3] = data[6];
    numOfParticles[4] = data[7];
    numOfParticles[5] = data[8];
    
    return status;
}