#include "VL53L0X.h"

#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)
// Encode VCSEL pulse period register value from period in PCLKs
// based on VL53L0X_encode_vcsel_period()
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)

#define MAX_TIMEOUT 50

//
// Register init lists consist of the count followed by register/value pairs
//
char ucI2CMode[] = {4, 0x88,0x00, 0x80,0x01, 0xff,0x01, 0x00,0x00};
char ucI2CMode2[] = {3, 0x00,0x01, 0xff,0x00, 0x80,0x00};
char ucSPAD0[] = {4, 0x80,0x01, 0xff,0x01, 0x00,0x00, 0xff,0x06};
char ucSPAD1[] = {5, 0xff,0x07, 0x81,0x01, 0x80,0x01, 0x94,0x6b, 0x83,0x00};
char ucSPAD2[] = {4, 0xff,0x01, 0x00,0x01, 0xff,0x00, 0x80,0x00};
char ucSPAD[] = {5, 0xff,0x01, 0x4f,0x00, 0x4e,0x2c, 0xff,0x00, 0xb6,0xb4};
char ucDefTuning[] = {80, 0xff,0x01, 0x00,0x00, 0xff,0x00, 0x09,0x00,
        0x10,0x00, 0x11,0x00, 0x24,0x01, 0x25,0xff, 0x75,0x00, 0xff,0x01, 0x4e,0x2c,
        0x48,0x00, 0x30,0x20, 0xff,0x00, 0x30,0x09, 0x54,0x00, 0x31,0x04, 0x32,0x03,
        0x40,0x83, 0x46,0x25, 0x60,0x00, 0x27,0x00, 0x50,0x06, 0x51,0x00, 0x52,0x96,
        0x56,0x08, 0x57,0x30, 0x61,0x00, 0x62,0x00, 0x64,0x00, 0x65,0x00, 0x66,0xa0,
        0xff,0x01, 0x22,0x32, 0x47,0x14, 0x49,0xff, 0x4a,0x00, 0xff,0x00, 0x7a,0x0a,
        0x7b,0x00, 0x78,0x21, 0xff,0x01, 0x23,0x34, 0x42,0x00, 0x44,0xff, 0x45,0x26,
        0x46,0x05, 0x40,0x40, 0x0e,0x06, 0x20,0x1a, 0x43,0x40, 0xff,0x00, 0x34,0x03,
        0x35,0x44, 0xff,0x01, 0x31,0x04, 0x4b,0x09, 0x4c,0x05, 0x4d,0x04, 0xff,0x00,
        0x44,0x00, 0x45,0x20, 0x47,0x08, 0x48,0x28, 0x67,0x00, 0x70,0x04, 0x71,0x01,
        0x72,0xfe, 0x76,0x00, 0x77,0x00, 0xff,0x01, 0x0d,0x01, 0xff,0x00, 0x80,0x01,
        0x01,0xf8, 0xff,0x01, 0x8e,0x01, 0x00,0x01, 0xff,0x00, 0x80,0x00};

// ----- CONSTRUCTORS -----

/**
 * VL53L0X initialisation.
 */
VL53L0X::VL53L0X(I2C *i2c, char address) {
    _i2c = i2c;
    _address = address;
}

// ----- STATIC METHODS -----

