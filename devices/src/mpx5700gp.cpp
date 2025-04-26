#include "mpx5700gp.h"
#include <stdio.h>
#include "windowAverage.h"

MPX5700GP::MPX5700GP(byte adcPin) {
    this->adcPin = adcPin;
    adc_init();
    adc_gpio_init(adcPin);
    adc_select_input(adcPin - 26);
    adc_set_clkdiv(0);
    getZeroVoltage(zeroVal);
    zeroVal *= 0.2;
}

bool MPX5700GP::getVelocity(float& velocity) {
    static WindowAverage movingAverage = WindowAverage(520, 0.2);
    static const float convFactor = 3.3f / (1 << 12);
    adc_select_input(adcPin - 26);
    uint16_t raw = adc_read();
    float vOut = movingAverage.update(raw * convFactor);
    //printf("voltage: %f\n", vOut);
    float p = (vOut/5.0-zeroVal)*777.7259294; //kpa
    //printf("kpa: %f\n", p);
    p *= 1000; //pa
    if(p < 0) {
        p *= -1;
    }
    velocity = std::sqrt(2*p*airDensityRecip);
    return true;
}

void MPX5700GP::setAirDensity(float density) {
    this->airDensityRecip = 1.0/density;
}

bool MPX5700GP::getDrift(float &drift)
{
    float temp = 0.0;
    double tDrift = 0.0;
    for(int i = 0; i < 100000; i++) {
        sleep_us(50);
        getVelocity(temp);
        tDrift += temp*0.00001;
    }
    drift = (float)tDrift;
    return true;
}

bool MPX5700GP::checkConnection()
{
    adc_select_input(adcPin - 26);
    uint16_t raw = adc_read();
    return abs(raw - 250) < 10;
}

bool MPX5700GP::getZeroVoltage(float &zeroV) {
    static const float convFactor = 3.3f / (1 << 12);
    adc_select_input(adcPin - 26);
    uint16_t raw = adc_read();
    double zV = 0.0;
    for(int i = 0; i < 100000; i++) {
        sleep_us(50);
        double vOut = raw * convFactor;
        zV += vOut*0.00001;
    }
    zeroV = (float) zV;
    return true;
}
