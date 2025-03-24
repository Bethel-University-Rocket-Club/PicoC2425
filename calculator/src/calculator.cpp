#include "calculator.h"

int Calculator::addSensor(FloatArray3FunctionPtr calcFunction) {
    int identifier = getNextIdentifier();
    calcFunctions[identifier] = calcFunction;
    return 0; // Placeholder
}

bool Calculator::removeSensor(int identifier) {
    identifiers[identifier] = false;
    return !identifiers[identifier];
}

bool Calculator::newSample(int identifier, float newMeasurement, uint64_t newTime, FloatArray3* out) {
    // Implement new sample processing logic here.
    return false;
}

bool Calculator::configureInitialOffset(int identifier, float offset) {
    // Implement initial offset configuration logic here.
    return false; // Placeholder
}

bool Calculator::bmp280Calculations(CalcInfo* info, float newAltitude, uint64_t newTime, FloatArray3* out) {
    BMP280CalcInfo* bmp280InfoPtr = static_cast<BMP280CalcInfo*>(info->sensorSpecificData);
    // Implement BMP280 calculations here.
    return false;

}

bool Calculator::mpu6050Calculations(CalcInfo* info, float newAcceleration, uint64_t newTime, FloatArray3* out) {
    MPU6050CalcInfo* bmp280InfoPtr = static_cast<MPU6050CalcInfo*>(info->sensorSpecificData);
    // Implement MPU6050 calculations here.
    return false;

}

bool Calculator::gtu7Calculations(CalcInfo* info, float newAltitude, uint64_t newTime, FloatArray3* out) {
    GTU7CalcInfo* bmp280InfoPtr = static_cast<GTU7CalcInfo*>(info->sensorSpecificData);
    // Implement GTU7 calculations here.
    return false;
}

bool Calculator::mpx5700gpCalculations(CalcInfo* info, float newVelocity, uint64_t newTime, FloatArray3* out) {
    MPX5700GPCalcInfo* bmp280InfoPtr = static_cast<MPX5700GPCalcInfo*>(info->sensorSpecificData);
    // Implement MPX5700GP calculations here.
    return false;

}

CalcInfo Calculator::getAppropriateInfo(FloatArray3FunctionPtr calcFunction) {
    CalcInfo info;
    if(calcFunction == &bmp280Calculations) {
        info = {new BMP280CalcInfo(), 0};
    } else if(calcFunction == &mpu6050Calculations) {
        info = {new MPU6050CalcInfo(), 0};
    } else if(calcFunction == &gtu7Calculations) {
        info = {new GTU7CalcInfo, 0};
    } else if(calcFunction == &mpx5700gpCalculations) {
        info = {new MPX5700GPCalcInfo, 0};
    }
    return info;
}

int Calculator::getNextIdentifier() {
    for(uint i = 0; i < CALCULATOR_MAX_SIZE; i++) {
        if(!identifiers[i]) {
            return i;
        }
    }
    return -1;
}

float Calculator::calculateDerivative(float oldMeasurement, float changeRate, float elapsedSeconds) {
    return oldMeasurement + changeRate*elapsedSeconds;
}

float Calculator::calculateIntegral(float measurementBefore, float measurementAfter, float elapsedSeconds) {
    return (measurementAfter - measurementBefore) / elapsedSeconds;
}

/*
returns delta displacement
*/
float Calculator::secondKinEq(float initialVelocity, float acceleration, float elapsedSeconds) {
    return initialVelocity*elapsedSeconds + acceleration*elapsedSeconds*elapsedSeconds;
}