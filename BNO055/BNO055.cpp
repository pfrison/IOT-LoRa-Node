#include "BNO055.h"


// ----- CONSTRUCTORS -----

/**
 * BNO055 initialisation.
 */
BNO055::BNO055(I2C *i2c, DigitalOut *reset, char address) {
    _i2c = i2c;
    _reset = reset;
    _address = address;
    
    _config = 0;
    _calibrated = false;
}

// ----- PRIVATE METHODS -----

/**
 * Read a signle bit register
 */
int BNO055::readReg(char reg, char *res){
    // init
    char cmd[1];
    cmd[0] = reg;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, 1);
    if(status != 0) return BNO055_ERR_WRITE;
    _i2c->stop();
    
    // read
    status = _i2c->read(_address, res, 1, 0);
    if(status != 0) return BNO055_ERR_READ;
    
    return BNO055_SUCCESS;
}

/**
 * Write a signle bit register
 */
int BNO055::writeReg(char reg, char val){
    // init
    char cmd[2];
    cmd[0] = reg;
    cmd[1] = val;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, 2);
    if(status != 0) return BNO055_ERR_WRITE;
    _i2c->stop();
    
    // test
    char res = 0x00;
    status = readReg(reg, &res);
    if(status != 0) return status;
    if(res != val) return BNO055_WARN_FORBIDDEN;
    
    return BNO055_SUCCESS;
}

// ----- PUBLIC METHODS -----

/**
 * Init : Activate reset pin and wait for reboot + write config
 */
int BNO055::init(int config){
    // test if config is correct
    if((config & 0x0F) > BNO055_CONFIG_NDOF)
        return BNO055_ERR_WRONG_CONFIG;
    
    // reset
    *_reset = 0;
    wait(0.5);
    *_reset = 1;
    wait(1);
    
    // no longer calibrated
    _calibrated = false;
    
    // write config
    _config = 0x0F & config;
    return writeReg(BNO055_CONFIG_REG, _config);
    
}

/**
 * Instruments calibrations.
 * 
 * To calibrate magnetometer, call this function and write a '8' on
 * air with the sensor in your hand.
 * 
 * @param calib is used to choose which instrument to calibrate.
 *  Use BNO055_CALIB_SYS, BNO055_CALIB_GYR, BNO055_CALIB_ACC and BNO055_CALIB_MAG
 * 
 * Must be called before using COMPASS, M4G or NDOF_FMC_OFF modes
 */
int BNO055::performCalibration(int calib){
    // test if we are in compass mode
    if(_config != BNO055_CONFIG_COMPASS)
        return BNO055_ERR_WRONG_CONFIG;
    
    // calibration
    _calibrated = false;
    while(!_calibrated){
        char res = 0x00;
        int status = readReg(BNO055_CALIB_REG, &res);
        if(status != BNO055_SUCCESS) return status;
        
        // review every instrument calibration according to the selection in calib
        _calibrated = true;
        if((calib & BNO055_CALIB_GYR) != 0)
            _calibrated &= (res & BNO055_CALIB_GYR_MASK) == BNO055_CALIB_GYR_MASK;
        if((calib & BNO055_CALIB_ACC) != 0)
            _calibrated &= (res & BNO055_CALIB_ACC_MASK) == BNO055_CALIB_ACC_MASK;
        if((calib & BNO055_CALIB_MAG) != 0)
            _calibrated &= (res & BNO055_CALIB_MAG_MASK) == BNO055_CALIB_MAG_MASK;
        if((calib & BNO055_CALIB_SYS) != 0)
            _calibrated &= (res & BNO055_CALIB_SYS_MASK) == BNO055_CALIB_SYS_MASK;
        
        wait(0.1);
    }
    
    return BNO055_SUCCESS;
}

/**
 * Get the euler angles in a table (raw, degrees or radians values) : [pitch, roll, head]
 */
int BNO055::getEulerAngles(int angles[3], int unit){
    // test the config
    if(_config != BNO055_CONFIG_COMPASS
            && _config != BNO055_CONFIG_NDOF_FMC_OFF
            && _config != BNO055_CONFIG_NDOF)
        return BNO055_ERR_WRONG_CONFIG;
    // test unit selection
    if(unit != BNO055_UNIT_RAW)
        return BNO055_ERR_WRONG_UNIT;
    
    // pitch
    char res1 = 0x00;
    char res2 = 0x00;
    int status = readReg(BNO055_EUL_P_MSB, &res1);
    if(status != BNO055_SUCCESS) return status;
    status = readReg(BNO055_EUL_P_LSB, &res2);
    if(status != BNO055_SUCCESS) return status;
    if(res1 >> 7 == 1) // negative
        angles[0] = (((res1 & 0x7F) << 8) + res2) - 0x7FFF;
    else // positive
        angles[0] = ((res1 & 0x7F) << 8) + res2;
    
    // roll
    res1 = 0x00;
    res2 = 0x00;
    status = readReg(BNO055_EUL_R_MSB, &res1);
    if(status != BNO055_SUCCESS) return status;
    status = readReg(BNO055_EUL_R_LSB, &res2);
    if(status != BNO055_SUCCESS) return status;
    if(res1 >> 7 == 1) // negative
        angles[1] = (((res1 & 0x7F) << 8) + res2) - 0x7FFF;
    else // positive
        angles[1] = ((res1 & 0x7F) << 8) + res2;
    
    // head
    res1 = 0x00;
    res2 = 0x00;
    status = readReg(BNO055_EUL_H_MSB, &res1);
    if(status != BNO055_SUCCESS) return status;
    status = readReg(BNO055_EUL_H_LSB, &res2);
    if(status != BNO055_SUCCESS) return status;
    if(res1 >> 7 == 1) // negative
        angles[2] = (((res1 & 0x7F) << 8) + res2) - 0x7FFF;
    else // positive
        angles[2] = ((res1 & 0x7F) << 8) + res2;
    
    if(_calibrated)
        return BNO055_SUCCESS;
    return BNO055_WARN_NOT_CALIBRATED;
}
// function used for convertion to degrees or radians
int BNO055::getEulerAngles(double angles[3], int unit){
    // test unit selection
    if(unit != BNO055_UNIT_RAW
    // note that raw unit here is deprecated, use getEulerAngles(int[], int) instead
            && unit != BNO055_UNIT_DEG
            && unit != BNO055_UNIT_RAD)
        return BNO055_ERR_WRONG_UNIT;
    
    // get raw angles
    int _angles[3];
    int status = getEulerAngles(_angles, BNO055_UNIT_RAW);
    if(status != BNO055_SUCCESS
            && status != BNO055_WARN_NOT_CALIBRATED)
        return status;
    
    // unit convertion
    if(unit == BNO055_UNIT_RAW) ; // do nothing
    else if(unit == BNO055_UNIT_DEG) {
        for(int i=0; i<3; i++)
            angles[i] = (double) _angles[i] / 16;
    } else if(unit == BNO055_UNIT_RAD) {
        for(int i=0; i<3; i++)
            angles[i] = (double) _angles[i] / 900;
    }
    
    return status;
}
