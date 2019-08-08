#ifndef SEN0177_h
#define SEN0177_h

#include "mbed.h"

// Error codes
#define SEN0177_SUCCESS                 0
#define SEN0177_WARN_CHECKSUM           1 // packet contains one or more errors
#define SEN0177_ERR_GET_PACKET          2 // packet not recognized

// Serial config
#define SEN0177_SERIAL_BAUD             9600 // default config is 9600 baud, 8 bits, no parity with 1 stop bit
#define SEN0177_SERIAL_BITS             8
#define SEN0177_SERIAL_PARITY           SerialBase::None
#define SEN0177_SERIAL_STOP             1

// Serial fixed bytes
#define SEN0177_SERIAL_PACKET_LENGTH    32
#define SEN0177_SERIAL_DATA_LENGTH      9
#define SEN0177_SERIAL_START_1          0x42
#define SEN0177_SERIAL_START_2          0x4D

// Data sizes
#define SEN0177_ARRAY_CONCENTR_LENGTH   3
#define SEN0177_ARRAY_NUM_PART_LENGTH   6

class SEN0177 {
    private:
        Serial *_serial;
        DigitalOut *_reset;
        
        int checkPacket(unsigned char packet[SEN0177_SERIAL_PACKET_LENGTH]);
        int readPacket(unsigned char packet[SEN0177_SERIAL_PACKET_LENGTH]);

    public:
        SEN0177(Serial *serial, DigitalOut *reset);
        
        void init();
        int getData(int packet[SEN0177_SERIAL_PACKET_LENGTH]);
        int getConcentration(int concentrations[SEN0177_ARRAY_CONCENTR_LENGTH]);
        int getNumOfParticles(int numOfParticles[SEN0177_ARRAY_NUM_PART_LENGTH]);
};

#endif