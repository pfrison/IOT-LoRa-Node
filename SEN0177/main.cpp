#include "mbed.h"
#include "SEN0177.h"
#include "util.h"

Serial pc(PA_2, PA_3); // TX:D1 RX:D0 (default ports for pc usb)
Serial serial(PA_9, PA_10); // TX:D8 RX:D2
DigitalOut reset(PB_13); // D13

SEN0177 sen0177(&serial, &reset);

int main() {
    // initialisation
    pc.printf("Disco Init...\n\r");
    sen0177.init();
    
    while(1){
        int numOfParticles[SEN0177_ARRAY_NUM_PART_LENGTH];
        int status = sen0177.getNumOfParticles(numOfParticles);
        if(printErr(&pc, status) < 0)
            return 0;
        
        pc.printf(">0.3:%4d | >0.5:%4d | >1:%4d | >2.5:%4d | >5:%4d | >10:%4d | (in um par 0.1L)\n\r",
                numOfParticles[0], numOfParticles[1], numOfParticles[2], numOfParticles[3], numOfParticles[4], numOfParticles[5]);
    }
}
