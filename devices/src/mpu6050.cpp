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
    sleep_ms(1000);
    setSampleRate(0);
    sleep_ms(100);
    resetSensors(false, true);
    sleep_ms(1000);
    setPowerDefaults1(true, 0);
    sleep_ms(100);
    setPowerDefaults2(MPU6050_SENSORS_DISABLED);
    sleep_ms(100);
    setGyroRange(gRange);
    sleep_ms(100);
    disableFIFO();
    sleep_ms(100);
    disableInterrupts();
    sleep_ms(100);
    //setConfig(0, 7);
    sleep_ms(100);
    //setMotDetect(0);
    sleep_ms(100);
    setAccelRange(aRange);
    sleep_ms(100);
    return true;
}

bool MPU6050::setSampleRate(byte recipValue) {
    byte write = recipValue;
    writeI2C(i2c, address, MPU6050_REGISTER_SMPLE_RATE_DIV, 1, &write);
    return true;
}

bool MPU6050::setConfig(byte fsync, byte dlpf) {
    if(fsync < 0 || fsync > 7 || dlpf < 0 || dlpf > 7) {
        return false;
    }
    byte write = (fsync << 4) | dlpf;
    writeI2C(i2c, address, MPU6050_REGISTER_CONFIG, 1, &write);
    return true;
}

bool MPU6050::setAccelRange(byte range) {
    if(range < 0 || range >= 4) {
        return false;
    }
    aRange = range;
    byte write = 3 << 3;
    writeI2C(i2c, address, MPU6050_REGISTER_ACCEL_CONFIG, 1, &write);
    return true;
}

bool MPU6050::setGyroRange(byte range) {
    if(range < 0 || range > 4) {
        return false;
    }
    gRange = range;
    byte write = range << 3;
    writeI2C(i2c, address, MPU6050_REGISTER_GYRO_CONFIG, 1, &write);
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
        driftCal[0] += fVals.values[0] / 3000.0;
        driftCal[1] += fVals.values[1] / 3000.0;
        driftCal[2] += fVals.values[2] / 3000.0;
        fVals = {0, 0, 0};
        getGyro(&fVals);
        driftCal[3] += fVals.values[0] / 3000.0;
        driftCal[4] += fVals.values[1] / 3000.0;
        driftCal[5]+= fVals.values[2] / 3000.0;

    }
    for(byte i = 0; i < 6; i++) {
        drift[i] = driftCal[i];
    }
    #ifdef MPU6050INVAX
    drift[0] *= -1;
    #endif
    #ifdef MPU6050INVAY
    drift[1] *= -1;
    #endif
    #ifdef MPU6050INVAZ
    drift[2] *= -1;
    #endif
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

bool MPU6050::checkConnection() {
    // Attempt to write 0 bytes to the BMP280 (probe address)
    uint8_t dummy = 0;
    int ret = i2c_write_blocking(i2c, address, &dummy, 1, false);
    sleep_ms(100);
    if (ret >= 0) {
        return true;
    } else {
        return false;
    }
}