// Decode sequence step timeout in MCLKs from register value
// based on VL53L0X_decode_timeout()
// Note: the original function returned a uint32_t, but the return value is
// always stored in a uint16_t.
static int decodeTimeout(int reg_val){
    // format: "(LSByte * 2^MSByte) + 1"
    return (uint16_t)((reg_val & 0x00FF) <<
            (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L0X_encode_timeout()
// Note: the original function took a uint16_t, but the argument passed to it
// is always a uint16_t.
static int encodeTimeout(int timeout_mclks)
{
    // format: "(LSByte * 2^MSByte) + 1"
    uint32_t ls_byte = 0;
    uint16_t ms_byte = 0;
    
    if (timeout_mclks <= 0)
        return 0;
        
    ls_byte = timeout_mclks - 1;
    while ((ls_byte & 0xFFFFFF00) > 0){
        ls_byte >>= 1;
        ms_byte++;
    }
    return (ms_byte << 8) | (ls_byte & 0xFF);
}

// Convert sequence step timeout from MCLKs to microseconds with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_us()
static int timeoutMclksToMicroseconds(int timeout_period_mclks, char vcsel_period_pclks){
    int macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
    return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

// Convert sequence step timeout from microseconds to MCLKs with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_mclks()
static int timeoutMicrosecondsToMclks(int timeout_period_us, char vcsel_period_pclks){
    int macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
    return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

// ----- PRIVATE METHODS -----

/**
 * Read a single byte register
 */
int VL53L0X::readReg(char reg, char *res){
    // init
    char cmd[1];
    cmd[0] = reg;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, 1);
    if(status != 0) return VL53L0X_ERR_WRITE;
    _i2c->stop();
    
    // read
    status = _i2c->read(_address, res, 1, 0);
    if(status != 0) return VL53L0X_ERR_READ;
    
    return VL53L0X_SUCCESS;
}

/**
 * Read two bytes from register
 */
int VL53L0X::readReg16(char reg, int *res){
    // init
    char cmd[1];
    cmd[0] = reg;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, 1);
    if(status != 0) return VL53L0X_ERR_WRITE;
    _i2c->stop();
    
    // read
    char chars[2];
    status = _i2c->read(_address, chars, 2, 0);
    if(status != 0) return VL53L0X_ERR_READ;
    
    *res = (chars[0] << 8) + chars[1];
    
    return VL53L0X_SUCCESS;
}

/**
 * Read multiples bytes from register
 */
int VL53L0X::readMulti(char reg, char *res, int length){
    // init
    char cmd[1];
    cmd[0] = reg;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, 1);
    if(status != 0) return VL53L0X_ERR_WRITE;
    _i2c->stop();
    
    // read
    status = _i2c->read(_address, res, length, 0);
    if(status != 0) return VL53L0X_ERR_READ;
    
    return VL53L0X_SUCCESS;
}

/**
 * Write a single byte register
 */
int VL53L0X::writeReg(char reg, char val){
    // init
    char cmd[2];
    cmd[0] = reg;
    cmd[1] = val;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, 2);
    if(status != 0) return VL53L0X_ERR_WRITE;
    _i2c->stop();
    
    return VL53L0X_SUCCESS;
}

/**
 * Write two bytes from register
 */
int VL53L0X::writeReg16(char reg, int val){
    // init
    char cmd[3];
    cmd[0] = reg;
    cmd[1] = (val >> 8) & 0xFF;
    cmd[2] = val & 0xFF;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, 3);
    if(status != 0) return VL53L0X_ERR_WRITE;
    _i2c->stop();
    
    return VL53L0X_SUCCESS;
}

/**
 * Write multiples bytes from register
 */
int VL53L0X::writeMulti(char reg, char *val, int length){
    // init
    char cmd[length + 1];
    cmd[0] = reg;
    for(int i=0; i<length; i++)
        cmd[i+1] = (val[i] >> (i*8)) & 0xFF;
    
    // write
    _i2c->start();
    int status = _i2c->write(_address, cmd, length + 1);
    if(status != 0) return VL53L0X_ERR_WRITE;
    _i2c->stop();
    
    return VL53L0X_SUCCESS;
}

/**
 * Write list from register
 */
int VL53L0X::writeRegList(char *list){ // list : {length, reg, val, reg, val, ...}
    char length = list[0]; // count is the first element in the list
    
    for(int i=0; i<length; i++){
        int status = writeReg(list[(i*2)+1], list[(i+1)*2]);
        if(status != VL53L0X_SUCCESS)
            return status;
    }
    return VL53L0X_SUCCESS;
}

int VL53L0X::getSpadInfo(char *pCount, char *pTypeIsAperture) {
    int iTimeout;
    char ucTemp;
    
    int status = writeRegList(ucSPAD0);
    if(status != VL53L0X_SUCCESS)
        return status;
    char temp;
    status = readReg(0x83, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0x83, temp | 0x04);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeRegList(ucSPAD1);
    if(status != VL53L0X_SUCCESS)
        return status;
    iTimeout = 0;
    while(iTimeout < MAX_TIMEOUT) {
        status = readReg(0x83, &temp);
        if(status != VL53L0X_SUCCESS)
            return status;
        if (temp != 0x00) break;
        iTimeout++;
        wait(5);
    }
    if (iTimeout == MAX_TIMEOUT)
        return VL53L0X_ERR_SPAD_TIMEOUT;
    
    status = writeReg(0x83,0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    readReg(0x92, &ucTemp);
    if(status != VL53L0X_SUCCESS)
        return status;
    if(status != VL53L0X_SUCCESS)
        return status;
    *pCount = (ucTemp & 0x7f);
    *pTypeIsAperture = (ucTemp & 0x80);
    status = writeReg(0x81, 0x00);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0xff, 0x06);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = readReg(0x83, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0x83, temp & ~0x04);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeRegList(ucSPAD2);
    if(status != VL53L0X_SUCCESS)
        return status;
    
    return 1;
}

