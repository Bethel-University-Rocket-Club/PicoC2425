#ifndef WINDOWAVERAGE_H
#define WINDOWAVERAGE_H
#include "pico/stdlib.h"
class WindowAverage {
    public:
    WindowAverage(uint size, float initialValue);
    float update(float newValue);
    float getAverage();

    private:
    uint windowSize;
    float avgCoef;
    float* window;
    uint replaceIndex;
    float average;
};
#endif