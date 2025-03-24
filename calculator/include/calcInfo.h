#ifndef CALCINFO_H
#define CALCINFO_H
#include "windowAverage.h"
struct CalcInfo {
    void* sensorSpecificData;
    uint64_t timeInfo;
};
struct BMP280CalcInfo{
    WindowAverage* recentAltitudeValues;
    float pastVelocity;
    float offsetInfo;
};
struct MPU6050CalcInfo{
    float runningAltitude;
    float runningSpeed;
};
struct GTU7CalcInfo{
    float pastAltitude;
    float pastVelocity;
    float offsetInfo;
};
struct MPX5700GPCalcInfo{
    float runningAltitude;
    float pastVelocity;
};
#endif