#ifndef MPX5700GP_H
#define MPX5700GP_H
#include "common.h"
#include "hardware/adc.h"

class MPX5700GP {
    public:
    MPX5700GP(byte adcPin);
    bool getVelocity(float& velocity);
    void setAirDensity(float density);
    bool getDrift(float& drift);
    bool checkConnection();
    bool getZeroVoltage(float& zeroV);

    private:
    uint16_t oldRaw = 0;
    float oldVel = 0;
    byte adcPin;
    float airDensityRecip;
    float zeroVal;
};
#endif