int VL53L0X::getSequenceStepTimeouts(char enables, SequenceStepTimeouts *timeouts) {
    char temp;
    int status = readReg(PRE_RANGE_CONFIG_VCSEL_PERIOD, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    timeouts->pre_range_vcsel_period_pclks = ((temp + 1) << 1);
    
    status = readReg(MSRC_CONFIG_TIMEOUT_MACROP, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    timeouts->msrc_dss_tcc_mclks = temp + 1;
    timeouts->msrc_dss_tcc_us = timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks, timeouts->pre_range_vcsel_period_pclks);
    
    int tempInt;
    status = readReg16(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, &tempInt);
    if(status != VL53L0X_SUCCESS)
        return status;
    timeouts->pre_range_mclks = decodeTimeout(tempInt);
    timeouts->pre_range_us = timeoutMclksToMicroseconds(timeouts->pre_range_mclks, timeouts->pre_range_vcsel_period_pclks);
    
    status = readReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    timeouts->final_range_vcsel_period_pclks = ((temp + 1) << 1);
    
    status = readReg16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, &tempInt);
    if(status != VL53L0X_SUCCESS)
        return status;
    timeouts->final_range_mclks = decodeTimeout(tempInt);
    
    if (enables & SEQUENCE_ENABLE_PRE_RANGE)
        timeouts->final_range_mclks -= timeouts->pre_range_mclks;
    
    timeouts->final_range_us = timeoutMclksToMicroseconds(timeouts->final_range_mclks, timeouts->final_range_vcsel_period_pclks);
    return VL53L0X_SUCCESS;
}

// Set the VCSEL (vertical cavity surface emitting laser) pulse period for the
// given period type (pre-range or final range) to the given value in PCLKs.
// Longer periods seem to increase the potential range of the sensor.
// Valid values are (even numbers only):
//  pre:  12 to 18 (initialized default: 14)
//  final: 8 to 14 (initialized default: 10)
// based on VL53L0X_set_vcsel_pulse_period()
int VL53L0X::setVcselPulsePeriod(vcselPeriodType type, char period_pclks){
    char vcsel_period_reg = encodeVcselPeriod(period_pclks);
    
    char enables;
    SequenceStepTimeouts timeouts;
    
    int status = readReg(SYSTEM_SEQUENCE_CONFIG, &enables);
    if(status != VL53L0X_SUCCESS)
        return status;
    getSequenceStepTimeouts(enables, &timeouts);
    
    // "Apply specific settings for the requested clock period"
    // "Re-calculate and apply timeouts, in macro periods"
    
    // "When the VCSEL period for the pre or final range is changed,
    // the corresponding timeout must be read from the device using
    // the current VCSEL period, then the new VCSEL period can be
    // applied. The timeout then must be written back to the device
    // using the new VCSEL period.
    //
    // For the MSRC timeout, the same applies - this timeout being
    // dependant on the pre-range vcsel period."
    
    
    if (type == VcselPeriodPreRange) {
        // "Set phase check limits"
        switch (period_pclks) {
            case 12:
                writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18);
                break;
            case 14:
                writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30);
                break;
            case 16:
                writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40);
                break;
            case 18:
                writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50);
                break;
            default:
                return VL53L0X_ERR_VCSEL_PERIOD;
        }
        status = writeReg(PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
        if(status != VL53L0X_SUCCESS)
            return status;
        
        // apply new VCSEL period
        status = writeReg(PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);
        if(status != VL53L0X_SUCCESS)
            return status;
        
        // update timeouts
        
        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)
        
        int new_pre_range_timeout_mclks = timeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);
        
        status = writeReg16(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, encodeTimeout(new_pre_range_timeout_mclks));
        if(status != VL53L0X_SUCCESS)
            return status;
        
        // set_sequence_step_timeout() end
        
        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)
        
        int new_msrc_timeout_mclks = timeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);
    
        status = writeReg(MSRC_CONFIG_TIMEOUT_MACROP, (new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1));
        if(status != VL53L0X_SUCCESS)
            return status;
        
        // set_sequence_step_timeout() end
    } else if (type == VcselPeriodFinalRange) {
        switch (period_pclks) {
            case 8:
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x01);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_LIM, 0x30);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x00);
                if(status != VL53L0X_SUCCESS)
                    return status;
                break;
            case 10:
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x01);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_LIM, 0x20);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x00);
                if(status != VL53L0X_SUCCESS)
                    return status;
                break;
            case 12:
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x01);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_LIM, 0x20);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x00);
                if(status != VL53L0X_SUCCESS)
                    return status;
                break;
            case 14:
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x01);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(ALGO_PHASECAL_LIM, 0x20);
                if(status != VL53L0X_SUCCESS)
                    return status;
                status = writeReg(0xFF, 0x00);
                if(status != VL53L0X_SUCCESS)
                    return status;
                break;
            default:
                return VL53L0X_ERR_VCSEL_PERIOD;
        }
    
        // apply new VCSEL period
        status = writeReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);
        if(status != VL53L0X_SUCCESS)
            return status;
        
        // update timeouts
        
        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)
        
        // "For the final range timeout, the pre-range timeout
        //  must be added. To do this both final and pre-range
        //  timeouts must be expressed in macro periods MClks
        //  because they have different vcsel periods."
        
        int new_final_range_timeout_mclks = timeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);
        
        if (enables & SEQUENCE_ENABLE_PRE_RANGE)
            new_final_range_timeout_mclks += timeouts.pre_range_mclks;
    
        status = writeReg16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, encodeTimeout(new_final_range_timeout_mclks));
        if(status != VL53L0X_SUCCESS)
            return status;
        
        // set_sequence_step_timeout end
    } else
        return VL53L0X_ERR_VCSEL_TYPE;
    
    // "Finally, the timing budget must be re-applied"
    
    setMeasurementTimingBudget(measurement_timing_budget_us);
    
    // "Perform the phase calibration. This is needed after changing on vcsel period."
    // VL53L0X_perform_phase_calibration() begin
    
    char sequence_config;
    status = readReg(SYSTEM_SEQUENCE_CONFIG, &sequence_config);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    if(status != VL53L0X_SUCCESS)
        return status;
    performSingleRefCalibration(0x0);
    status = writeReg(SYSTEM_SEQUENCE_CONFIG, sequence_config);
    if(status != VL53L0X_SUCCESS)
        return status;
    
    // VL53L0X_perform_phase_calibration() end
    
    return VL53L0X_SUCCESS;
}

