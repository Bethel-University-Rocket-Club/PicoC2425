#ifndef BMP280_H
#define BMP280_H
#include "bmp280Settings.h"
#include <hardware/i2c.h>
#include "common.h"

class BMP280;
typedef bool (BMP280::*updateFunctionPtr)();

class BMP280 {
    public:
    BMP280(uint16_t address, i2c_inst_t* i2c);
    updateFunctionPtr setOversampling(byte tempORate, byte presORate);
    updateFunctionPtr setOperatingMode(byte mode);
    updateFunctionPtr setStandbyTime(byte standbyTime);
    updateFunctionPtr setIIRFilter(byte iirFilter);
    bool updateMode();
    byte getOperatingMode();
    FloatArray2 requestSample();
    bool hasSample();
    FloatArray2 readSample();
    float getAltitude(float seaPressure);
    bool softReset();
    bool setDefaults();

    private:
    struct BMP280Calibration;
    bool rawBurstRead(uint16_t startAddr, uint readAmount);
    float compensateTemperature(float rawTemp);
    float compensatePressure(float rawPressure);
    bool calibrate();

    BMP280Calibration* calibrateValues = new BMP280Calibration();
    uint16_t address;
    i2c_inst_t* i2c;
    byte tempORate;
    byte presORate;
    byte mode;
    byte standbyTime;
    byte iirFilter;
    byte rawData[6];

    struct BMP280Calibration {
        //temp params
        uint16_t digT1;
        int16_t digT2;
        int16_t digT3;
        //pressure params
        uint16_t gitP1;
        int16_t digP2;
        int16_t digP3;
        int16_t digP4;
        int16_t digP5;
        int16_t digP6;
        int16_t digP7;
        int16_t digP8;
        int16_t digP9;
    };
};
#endif