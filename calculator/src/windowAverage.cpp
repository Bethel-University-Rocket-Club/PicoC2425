#include "windowAverage.h"

WindowAverage::WindowAverage(uint windowSize, float initialValue) {
    this->windowSize = windowSize;
    this->avgCoef = 1.0 / (float)windowSize;
    this->average = initialValue;
    this->replaceIndex = 0;
    this->window = new float[windowSize];
    for(uint i = 0; i < windowSize; i++) {
        window[i] = initialValue*avgCoef;
    }
}

float WindowAverage::update(float newValue) {
    float newVal = newValue*avgCoef;
    average -= window[replaceIndex];
    average += newVal;
    window[replaceIndex] = newVal;
    replaceIndex  = (replaceIndex + 1) % windowSize;
    return average;
}

float WindowAverage::getAverage() {
    return average;
}

bool WindowAverage::reMake(float initialValue) {
    this->replaceIndex = 0;
    this->average = initialValue;
    for(uint i = 0; i < this->windowSize; i++) {
        window[i] = initialValue*avgCoef;
    }
    return true;
}