// Set the measurement timing budget in microseconds, which is the time allowed
// for one measurement; the ST API and this library take care of splitting the
// timing budget among the sub-steps in the ranging sequence. A longer timing
// budget allows for more accurate measurements. Increasing the budget by a
// factor of N decreases the range measurement standard deviation by a factor of
// sqrt(N). Defaults to about 33 milliseconds; the minimum is 20 ms.
// based on VL53L0X_set_measurement_timing_budget_micro_seconds()
int VL53L0X::setMeasurementTimingBudget(int budget_us) {
    int used_budget_us;
    int final_range_timeout_us;
    int final_range_timeout_mclks;
    
    char enables;
    SequenceStepTimeouts timeouts;
    
    int const StartOverhead      = 1320; // note that this is different than the value in get_
    int const EndOverhead        = 960;
    int const MsrcOverhead       = 660;
    int const TccOverhead        = 590;
    int const DssOverhead        = 690;
    int const PreRangeOverhead   = 660;
    int const FinalRangeOverhead = 550;
    
    int const MinTimingBudget = 20000;
    
    if (budget_us < MinTimingBudget)
        return VL53L0X_ERR_BUDGET_TOO_LOW;
    
    used_budget_us = StartOverhead + EndOverhead;
    
    int status = readReg(SYSTEM_SEQUENCE_CONFIG, &enables);
    if(status != VL53L0X_SUCCESS)
        return status;
    getSequenceStepTimeouts(enables, &timeouts);
    
    if (enables & SEQUENCE_ENABLE_TCC)
        used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
    if (enables & SEQUENCE_ENABLE_DSS)
        used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    else if (enables & SEQUENCE_ENABLE_MSRC)
        used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
    if (enables & SEQUENCE_ENABLE_PRE_RANGE)
        used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
    
    if (enables & SEQUENCE_ENABLE_FINAL_RANGE) {
        used_budget_us += FinalRangeOverhead;
        
        // "Note that the final range timeout is determined by the timing
        // budget and the sum of all other timeouts within the sequence.
        // If there is no room for the final range timeout, then an error
        // will be set. Otherwise the remaining time will be applied to
        // the final range."
        
        if (used_budget_us > budget_us)
            return 0;
        
        final_range_timeout_us = budget_us - used_budget_us;
        
        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)
        
        // "For the final range timeout, the pre-range timeout
        //  must be added. To do this both final and pre-range
        //  timeouts must be expressed in macro periods MClks
        //  because they have different vcsel periods."
        
        final_range_timeout_mclks = timeoutMicrosecondsToMclks(final_range_timeout_us, timeouts.final_range_vcsel_period_pclks);
    
        if (enables & SEQUENCE_ENABLE_PRE_RANGE)
            final_range_timeout_mclks += timeouts.pre_range_mclks;
        
        status = writeReg16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, encodeTimeout(final_range_timeout_mclks));
        if(status != VL53L0X_SUCCESS)
            return status;
        
        // set_sequence_step_timeout() end
        
        measurement_timing_budget_us = budget_us; // store for internal reuse
    }
    return VL53L0X_SUCCESS;
}

