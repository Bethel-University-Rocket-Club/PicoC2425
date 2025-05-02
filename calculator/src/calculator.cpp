#include "calculator.h"

int Calculator::addSensor(FloatArray3FunctionPtr calcFunction) {
    int identifier = getNextIdentifier();
    calcFunctions[identifier] = calcFunction;
    getAppropriateInfo(identifier, calcFunction);
    identifiers[identifier] = true;
    return identifier;
}

bool Calculator::removeSensor(int identifier) {
    identifiers[identifier] = false;
    return !identifiers[identifier];
}

bool Calculator::newSample(int identifier, float newMeasurement, double newTime, FloatArray3& out) {
    return (this->*calcFunctions[identifier])(sensorInfo[identifier], newMeasurement, newTime, out);
}

float Calculator::getInitialOffset(int identifier) {
    if(sensorInfo[identifier].type == CalcInfo::BMP280CalcInfo) {
        BMP280CalcInfo* bmp280InfoPtr = static_cast<BMP280CalcInfo*>(sensorInfo[identifier].sensorSpecificData);
        return bmp280InfoPtr->offsetInfo;
    } else if(sensorInfo[identifier].type == CalcInfo::GTU7CalcInfo) {
        GTU7CalcInfo* gtu7InfoPtr = static_cast<GTU7CalcInfo*>(sensorInfo[identifier].sensorSpecificData);
        return gtu7InfoPtr->offsetInfo;
    } else if(sensorInfo[identifier].type == CalcInfo::MPX5700GPCalcInfo) {
        MPX5700GPCalcInfo* mpx5700gpInfoPtr = static_cast<MPX5700GPCalcInfo*>(sensorInfo[identifier].sensorSpecificData);
        return mpx5700gpInfoPtr->offsetInfo;
    }
    //not a type that has offset info
    return 0.0;
}

bool Calculator::configureInitialOffset(int identifier, float offset) {
    if(sensorInfo[identifier].type == CalcInfo::BMP280CalcInfo) {
        BMP280CalcInfo* bmp280InfoPtr = static_cast<BMP280CalcInfo*>(sensorInfo[identifier].sensorSpecificData);
        bmp280InfoPtr->offsetInfo = offset;
        return true;
    } else if(sensorInfo[identifier].type == CalcInfo::GTU7CalcInfo) {
        GTU7CalcInfo* gtu7InfoPtr = static_cast<GTU7CalcInfo*>(sensorInfo[identifier].sensorSpecificData);
        gtu7InfoPtr->offsetInfo = offset;
        return true;
    } else if(sensorInfo[identifier].type == CalcInfo::MPX5700GPCalcInfo) {
        MPX5700GPCalcInfo* mpx5700gpInfoPtr = static_cast<MPX5700GPCalcInfo*>(sensorInfo[identifier].sensorSpecificData);
        mpx5700gpInfoPtr->offsetInfo = offset;
        return true;
    }
    //not a type that has offset info
    return false;
}

bool Calculator::bmp280Calculations(CalcInfo& info, float newAltitude, double newTime, FloatArray3& out) {
    BMP280CalcInfo* bmp280InfoPtr = static_cast<BMP280CalcInfo*>(info.sensorSpecificData);
    double elapsedTime = newTime - info.timeInfo;
    newAltitude -= bmp280InfoPtr->offsetInfo;
    float initAvg = bmp280InfoPtr->recentAltitudeValues->getAverage();
    out.values[0] = bmp280InfoPtr->recentAltitudeValues->update(newAltitude);
    ///bmp280InfoPtr->recentAltitudeValues->getValue(initAvg);
    //updated value gets put in out->values[0]
    //bmp280InfoPtr->recentAltitudeValues->update(newAltitude, out.values[0]);
    out.values[1] = calculateDerivative(initAvg, out.values[0], elapsedTime);
    out.values[2] = calculateDerivative(bmp280InfoPtr->pastVelocity, out.values[1], elapsedTime);
    bmp280InfoPtr->pastVelocity = out.values[1];
    info.timeInfo = newTime;
    return true;
}

bool Calculator::mpu6050Calculations(CalcInfo& info, float newAcceleration, double newTime, FloatArray3& out) {
    MPU6050CalcInfo* mpu6050InfoPtr = static_cast<MPU6050CalcInfo*>(info.sensorSpecificData);
    double elapsedTime = newTime - info.timeInfo;
    mpu6050InfoPtr->runningVelocity = calculateIntegral(mpu6050InfoPtr->runningVelocity, newAcceleration, elapsedTime);
    mpu6050InfoPtr->runningAltitude += secondKinEq(mpu6050InfoPtr->runningVelocity, newAcceleration, elapsedTime);
    info.timeInfo = newTime;
    out.values[0] = mpu6050InfoPtr->runningAltitude;
    out.values[1] = mpu6050InfoPtr->runningVelocity;
    out.values[2] = newAcceleration;
    return true;

}

