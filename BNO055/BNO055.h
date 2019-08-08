#ifndef BNO055_h
#define BNO055_h

#include "mbed.h"

// Error codes
#define BNO055_SUCCESS              0
#define BNO055_ERR_WRITE            1 // an error occured when writing
#define BNO055_ERR_READ             2 // an error occured when reading
#define BNO055_WARN_FORBIDDEN       3 // user trying to write on a read-only register
#define BNO055_ERR_WRONG_CONFIG     4 // wrong config given
#define BNO055_ERR_WRONG_UNIT       5 // wrong unit selection (i.e. asking to convert meters in degrees)
#define BNO055_WARN_NOT_CALIBRATED  6 // user didn't calibrate the sensor

// I2C Addresses
#define BNO055_DEFAULT_ADDR         (0x28 << 1)
#define BNO055_ALT_ADDR             (0x29 << 1)

// Config codes
#define BNO055_CONFIG_REG           0x3D
#define BNO055_CONFIG_BLANK         0b0000 // waiting for config
#define BNO055_CONFIG_ACCONLY       0b0001 // accelerometer
#define BNO055_CONFIG_MAGONLY       0b0010 // magnetometer
#define BNO055_CONFIG_GYROONLY      0b0011 // gyroscope
#define BNO055_CONFIG_ACCMAG        0b0100 // accelerometer + magnetometer
#define BNO055_CONFIG_ACCGYRO       0b0101 // accelerometer + gyroscope
#define BNO055_CONFIG_MAGGYRO       0b0110 // magnetometer + gyroscope
#define BNO055_CONFIG_AMG           0b0111 // accelerometer + magnetometer + gyroscope
#define BNO055_CONFIG_IMU           0b1000 // inertial measurement unit
#define BNO055_CONFIG_COMPASS       0b1001 // compass
#define BNO055_CONFIG_M4G           0b1010 // magnet for gyroscope
#define BNO055_CONFIG_NDOF_FMC_OFF  0b1011 // NDOF + fast magnetometer calibration off
#define BNO055_CONFIG_NDOF          0b1100 // 9 degrees of freedom

// Instruments registers
#define BNO055_ACC_X_LSB            0x08 // accelerometer
#define BNO055_ACC_X_MSB            0x09
#define BNO055_ACC_Y_LSB            0x0A
#define BNO055_ACC_Y_MSB            0x0B
#define BNO055_ACC_Z_LSB            0x0C
#define BNO055_ACC_Z_MSB            0x0D
#define BNO055_MAG_X_LSB            0x0E // magnetometer
#define BNO055_MAG_X_MSB            0x0F
#define BNO055_MAG_Y_LSB            0x10
#define BNO055_MAG_Y_MSB            0x11
#define BNO055_MAG_Z_LSB            0x12
#define BNO055_MAG_Z_MSB            0x13
#define BNO055_GYR_X_LSB            0x14 // gyroscope
#define BNO055_GYR_X_MSB            0x15
#define BNO055_GYR_Y_LSB            0x16
#define BNO055_GYR_Y_MSB            0x17
#define BNO055_GYR_Z_LSB            0x18
#define BNO055_GYR_Z_MSB            0x19
#define BNO055_EUL_H_LSB            0x1A // euler angles
#define BNO055_EUL_H_MSB            0x1B
#define BNO055_EUL_R_LSB            0x1C
#define BNO055_EUL_R_MSB            0x1D
#define BNO055_EUL_P_LSB            0x1E
#define BNO055_EUL_P_MSB            0x1F
#define BNO055_QUA_W_LSB            0x20 // quaternions
#define BNO055_QUA_W_MSB            0x21
#define BNO055_QUA_X_LSB            0x22
#define BNO055_QUA_X_MSB            0x23
#define BNO055_QUA_Y_LSB            0x24
#define BNO055_QUA_Y_MSB            0x25
#define BNO055_QUA_Z_LSB            0x26
#define BNO055_QUA_Z_MSB            0x27
#define BNO055_LIA_X_LSB            0x28 // linear acceleration
#define BNO055_LIA_X_MSB            0x29
#define BNO055_LIA_Y_LSB            0x2A
#define BNO055_LIA_Y_MSB            0x2B
#define BNO055_LIA_Z_LSB            0x2C
#define BNO055_LIA_Z_MSB            0x2D
#define BNO055_GRV_X_LSB            0x2E // gravity vector
#define BNO055_GRV_X_MSB            0x2F
#define BNO055_GRV_Y_LSB            0x30
#define BNO055_GRV_Y_MSB            0x31
#define BNO055_GRV_Z_LSB            0x32
#define BNO055_GRV_Z_MSB            0x33

// Calibration
#define BNO055_CALIB_REG            0x35
#define BNO055_CALIB_SYS_MASK       0xC0
#define BNO055_CALIB_GYR_MASK       0x30
#define BNO055_CALIB_ACC_MASK       0x0C
#define BNO055_CALIB_MAG_MASK       0x03
#define BNO055_CALIB_SYS            0b1000
#define BNO055_CALIB_GYR            0b0100
#define BNO055_CALIB_ACC            0b0010
#define BNO055_CALIB_MAG            0b0001

// Unit selection
#define BNO055_UNIT_RAW             0
#define BNO055_UNIT_DEG             1
#define BNO055_UNIT_RAD             2

class BNO055 {
    private:
        I2C *_i2c;
        DigitalOut *_reset;
        char _address;
        
        int _config;
        bool _calibrated;

        int readReg(char reg, char *res);
        int writeReg(char reg, char val);
    public:
        BNO055(I2C *i2c, DigitalOut *reset, char address);
        
        int init(int config);
        int performCalibration(int calib);
        int getEulerAngles(int angles[3], int unit);
        int getEulerAngles(double angles[3], int unit);
};

#endif