#ifndef MPX5700GP_H
#define MPX5700GP_H
#include "common.h"

class MPX5700GP {
    public:
    MPX5700GP(byte adcPin);
    float getVelocity();
};
#endif