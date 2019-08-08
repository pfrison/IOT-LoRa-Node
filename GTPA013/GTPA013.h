#ifndef GTPA013_h
#define GTPA013_h

#include "mbed.h"

// Error codes
#define GTPA013_SUCCESS                     0
#define GTPA013_WARN_INCOMPLETE_PACKET      1 // length passed to readPacket was too short
#define GTPA013_ERR_PACKET_TIMEOUT          2 // Timeout while attempting to receive the packet
#define GTPA013_WARN_UNKNOW_SYNTAX          3 // Unknow syntax detected while spliting the packet in NMEA data
#define GTPA013_WARN_LOCATION_UNAVAILABLE   4 // GPS can't determine location

// Serial config
#define GTPA013_SERIAL_BUFFER_SIZE          300 // recommanded
#define GTPA013_SERIAL_BAUD                 9600 // default config is 9600 baud, 8 bits, no parity with 1 stop bit
#define GTPA013_SERIAL_BITS                 8
#define GTPA013_SERIAL_PARITY               SerialBase::None
#define GTPA013_SERIAL_STOP                 1

class GTPA013 {
    private:
        Serial *_serial;
        
        int readPacket(unsigned char *packet, int length);
        int getNMEAs(unsigned char *NMEA, int max_length);

    public:
        GTPA013(Serial *serial);
        
        void init();
        int getLocation(float *degrees);
};

#endif