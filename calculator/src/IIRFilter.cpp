#include "IIRFilter.h"

IIRFilter::IIRFilter(float alpha) {
    this->alpha = alpha;
}

IIRFilter::IIRFilter(float alpha, float startVal) {
    this->alpha = alpha;
    this->value = startVal;
}

bool IIRFilter::update(float in, float &out)
{
    out = alpha * in + (1.0f - alpha) * value;
    value = out;
    return true;
}

bool IIRFilter::getValue(float &val)
{
    val = value;
    return true;
}
