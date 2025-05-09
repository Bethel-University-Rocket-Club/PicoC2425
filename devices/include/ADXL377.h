#ifndef ADXL377_H
#define ADXL377_h
#include "common.h"
#include "hardware/adc.h"
#include "windowAverage.h"

class ADXL377 {
    public:
    ADXL377(byte adcPinX, byte adcPinY, byte adcPinZ);
    bool getAccel(float &accelX, float &accelY, float &accelZ);
    bool getAccelX(float &accelX);
    bool getAccelY(float &accelY);
    bool getAccelZ(float &accelZ);
    bool checkConnection();
    bool calibrate();

    private:
    bool getRawAccel(float &accelX, float &accelY, float &accelZ);
    bool getRawAccelX(float &accelX);
    bool getRawAccelY(float &accelY);
    bool getRawAccelZ(float &accelZ);
    byte adcPinX;
    byte adcPinY;
    byte adcPinZ;
    bool xSet = false;
    bool ySet = false;
    bool zSet = false;
    float drift[3] = {0.0, 0.0, 0.0}; //x, y, z
    float tempDrift[3] = {0.0, 0.0, 0.0};
};

#endif