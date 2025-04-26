#ifndef CALCINFO_H
#define CALCINFO_H
#include "IIRFilter.h"
#include "windowAverage.h"
struct CalcInfo {
    void* sensorSpecificData;
    double timeInfo;
    enum {BMP280CalcInfo, MPU6050CalcInfo, GTU7CalcInfo, MPX5700GPCalcInfo} type;
};
struct BMP280CalcInfo{
    //IIRFilter* recentAltitudeValues;
    WindowAverage* recentAltitudeValues;
    float pastVelocity;
    float offsetInfo;
};
struct MPU6050CalcInfo{
    float runningAltitude;
    float runningVelocity;
};
struct GTU7CalcInfo{
    float pastAltitude;
    float pastVelocity;
    float offsetInfo;
};
struct MPX5700GPCalcInfo{
    //float recentVelocity;
    IIRFilter* recentVelocityValues;
    float runningAltitude;
    float offsetInfo;
};
#endif