bool Calculator::gtu7Calculations(CalcInfo& info, float newAltitude, double newTime, FloatArray3& out) {
    GTU7CalcInfo* gtu7InfoPtr = static_cast<GTU7CalcInfo*>(info.sensorSpecificData);
    double elapsedTime = newTime - info.timeInfo;
    newAltitude -= gtu7InfoPtr->offsetInfo;
    out.values[1] = calculateDerivative(gtu7InfoPtr->pastAltitude, newAltitude, elapsedTime);
    out.values[2] = calculateDerivative(gtu7InfoPtr->pastVelocity, out.values[1], elapsedTime);
    out.values[0] = newAltitude;
    gtu7InfoPtr->pastAltitude = newAltitude;
    gtu7InfoPtr->pastVelocity = out.values[1];
    info.timeInfo = newTime;
    return true;
}

bool Calculator::mpx5700gpCalculations(CalcInfo& info, float newVelocity, double newTime, FloatArray3& out) {
    MPX5700GPCalcInfo* mpx5700gpInfoPtr = static_cast<MPX5700GPCalcInfo*>(info.sensorSpecificData);
    double elapsedTime = newTime - info.timeInfo;
    newVelocity -= mpx5700gpInfoPtr->offsetInfo;
    float oldVel = 0.0;//mpx5700gpInfoPtr->recentVelocity;
    //out.values[1] = newVelocity;
    mpx5700gpInfoPtr->recentVelocityValues->getValue(oldVel);
    mpx5700gpInfoPtr->recentVelocityValues->update(newVelocity, out.values[1]);
    out.values[2] = calculateDerivative(oldVel, out.values[1], elapsedTime);
    mpx5700gpInfoPtr->runningAltitude += secondKinEq(oldVel, out.values[2], elapsedTime);
    out.values[0] = mpx5700gpInfoPtr->runningAltitude;
    info.timeInfo = newTime;
    return true;

}

bool Calculator::setStartTime(double newTimeSeconds) {
    for(CalcInfo ci : sensorInfo) {
        ci.timeInfo = newTimeSeconds;
    }
    return true;
}

bool Calculator::getAppropriateInfo(int index, FloatArray3FunctionPtr calcFunction) {
    CalcInfo& info = sensorInfo[index];
    if(calcFunction == &Calculator::bmp280Calculations) {
        //BMP280CalcInfo* bmp280Info = new BMP280CalcInfo{new IIRFilter(0.2), 0, 0}; //play around to try to get smoother changes
        BMP280CalcInfo* bmp280Info = new BMP280CalcInfo{new WindowAverage(20, 0), 0, 0};
        info.sensorSpecificData = bmp280Info;
        info.type = CalcInfo::BMP280CalcInfo;
    } else if(calcFunction == &Calculator::mpu6050Calculations) {
        MPU6050CalcInfo* mpu6050Info = new MPU6050CalcInfo{0, 0};
        info.sensorSpecificData = mpu6050Info;
        info.type = CalcInfo::MPU6050CalcInfo;
    } else if(calcFunction == &Calculator::gtu7Calculations) {
        GTU7CalcInfo* gtu7Info = new GTU7CalcInfo{0, 0, 0};
        info.sensorSpecificData = gtu7Info;
        info.type = CalcInfo::GTU7CalcInfo;
    } else if(calcFunction == &Calculator::mpx5700gpCalculations) {
        MPX5700GPCalcInfo* mpx5700gpInfo = new MPX5700GPCalcInfo{new IIRFilter(0.1), 0}; //also has WindowAverage on initial measurement
        //MPX5700GPCalcInfo* mpx5700gpInfo = new MPX5700GPCalcInfo{0, 0, 0};
        info.sensorSpecificData = mpx5700gpInfo;
        info.type = CalcInfo::MPX5700GPCalcInfo;
    }
    return true;
}

int Calculator::getNextIdentifier() {
    for(uint i = 0; i < CALCULATORMAXSIZE; i++) {
        if(!identifiers[i]) {
            return i;
        }
    }
    return -1;
}

float Calculator::calculateIntegral(float oldMeasurement, float changeRate, double elapsedSeconds) {
    return oldMeasurement + changeRate*elapsedSeconds;
}

float Calculator::calculateDerivative(float measurementBefore, float measurementAfter, double elapsedSeconds) {
    return (measurementAfter - measurementBefore) / elapsedSeconds;
}

/*
returns delta displacement
*/
float Calculator::secondKinEq(float initialVelocity, float acceleration, double elapsedSeconds) {
    return initialVelocity*elapsedSeconds + acceleration*elapsedSeconds*elapsedSeconds;
}