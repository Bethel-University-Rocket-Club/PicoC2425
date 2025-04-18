#include "mpx5700gp.h"
#include <stdio.h>

MPX5700GP::MPX5700GP(byte adcPin) {
    this->adcPin = adcPin;
    adc_init();
    adc_gpio_init(adcPin);
    adc_select_input(adcPin - 26);
    adc_set_clkdiv(0);
}

bool MPX5700GP::getVelocity(float& velocity) {
    adc_select_input(adcPin - 26);
    static const float convFactor = 3.3f / (1 << 12);
    uint16_t raw = adc_read();
    if(abs((int)raw - (int)oldRaw) >= 5) {
        oldRaw = raw;
        float vOut = raw * convFactor;
        float p = (vOut/5.0-0.04)*777.7259294; //kpa
        p *= 1000; //pa
        p = p * (p > 0);
        velocity = std::sqrt(2*p*airDensityRecip);
        oldVel = velocity;
        return true;
    }
    velocity = oldVel;
    return false;
}

void MPX5700GP::setAirDensity(float density) {
    this->airDensityRecip = 1.0/density;
}

bool MPX5700GP::getDrift(float &drift)
{
    float temp = 0.0;
    for(int i = 0; i < 10000; i++) {
        sleep_us(50);
        getVelocity(temp);
        drift += temp*0.0001;
    }
    return true;
}