int VL53L0X::getMeasurementTimingBudget(int *budget_us) {
    int const StartOverhead      = 1910; // note that this is different than the value in set_
    int const EndOverhead        = 960;
    int const MsrcOverhead       = 660;
    int const TccOverhead        = 590;
    int const DssOverhead        = 690;
    int const PreRangeOverhead   = 660;
    int const FinalRangeOverhead = 550;
    
    // "Start and end overhead times always present"
    *budget_us = StartOverhead + EndOverhead;
    
    char enables;
    int status = readReg(SYSTEM_SEQUENCE_CONFIG, &enables);
    if(status != VL53L0X_SUCCESS)
        return status;
    SequenceStepTimeouts timeouts;
    getSequenceStepTimeouts(enables, &timeouts);
    
    if (enables & SEQUENCE_ENABLE_TCC)
        *budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
    if (enables & SEQUENCE_ENABLE_DSS)
        *budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    else if (enables & SEQUENCE_ENABLE_MSRC)
        *budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
    if (enables & SEQUENCE_ENABLE_PRE_RANGE)
        *budget_us += (timeouts.pre_range_us + PreRangeOverhead);
    if (enables & SEQUENCE_ENABLE_FINAL_RANGE)
        *budget_us += (timeouts.final_range_us + FinalRangeOverhead);
    
    measurement_timing_budget_us = *budget_us; // store for internal reuse
    return VL53L0X_SUCCESS;
}

