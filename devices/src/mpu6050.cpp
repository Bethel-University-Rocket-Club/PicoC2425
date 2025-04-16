#include "mpu6050.h"

MPU6050::MPU6050(uint16_t address, i2c_inst_t *i2c) {
    this->address = address;
    this->i2c = i2c;
    setDefaults();
    sleep_ms(200);
    calibrate();
}

bool MPU6050::setDefaults() {
    wake();
    setSampleRate(0);
    setPowerDefaults1(true, 0);
    setPowerDefaults2(MPU6050_SENSORS_DISABLED);
    setAccelRange(aRange);
    setGyroRange(gRange);
    disableFIFO();
    disableInterrupts();
    setConfig(0, 7);
    setMotDetect(0);
    return true;
}

bool MPU6050::setSampleRate(byte recipValue) {
    byte* write = &recipValue;
    writeI2C(i2c, address, MPU6050_REGISTER_SMPLE_RATE_DIV, 1, &write[0]);
    return true;
}

bool MPU6050::setConfig(byte fsync, byte dlpf) {
    if(fsync < 0 || fsync > 7 || dlpf < 0 || dlpf > 7) {
        return false;
    }
    byte* write = &fsync;
    write[0] <<= 4;
    write[0] |= dlpf;
    writeI2C(i2c, address, MPU6050_REGISTER_CONFIG, 1, &write[0]);
    return true;
}

bool MPU6050::setAccelRange(byte range) {
    if(range < 0 || range > 4) {
        return false;
    }
    aRange = range;
    byte* write = &range;
    write[0] <<= 3;
    writeI2C(i2c, address, MPU6050_REGISTER_ACCEL_CONFIG, 1, &write[0]);
    return true;
}

bool MPU6050::setGyroRange(byte range) {
    if(range < 0 || range > 4) {
        return false;
    }
    gRange = range;
    byte* write = &range;
    write[0] <<= 3;
    writeI2C(i2c, address, MPU6050_REGISTER_GYRO_CONFIG, 1, &write[0]);
    return true;
}

bool MPU6050::setMotDetect(byte msens) {
    byte toWrite = msens;
    writeI2C(i2c, address, MPU6050_REGISTER_GYRO_CONFIG, 1, &toWrite);
    return true;
}

bool MPU6050::calibrate() {
    for(byte i = 0; i < 6; i++) {
        drift[i] = 0;
        driftCal[i] = 0;
    }
    FloatArray3 fVals = {0, 0, 0};
    for(uint16_t i = 0; i < 3000; i++) {
        getAccel(&fVals);
        driftCal[0] = fVals.values[0];
        driftCal[1] = fVals.values[1];
        driftCal[2] = fVals.values[2];
        fVals = {0, 0, 0};
        getGyro(&fVals);
        driftCal[3] = fVals.values[0];
        driftCal[4] = fVals.values[1];
        driftCal[5] = fVals.values[2];

    }
    for(byte i = 0; i < 6; i++) {
        drift[i] = driftCal[i];
    }
    return true;
}

bool MPU6050::reset() {
    byte toWrite = 1 << 7;
    writeI2C(i2c, address, MPU6050_REGISTER_POWER_MGMNT1, 1, &toWrite);
    return true;
}

bool MPU6050::wake() {
    byte toWrite = 0;
    writeI2C(i2c, address, MPU6050_REGISTER_POWER_MGMNT1, 1, &toWrite);
    return true;
}

bool MPU6050::setPowerDefaults1(bool temp, byte clock) {
    if(clock < 0 || clock > 7) {
        return false;
    }
    byte toWrite = (temp << 3) | clock;
    writeI2C(i2c, address, MPU6050_REGISTER_POWER_MGMNT1, 1, &toWrite);
    return true;
}

bool MPU6050::setPowerDefaults2(byte sensorsDisabled) {
    if(sensorsDisabled < 0 || sensorsDisabled > 63) {
        return false;
    }
    byte toWrite = sensorsDisabled;
    writeI2C(i2c, address, MPU6050_REGISTER_POWER_MGMNT2, 1, &toWrite);
    return true;
}

bool MPU6050::disableInterrupts() {
    byte toWrite = 0;
    writeI2C(i2c, address, MPU6050_REGISTER_INT_CONFIG, 1, &toWrite);
    writeI2C(i2c, address, MPU6050_REGISTER_INT_ENABLE, 1, &toWrite);
    return true;
}

bool MPU6050::resetSensors(bool gyro, bool accel) {
    byte toWrite = (gyro << 2) | (accel << 1) | 1;
    writeI2C(i2c, address, MPU6050_REGISTER_PATH_RESET, 1, &toWrite);
    return true;
}

bool MPU6050::disableFIFO() {
    byte toWrite = 0;
    writeI2C(i2c, address, MPU6050_REGISTER_USER_CONTROL, 1, &toWrite);
    return true;
}