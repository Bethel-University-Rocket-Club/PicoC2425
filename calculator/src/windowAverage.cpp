#include "windowAverage.h"

WindowAverage::WindowAverage(uint windowSize, float initialValue) {
    this->windowSize = windowSize;
    this->avgCoef = 1.0 / windowSize;
    this->replaceIndex = 0;
    this->window = new float[windowSize];
    for(uint i = 0; i < windowSize; i++) {
        window[i] = initialValue*avgCoef;
    }
}

float WindowAverage::update(float newValue) {
    float newVal = newVal*avgCoef;
    average -= window[replaceIndex];
    average += newVal;
    window[replaceIndex] = newVal;
    replaceIndex  = (replaceIndex + 1) % windowSize;
}

float WindowAverage::getAverage() {
    return average;
}