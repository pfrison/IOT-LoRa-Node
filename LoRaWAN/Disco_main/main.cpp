#include "mbed.h"
#include "LoRaWAN.h"
#include "util.h"

Serial pc(SERIAL_TX, SERIAL_RX);
LoRaWAN lora(LORA_DEFAULT_TX_TIMEOUT, LORA_DEFAULT_RX_TIMEOUT, LOAR_VIETNAM_FREQUENCY, true);

DigitalOut success(LED1);
DigitalOut fail(LED3);

void asyncCallback(int status){
    if(printErr(&pc, status) != 0)
        return;
    pc.printf("Send async (callback) successfull.\n\r");
}

int main() {
    // initialisation
    fail = 1;
    success = 1;
    pc.printf("Disco Init...\n\r");
    int status = lora.init();
    if(printErr(&pc, status) != 0)
        return 0;
    
    char send[2];
    send[0] = 'a';
    send[1] = '\0';
    status = lora.sendSync(send, 2);
    if(printErr(&pc, status) != 0)
        return 0;
    pc.printf("Send sync successfull.\n\r");
    
    wait(1);
    
    char send2[3];
    send2[0] = 'b';
    send2[1] = 'b';
    send2[2] = '\0';
    status = lora.sendAsync(send2, 3, asyncCallback);
    if(printErr(&pc, status) != 0)
        return 0;
    pc.printf("Send async (call) successfull.\n\r");
    
    while(1)
        sleep();
}
