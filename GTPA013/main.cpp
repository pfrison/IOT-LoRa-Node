#include "mbed.h"
#include "GTPA013.h"
#include "util.h"

Serial pc(SERIAL_TX, SERIAL_RX);

Serial serial(PA_9, PA_10); // TX:D8 RX:D2
GTPA013 gtpa013(&serial);


int main() {
    // initialisation
    pc.printf("Disco Init...\n\r");
    gtpa013.init();
    
    wait(0.2);
    float location[2];
    int status = gtpa013.getLocation(location);
    if(printErr(&pc, status) < 0)
        return 0;
        
    pc.printf("%f N | %f E\n\r", location[0], location[1]);
    return 0;
}
