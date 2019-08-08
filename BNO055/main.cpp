#include "mbed.h"
#include "BNO055.h"
#include "util.h"

#define BUFFER_SIZE 50

I2C i2c(I2C_SDA, I2C_SCL);
Serial pc(SERIAL_TX, SERIAL_RX);
DigitalOut reset(PB_13); // D13

BNO055 bno055(&i2c, &reset, BNO055_DEFAULT_ADDR);

int main() {
    // initialisation
    pc.printf("Disco Init...\n\r");
    bno055.init(BNO055_CONFIG_COMPASS);
    
    // calibration
    pc.printf("Calibration... Do an '8' on air with the sensor in your hand.\n\r");
    int status = bno055.performCalibration(BNO055_CALIB_SYS);
    if(status != BNO055_SUCCESS){
        pc.printf("Couldn't perform calibration !\n\r");
        printErr(&pc, status);
        return -1;
    }
    pc.printf("Calibration complete.\n\r");
    
    // compass mode
    while(1){
        double angles[3];
        int status = bno055.getEulerAngles(angles, BNO055_UNIT_DEG);
        if(status != BNO055_SUCCESS){
            pc.printf("Couldn't read angles !\n\r");
            printErr(&pc, status);
            return -1;
        }
        
        pc.printf("P:%10.02f deg | R:%10.02f deg | Y:%10.02f deg\n\r", angles[0], angles[1], angles[2]);
        
        wait(0.1);
    }
}
