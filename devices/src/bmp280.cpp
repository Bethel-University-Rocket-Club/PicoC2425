#include "bmp280.h"

BMP280::BMP280(uint16_t address, i2c_inst_t *i2c) {
    this->address = address;
    this->i2c = i2c;
    /*while(!check_bmp280_connection()) {
        tight_loop_contents;
    }*/
    calibrate();
    setDefaults();
}

bool BMP280::check_bmp280_connection() {
    // Attempt to write 0 bytes to the BMP280 (probe address)
    uint8_t dummy = 0;
    int ret = i2c_write_blocking(i2c, address, &dummy, 1, false);
    sleep_ms(100);
    printf("written:%d\n", ret);
    if (ret >= 0) {
        printf("BMP280 detected at 0x%02X\n", address);
        return true;
    } else {
        printf("No response at 0x%02X, error: %d\n", address, ret);
        return false;
    }
}

updateFunctionPtr BMP280::setOversampling(byte tempORate, byte presORate) {
    if(!contains(&oSamples[0], 6, tempORate)) {
        return &BMP280::updateMode;
    }
    if(!contains(&oSamples[0], 6, presORate)) {
        return &BMP280::updateMode;
    }
    this->tempORate = tempORate;
    this->presORate = presORate;
    return &BMP280::updateMode;
}

updateFunctionPtr BMP280::setOperatingMode(byte mode) {
    if(!contains(&operatingModes[0], 3, mode)) {
        return &BMP280::updateMode;
    }
    this->mode = mode;
    return &BMP280::updateMode;
}

updateFunctionPtr BMP280::setStandbyTime(byte standbyTime) {
    if(!contains(&standbyTimes[0], 8, standbyTime)) {
        return &BMP280::updateMode;
    }
    this->standbyTime = standbyTime;
    return &BMP280::updateMode;
}

updateFunctionPtr BMP280::setIIRFilter(byte iirFilter) {
    if(!contains(&IIRFilters[0], 5, iirFilter)) {
        return &BMP280::updateMode;
    }
    this->iirFilter = iirFilter;
    return &BMP280::updateMode;
}

bool BMP280::updateMode() {
    byte config = (standbyTime << 5) | (iirFilter << 2);
    byte control = (tempORate << 5) | (presORate << 2) | mode;
    writeI2C(i2c, address, BMP280_REGISTER_CONFIG, 1, &config);
    writeI2C(i2c, address, BMP280_REGISTER_CONTROL, 1, &control);
    return true;
}

byte BMP280::getOperatingMode() {
    return mode;
}

FloatArray2 BMP280::requestSample() {
    byte control = (tempORate << 5) | (presORate << 2) | 1;
    writeI2C(i2c, address, BMP280_REGISTER_CONTROL, 1, &control);
    rawBurstRead(BMP280_REGISTER_PRESSURE_DATA, 6);
    while(!hasSample()) {
        tight_loop_contents;
    }
    compensateTemperature();
    compensatePressure();
    return {rawData->pres *(float)0.01, rawData->temp * (float)0.01};
}

bool BMP280::hasSample() {
    readI2C(i2c, address, BMP280_REGISTER_STATUS, 2, &miscBuf[0]);
    return *(uint16_t *)&miscBuf[0] & 0b00000100;
}

bool BMP280::readSample(FloatArray2& fa) {
    if(rawBurstRead(BMP280_REGISTER_PRESSURE_DATA, 6)) {
        compensateTemperature();
        compensatePressure();
        data->pres = rawData->pres * (float)0.01;
        data->temp = rawData->temp * (float)0.01;
        fa.values[0] = data->pres;
        fa.values[1] = data->temp;
        return true;
    }
    fa.values[0] = data->pres;
    fa.values[1] = data->temp;
    return false;
}

bool BMP280::getAltitude(float& out) {
    if(rawBurstRead(BMP280_REGISTER_PRESSURE_DATA, 6)) {
        compensateTemperature();
        compensatePressure();
        data->altitude = 44330 * (1 - std::pow((rawData->pres * seaPressureRecip * (float)0.01), (float)0.1903));
        out = data->altitude;
        return true;
    }
    out = data->altitude;
    return false;
}

bool BMP280::setSeaPressure(float seaPressure) {
    seaPressureRecip = 1.0/seaPressure;
    return true;
}

bool BMP280::softReset() {
    byte val = 0xe0;
    return writeI2C(i2c, address, BMP280_REGISTER_SOFTRESET, 1, &val);
}

