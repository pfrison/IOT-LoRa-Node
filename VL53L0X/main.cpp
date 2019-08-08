#include "mbed.h"
#include "VL53L0X.h"
#include "util.h"

I2C i2c(I2C_SDA, I2C_SCL);
Serial pc(SERIAL_TX, SERIAL_RX);

VL53L0X vl53l0x(&i2c, VL53L0X_ADDRESS);

int main() {
    // initialisation
    pc.printf("Disco Init...\n\r");
    int status = vl53l0x.init(true);
    if(status != VL53L0X_SUCCESS){
        pc.printf("Couldn't perform initialisation !\n\r");
        printErr(&pc, status);
        return -1;
    }
    pc.printf("Initialisation complete.\n\r");
    
    pc.printf("Successfull\n\r");
}
