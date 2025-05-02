#ifndef CALCULATOR_H
#define CALCULATOR_H
#include "calcInfo.h"
#include "IIRFilter.h"
#include "common.h"
#include "pico/stdlib.h"

#ifndef CALCULATORMAXSIZE
#define CALCULATORMAXSIZE 4
#endif
class Calculator;
typedef bool (Calculator::*FloatArray3FunctionPtr)(CalcInfo& info, float, double, FloatArray3& out);

class Calculator {
    public:
    int addSensor(FloatArray3FunctionPtr calcFunction);
    bool removeSensor(int identifier);
    bool newSample(int idenfitier, float newMeasurement, double newTimeSeconds, FloatArray3& out);
    bool configureInitialOffset(int identifier, float offset);
    float getInitialOffset(int identifier);
    bool bmp280Calculations(CalcInfo& info, float newAltitude, double newTimeSeconds, FloatArray3& out);
    bool mpu6050Calculations(CalcInfo& info, float newAcceleration, double newTimeSeconds, FloatArray3& out);
    bool gtu7Calculations(CalcInfo& info, float newAltitude, double newTimeSeconds, FloatArray3& out);
    bool mpx5700gpCalculations(CalcInfo& info, float newVelocity, double newTimeSeconds, FloatArray3& out);
    bool setStartTime(double newTimeSeconds);
    private:
    bool getAppropriateInfo(int index, FloatArray3FunctionPtr calcFunction);
    int getNextIdentifier();
    float inline calculateDerivative(float oldMeasurement, float changeRate, double elapsedSeconds);
    float inline calculateIntegral(float measurementBefore, float measurementAfter, double elapsedSeconds);
    float inline secondKinEq(float initialVelocity, float acceleration, double elapsedSeconds);

    FloatArray3FunctionPtr calcFunctions[CALCULATORMAXSIZE] = {};
    CalcInfo sensorInfo[CALCULATORMAXSIZE] = {};
    bool identifiers[CALCULATORMAXSIZE] = {};
};
#endif