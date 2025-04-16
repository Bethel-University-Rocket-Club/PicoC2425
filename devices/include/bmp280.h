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
    bool readSample(FloatArray2& fa);
    bool getAltitude(float& out);
    bool setSeaPressure(float seaPressure);
    bool softReset();
    bool setDefaults();

    private:
    struct BMP280Calibration;
    struct NewRawData;
    struct OldRawData;
    struct CompensatedData;
    float seaPressure;
    bool check_bmp280_connection();
    bool rawBurstRead(uint8_t startAddr, uint readAmount);
    bool compensateTemperature();
    bool compensatePressure();
    bool calibrate();
    bool contains(const uint8_t* collection, uint8_t size, uint8_t val);

    BMP280Calibration* calibrateValues = new BMP280Calibration();
    uint16_t address;
    i2c_inst_t* i2c;
    int32_t tFine;
    float seaPressureRecip = 0.0;
    byte tempORate = BMP280_OSAMPLE_16;
    byte presORate = BMP280_OSAMPLE_16;
    byte mode = BMP280_MODE_FORCED;
    byte standbyTime = BMP280_STANDBY_0dot5;
    byte iirFilter = BMP280_IIR_FILTER_0;
    byte miscBuf[2];
    byte rawDataBuf[6];
    NewRawData* rawData = new NewRawData();
    OldRawData* oldRawData = new OldRawData();
    CompensatedData* data = new CompensatedData();

    struct NewRawData {
        int32_t pres;
        uint32_t temp;
    };
    struct OldRawData {
        int32_t pres;
        uint32_t temp;
    };
    struct CompensatedData {
        float pres;
        float temp;
        float altitude;
    };

    struct BMP280Calibration {
        //temp params
        uint16_t digT1;
        int16_t digT2;
        int16_t digT3;
        //pressure params
        uint16_t digP1;
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