int VL53L0X::performSingleRefCalibration(char vhv_init_byte){
    int status = writeReg(SYSRANGE_START, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP
    if(status != VL53L0X_SUCCESS)
        return status;
    
    int iTimeout = 0;
    char res;
    status = readReg(RESULT_INTERRUPT_STATUS, &res);
    if(status != VL53L0X_SUCCESS)
        return status;
    while ((res & 0x07) == 0){
        iTimeout++;
        wait(1);
        if (iTimeout > 100) { return VL53L0X_ERR_CALIBRATION_TIMEOUT; }
        
        status = readReg(RESULT_INTERRUPT_STATUS, &res);
        if(status != VL53L0X_SUCCESS)
            return status;
    }
    
    status = writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(SYSRANGE_START, 0x00);
    if(status != VL53L0X_SUCCESS)
        return status;
    return VL53L0X_SUCCESS;
}

int VL53L0X::readRangeContinuousMillimeters(int *range){
    int iTimeout = 0;

    char temp;
    int status = readReg(RESULT_INTERRUPT_STATUS, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    while ((temp & 0x07) == 0) {
        iTimeout++;
        wait(0.1);
        if (iTimeout > 50)
            return VL53L0X_ERR_INTERRUPT_TIMEOUT;
        int status = readReg(RESULT_INTERRUPT_STATUS, &temp);
        if(status != VL53L0X_SUCCESS)
            return status;
    }
    
    // assumptions: Linearity Corrective Gain is 1000 (default);
    // fractional ranging is not enabled
    status = readReg16(RESULT_RANGE_STATUS + 10, range);
    if(status != VL53L0X_SUCCESS)
        return status;
    
    status = writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    return VL53L0X_SUCCESS;
}

// ----- PUBLIC METHODS -----

/**
 * Init : Write configs
 */
int VL53L0X::init(bool longRange){
    char spad_count=0, spad_type_is_aperture=0, ref_spad_map[6];
    char ucFirstSPAD, ucSPADsEnabled;
    char temp;
    
    // set 2.8V mode
    int status = readReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, temp | 0x01); // set bit 0
    if(status != VL53L0X_SUCCESS)
        return status;
    // Set I2C standard mode
    status = writeRegList(ucI2CMode);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = readReg(0x91, &_stopVariable);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeRegList(ucI2CMode2);
    if(status != VL53L0X_SUCCESS)
        return status;
    // disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
    status = readReg(REG_MSRC_CONFIG_CONTROL, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(REG_MSRC_CONFIG_CONTROL, temp | 0x12);
    if(status != VL53L0X_SUCCESS)
        return status;
    // Q9.7 fixed point format (9 integer bits, 7 fractional bits)
    status = writeReg16(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 32); // 0.25
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF);
    if(status != VL53L0X_SUCCESS)
        return status;
    getSpadInfo(&spad_count, &spad_type_is_aperture);
    
    status = readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
    if(status != VL53L0X_SUCCESS)
        return status;
    //printf("initial spad map: %02x,%02x,%02x,%02x,%02x,%02x\n", ref_spad_map[0], ref_spad_map[1], ref_spad_map[2], ref_spad_map[3], ref_spad_map[4], ref_spad_map[5]);
    status = writeRegList(ucSPAD);
    if(status != VL53L0X_SUCCESS)
        return status;
    ucFirstSPAD = (spad_type_is_aperture) ? 12: 0;
    ucSPADsEnabled = 0;
    // clear bits for unused SPADs
    for (int i=0; i<48; i++) {
        if (i < ucFirstSPAD || ucSPADsEnabled == spad_count)
            ref_spad_map[i>>3] &= ~(1<<(i & 7));
        else if (ref_spad_map[i>>3] & (1<< (i & 7)))
            ucSPADsEnabled++;
    } // for i
    status = writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
    if(status != VL53L0X_SUCCESS)
        return status;
    //printf("final spad map: %02x,%02x,%02x,%02x,%02x,%02x\n", ref_spad_map[0], 
    //ref_spad_map[1], ref_spad_map[2], ref_spad_map[3], ref_spad_map[4], ref_spad_map[5]);
    
    // load default tuning settings
    status = writeRegList(ucDefTuning); // long list of magic numbers
    if(status != VL53L0X_SUCCESS)
        return status;
    
    // change some settings for long range mode
    if (longRange) {
        status = writeReg16(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 13); // 0.1
        if(status != VL53L0X_SUCCESS)
            return status;
        setVcselPulsePeriod(VcselPeriodPreRange, 18);
        setVcselPulsePeriod(VcselPeriodFinalRange, 14);
    }
    
    // set interrupt configuration to "new sample ready"
    status = writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = readReg(GPIO_HV_MUX_ACTIVE_HIGH, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(GPIO_HV_MUX_ACTIVE_HIGH, temp & ~0x10); // active low
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = getMeasurementTimingBudget(&measurement_timing_budget_us);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(SYSTEM_SEQUENCE_CONFIG, 0xe8);
    if(status != VL53L0X_SUCCESS)
        return status;
    setMeasurementTimingBudget(measurement_timing_budget_us);
    status = writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    if (!performSingleRefCalibration(0x40)) { return 0; }
    status = writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    if(status != VL53L0X_SUCCESS)
        return status;
    if (!performSingleRefCalibration(0x00)) { return 0; }
    status = writeReg(SYSTEM_SEQUENCE_CONFIG, 0xe8);
    if(status != VL53L0X_SUCCESS)
        return status;
    return 1;
}

//
// Read the current distance in mm
//
int VL53L0X::readDistance(int *distance) {
    int status = writeReg(0x80, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0xFF, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0x00, 0x00);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0x91, _stopVariable);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0x00, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0xFF, 0x00);
    if(status != VL53L0X_SUCCESS)
        return status;
    status = writeReg(0x80, 0x00);
    if(status != VL53L0X_SUCCESS)
        return status;
    
    status = writeReg(SYSRANGE_START, 0x01);
    if(status != VL53L0X_SUCCESS)
        return status;
    
    // "Wait until start bit has been cleared"
    int iTimeout = 0;
    char temp;
    status = readReg(SYSRANGE_START, &temp);
    if(status != VL53L0X_SUCCESS)
        return status;
    while (temp & 0x01) {
        iTimeout++;
        wait(0.1);
        if (iTimeout > 50)
            return VL53L0X_ERR_START_RANGE_TIMEOUT;
        status = readReg(SYSRANGE_START, &temp);
        if(status != VL53L0X_SUCCESS)
            return status;
    }
    
    status = readRangeContinuousMillimeters(distance);
    if(status != VL53L0X_SUCCESS)
        return status;
    return VL53L0X_SUCCESS;
}
