#ifndef IIRFILTER_H
#define IIRFILTER_H
#include "pico/stdlib.h"

class IIRFilter {
    public:
    IIRFilter(float alpha);
    IIRFilter(float alpha, float startVal);
    bool update(float in, float& out);
    bool getValue(float& val);
    private:
    float alpha;
    float value;
};

#endif