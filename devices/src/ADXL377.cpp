#include "ADXL377.h"

ADXL377::ADXL377(byte adcPinX, byte adcPinY, byte adcPinZ) {
    this->adcPinX = adcPinX;
    this->adcPinY = adcPinY;
    this->adcPinZ = adcPinZ;
    if(adcPinX == 26 || adcPinX == 27 || adcPinX == 28) {
        adc_gpio_init(adcPinX);
        xSet = true;
    }
    if(adcPinY == 26 || adcPinY == 27 || adcPinY == 28) {
        adc_gpio_init(adcPinY);
        ySet = true;
    }
    if(adcPinZ == 26 || adcPinZ == 27 || adcPinZ == 28) {
        adc_gpio_init(adcPinZ);
        zSet = true;
    }
    calibrate();
}

bool ADXL377::getAccel(float &accelX, float &accelY, float &accelZ) {
    bool xSuc = getAccelX(accelX);
    bool ySuc = getAccelY(accelY);
    bool zSuc = getAccelZ(accelZ);
    return xSuc && ySuc && zSuc;
}

bool ADXL377::getAccelX(float &accelX) {
    static WindowAverage movingAverage = WindowAverage(512, 0.0);
    if (!zSet) return false;
    adc_select_input(adcPinX - 26);
    uint16_t rawV = adc_read();
    if(rawV == oldRaw[0]) {
        accelX = oldComp[0];
        return false;
    }
    oldRaw[0] = rawV;
    float voltage = rawV * (3.3f / 4096.0f);
    accelX = (voltage/3.3f)*400 - 200;
    accelX -= drift[0];
    accelX = movingAverage.update(accelX);
    oldComp[0] = accelX;
    return true;
}

bool ADXL377::getAccelY(float &accelY) {
    static WindowAverage movingAverage = WindowAverage(512, 0.0);
    if (!ySet) return false;
    adc_select_input(adcPinY - 26);
    uint16_t rawV = adc_read();
    if(rawV == oldRaw[1]) {
        accelY = oldComp[1];
        return false;
    }
    oldRaw[1] = rawV;
    float voltage = rawV * (3.3f / 4096.0f);
    accelY = (voltage/3.3f)*400 - 200;
    accelY -= drift[1];
    accelY = movingAverage.update(accelY);
    oldComp[1] = accelY;
    return true;
}

bool ADXL377::getAccelZ(float &accelZ) {
    static WindowAverage movingAverage = WindowAverage(512, 0.0);
    if (!zSet) return false;
    adc_select_input(adcPinZ - 26);
    uint16_t rawV = adc_read();
    if(rawV == oldRaw[2]) {
        accelZ = oldComp[2];
        return false;
    }
    oldRaw[2] = rawV;
    float voltage = rawV * (3.3f / 4096.0f);
    accelZ = (voltage/3.3f)*400 - 200;
    accelZ -= drift[2];
    accelZ = movingAverage.update(accelZ);
    oldComp[2] = accelZ;
    return true;
}

bool ADXL377::checkConnection() {
    bool x = true;
    bool y = true;
    bool z = true;
    if(xSet) {
        adc_select_input(adcPinX-26);
        uint16_t xVal = adc_read();
        float xVoltage = xVal * (3.3 / 4096.0f);
        x = std::abs(xVoltage - 1.65) < 0.25;
    }
    if(ySet) {
        adc_select_input(adcPinY-26);
        uint16_t yVal = adc_read();
        float yVoltage = yVal * (3.3 / 4096.0f);
        y = std::abs(yVoltage - 1.65) < 0.25;    
    }
    if(zSet) {
        adc_select_input(adcPinZ-26);
        uint16_t zVal = adc_read();
        float zVoltage = zVal * (3.3 / 4096.0f);
        z = std::abs(zVoltage - 1.65) < 0.25;    
    }
    return x && y && z;
}

bool ADXL377::calibrate() {
    float x;
    float y;
    float z;
    drift[0] = 0.0;
    drift[1] = 0.0;
    drift[2] = 0.0;
    tempDrift[0] = 0.0;
    tempDrift[1] = 0.0;
    tempDrift[2] = 0.0;
    for(int i = 0; i < 1000; i++) {
        sleep_ms(10);
        getRawAccel(x, y, z);
            tempDrift[0] += x / 1000.0;
            tempDrift[1] += y / 1000.0;
            tempDrift[2] += z / 1000.0;
    }
    drift[0] = tempDrift[0];
    drift[1] = tempDrift[1];
    drift[2] = tempDrift[2];
    return true;
}

bool ADXL377::getRawAccel(float &accelX, float &accelY, float &accelZ) {
    bool xSuc = getRawAccelX(accelX);
    bool ySuc = getRawAccelY(accelY);
    bool zSuc = getRawAccelZ(accelZ);
    return xSuc && ySuc && zSuc;
}

bool ADXL377::getRawAccelX(float &accelX) {
    if (!xSet) return false;
    adc_select_input(adcPinX - 26);
    uint16_t rawV = adc_read();
    float voltage = rawV * (3.3f / 4096.0f);
    accelX = (voltage/3.3f)*400 - 200;
    return true;
}

bool ADXL377::getRawAccelY(float &accelY) {
    if (!ySet) return false;
    adc_select_input(adcPinY - 26);
    uint16_t rawV = adc_read();
    float voltage = rawV * (3.3f / 4096.0f);
    accelY = (voltage/3.3f)*400 - 200;
    return true;
}

bool ADXL377::getRawAccelZ(float &accelZ) {
    if (!zSet) return false;
    adc_select_input(adcPinZ - 26);
    uint16_t rawV = adc_read();
    float voltage = rawV * (3.3f / 4096.0f);
    accelZ = (voltage/3.3f)*400 - 200;
    return true;
}
