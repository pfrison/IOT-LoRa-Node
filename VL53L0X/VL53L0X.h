#ifndef VL53L0X_h
#define VL53L0X_h

#include "mbed.h"

// Error codes
#define VL53L0X_SUCCESS                 0
#define VL53L0X_ERR_READ                1 // Error while reading on ic2
#define VL53L0X_ERR_WRITE               2 // Error while writing on ic2
#define VL53L0X_ERR_CALIBRATION_TIMEOUT 3 // Calibration timeout
#define VL53L0X_ERR_SPAD_TIMEOUT        4 // Calibration timeout
#define VL53L0X_ERR_BUDGET_TOO_LOW      5 // Timing budget is too low
#define VL53L0X_ERR_REQ_TIMEOUT_TOO_BIG 6 // Requested timeout is too big
#define VL53L0X_ERR_VCSEL_PERIOD        7 // Wrong VCSEL period
#define VL53L0X_ERR_VCSEL_TYPE          8 // Wrong VCSEL type
#define VL53L0X_ERR_INTERRUPT_TIMEOUT   9 // Interrupt measurement order timeout
#define VL53L0X_ERR_START_RANGE_TIMEOUT 10 // Start range measurement timeout

// IC2 config
#define VL53L0X_ADDRESS                 0x29

// VL53L0X internal registers
#define REG_IDENTIFICATION_MODEL_ID                 0xc0
#define REG_IDENTIFICATION_REVISION_ID              0xc2
#define REG_SYSRANGE_START                          0x00

#define REG_RESULT_INTERRUPT_STATUS                 0x13
#define RESULT_RANGE_STATUS                         0x14
#define ALGO_PHASECAL_LIM                           0x30
#define ALGO_PHASECAL_CONFIG_TIMEOUT                0x30

#define GLOBAL_CONFIG_VCSEL_WIDTH                   0x32
#define FINAL_RANGE_CONFIG_VALID_PHASE_LOW          0x47
#define FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         0x48

#define PRE_RANGE_CONFIG_VCSEL_PERIOD               0x50
#define PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          0x51
#define PRE_RANGE_CONFIG_VALID_PHASE_LOW            0x56
#define PRE_RANGE_CONFIG_VALID_PHASE_HIGH           0x57

#define REG_MSRC_CONFIG_CONTROL                     0x60
#define FINAL_RANGE_CONFIG_VCSEL_PERIOD             0x70
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        0x71
#define MSRC_CONFIG_TIMEOUT_MACROP                  0x46
#define FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT 0x44
#define SYSRANGE_START                              0x00
#define SYSTEM_SEQUENCE_CONFIG                      0x01
#define SYSTEM_INTERRUPT_CONFIG_GPIO                0x0A
#define RESULT_INTERRUPT_STATUS                     0x13
#define VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           0x89
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_0            0xB0
#define GPIO_HV_MUX_ACTIVE_HIGH                     0x84
#define SYSTEM_INTERRUPT_CLEAR                      0x0B

#define SEQUENCE_ENABLE_FINAL_RANGE                 0x80
#define SEQUENCE_ENABLE_PRE_RANGE                   0x40
#define SEQUENCE_ENABLE_TCC                         0x10
#define SEQUENCE_ENABLE_DSS                         0x08
#define SEQUENCE_ENABLE_MSRC                        0x04

typedef struct tagSequenceStepTimeouts {
    int pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;
    
    int msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
    int msrc_dss_tcc_us,    pre_range_us,    final_range_us;
} SequenceStepTimeouts;

typedef enum vcselperiodtype { VcselPeriodPreRange, VcselPeriodFinalRange } vcselPeriodType;

class VL53L0X {
    private:
        I2C *_i2c;
        char _address;
        
        char _stopVariable;
        int measurement_timing_budget_us;
        
        int readReg(char reg, char *res);
        int readReg16(char reg, int *res);
        int readMulti(char reg, char *res, int length);
        int writeReg(char reg, char val);
        int writeReg16(char reg, int val);
        int writeMulti(char reg, char *val, int length);
        int writeRegList(char *list);
        int getSpadInfo(char *pCount, char *pTypeIsAperture);
        int getSequenceStepTimeouts(char enables, SequenceStepTimeouts *timeouts);
        int setVcselPulsePeriod(vcselPeriodType type, char period_pclks);
        int setMeasurementTimingBudget(int budget_us);
        int getMeasurementTimingBudget(int *budget_us);
        int performSingleRefCalibration(char vhv_init_byte);
        int readRangeContinuousMillimeters(int *range);

    public:
        VL53L0X(I2C *i2c, char address);
        
        int init(bool longRange);
        int readDistance(int *distance);
};

#endif