bool BMP280::setDefaults() {
    setOversampling(BMP280_OSAMPLE_1, BMP280_OSAMPLE_2);
    setOperatingMode(BMP280_MODE_NORMAL);
    setIIRFilter(BMP280_IIR_FILTER_2);
    setStandbyTime(BMP280_STANDBY_0dot5);
    return updateMode();
}

bool BMP280::rawBurstRead(uint8_t startAddr, uint readAmount) {
    #ifdef BMP280DMA
    return false;
    #else
    readI2C(i2c, address, startAddr, readAmount, &rawDataBuf[0]);
    rawData->pres = (rawDataBuf[0] << 12) | (rawDataBuf[1] << 4) | (rawDataBuf[2] >> 4);
    rawData->temp = (rawDataBuf[3] << 12) | (rawDataBuf[4] << 4) | (rawDataBuf[5] >> 4);
    if(rawData->temp != oldRawData->temp || rawData->pres != oldRawData->pres) {
        oldRawData->temp = rawData->temp;
        oldRawData->pres = rawData->pres;
        return true;
    }
    return false;
    #endif
}

bool BMP280::compensateTemperature() {

    int32_t var1 = ((rawData->temp >> 3) - (calibrateValues->digT1 << 1)) * (calibrateValues->digT2 >> 11);
    int32_t var2 = (int32_t)((((((rawData->temp >> 4) - calibrateValues->digT1) * ((rawData->temp >> 4) - calibrateValues->digT1)) >> 12) * calibrateValues->digT3)) >> 14;
    tFine = var1 + var2;
    rawData->temp = (tFine * 5 + 128) >> 8;
    return true;
}

bool BMP280::compensatePressure() {
    int32_t var1 = (tFine>>1) - (int32_t)64000;
    int32_t var2 = (((var1>>2) * (var1>>2))>>11) * (int32_t)calibrateValues->digP6;
    var2 = var2 + ((var1 * (int32_t)calibrateValues->digP5) << 1);
    var2 = (var2>>2) + ((int32_t)calibrateValues->digP4 << 16);
    var1 = (((calibrateValues->digP3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)calibrateValues->digP2) * var1)>>1))>>18;
    var1 =((((32768+var1))*((int32_t)calibrateValues->digP1))>>15);
    if(var1 == 0){
        return 0;
    }
    uint32_t p = ((((uint32_t)((int32_t)1048576)-rawData->pres)-(var2>>12)))*3125;
    if (p < 0x80000000) {
        p = (p << 1) / ((uint32_t)var1);
    } else {
        p = (p / (uint32_t)var1) << 1;
    }   
    var1 = (((int32_t)calibrateValues->digP9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
    var2 = (((int32_t)(p>>2)) * ((int32_t)calibrateValues->digP8))>>13;
    rawData->pres = (uint32_t)((int32_t)p + ((var1 + var2 + calibrateValues->digP7) >> 4));
    return true;
}

bool BMP280::calibrate() {
    #ifdef BMP280DMA
    return false;
    #else
    readI2C(i2c, address, BMP280_REGISTER_DIG_T1, 2, &miscBuf[0]);
    calibrateValues->digT1 = LEbytesToUShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_T2, 2, &miscBuf[0]);
    calibrateValues->digT2 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_T3, 2, &miscBuf[0]);
    calibrateValues->digT3 = LEbytesToShort(&miscBuf[0]);

    readI2C(i2c, address, BMP280_REGISTER_DIG_P1, 2, &miscBuf[0]);
    calibrateValues->digP1 = LEbytesToUShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P2, 2, &miscBuf[0]);
    calibrateValues->digP2 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P3, 2, &miscBuf[0]);
    calibrateValues->digP3 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P4, 2, &miscBuf[0]);
    calibrateValues->digP4 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P5, 2, &miscBuf[0]);
    calibrateValues->digP5 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P6, 2, &miscBuf[0]);
    calibrateValues->digP6 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P7, 2, &miscBuf[0]);
    calibrateValues->digP7 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P8, 2, &miscBuf[0]);
    calibrateValues->digP8 = LEbytesToShort(&miscBuf[0]);
    readI2C(i2c, address, BMP280_REGISTER_DIG_P9, 2, &miscBuf[0]);
    calibrateValues->digP9 = LEbytesToShort(&miscBuf[0]);
    return true;
    #endif
}

bool BMP280::contains(const uint8_t *collection, uint8_t size, uint8_t val)
{
    for(int i = 0; i < size; i++) {
        if(collection[i] == val) return true;
    }
    return false;
}
