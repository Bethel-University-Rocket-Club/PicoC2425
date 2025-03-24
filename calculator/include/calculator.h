#ifndef CALCULATOR_H
#define CALCULATOR_H
#include "calcInfo.h"
#include "windowAverage.h"
#include "common.h"

#define CALCULATOR_MAX_SIZE 4
class Calculator;
typedef bool (Calculator::*FloatArray3FunctionPtr)(CalcInfo* info, float, uint64_t, FloatArray3* out);

class Calculator {
    public:
    int addSensor(FloatArray3FunctionPtr calcFunction);
    bool removeSensor(int identifier);
    bool newSample(int idenfitier, float newMeasurement, uint64_t newTime, FloatArray3* out);
    bool configureInitialOffset(int identifier, float offset);
    bool bmp280Calculations(CalcInfo* info, float newAltitude, uint64_t newTime, FloatArray3* out);
    bool mpu6050Calculations(CalcInfo* info, float newAcceleration, uint64_t newTime, FloatArray3* out);
    bool gtu7Calculations(CalcInfo* info, float newAltitude, uint64_t newTime, FloatArray3* out);
    bool mpx5700gpCalculations(CalcInfo* info, float newVelocity, uint64_t newTime, FloatArray3* out);
    private:
    CalcInfo getAppropriateInfo(FloatArray3FunctionPtr calcFunction);
    int getNextIdentifier();
    float inline calculateDerivative(float oldMeasurement, float changeRate, float elapsedSeconds);
    float inline calculateIntegral(float measurementBefore, float measurementAfter, float elapsedSeconds);
    float inline secondKinEq(float initialVelocity, float acceleration, float elapsedSeconds);

    FloatArray3FunctionPtr calcFunctions[CALCULATOR_MAX_SIZE];
    CalcInfo sensorInfo[CALCULATOR_MAX_SIZE];
    bool identifiers[CALCULATOR_MAX_SIZE];
};
#endif