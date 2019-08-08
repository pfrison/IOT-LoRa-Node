#include "mbed.h"
#include "LoRaWAN.h"
#include "util.h"

#define BUFFER_SIZE 60

Serial pc(SERIAL_TX, SERIAL_RX);
LoRaWAN lora(LORA_DEFAULT_TX_TIMEOUT, 10000/*LORA_DEFAULT_RX_TIMEOUT*/, LOAR_VIETNAM_FREQUENCY, true);

char res[BUFFER_SIZE];

void receiveCallback(int status){
    if(printErr(&pc, status) == 0)
        pc.printf("%s\n\r\n\r", res);
    status = lora.receiveAsync(res, BUFFER_SIZE, receiveCallback);
    printErr(&pc, status);
}

int main() {
    // initialisation
    pc.printf("Nucleo Init...\n\r");
    int status = lora.init();
    if(printErr(&pc, status) != 0)
        return 0;
    
    // Async
    /*status = lora.receiveAsync(res, BUFFER_SIZE, receiveCallback);
    if(printErr(&pc, status) != 0)
        return 0;
    pc.printf("Reception loop launched.\n\r");
    while(1)
        sleep();*/
    
    // Sync
    while(1){
        for(int i=0; i<BUFFER_SIZE; i++)
            res[i] = '\0';
        status = lora.receiveSync(res, BUFFER_SIZE);
        if(printErr(&pc, status) != 0)
            continue;
        pc.printf("%s\n\r\n\r", res);
    }
}
