#ifndef MPU6050_H
#define MPU6050_H
#include "mpu6050Settings.h"
#include "common.h"
class MPU6050 {
    public:
    MPU6050(uint16_t address, i2c_inst_t* i2c);
    bool setDefaults();
    bool setSampleRate(uint recipValue);
    bool setConfig(byte fsync, byte dlpf);
    bool setAccelRange(byte range);
    bool setGyroRange(byte range);
    bool setMotDetect(byte msens);
    float getAccelX();
    float getAccelY();
    float getAccelZ();
    float getGyroX();
    float getGyroY();
    float getGyroZ();
    float getTemp();
    FloatArray3 getAccel();
    FloatArray3 getGyro();
    bool calibrate();
    bool setInvMeasures(byte invMeasures);
    bool reset();
    bool wake();
    bool setPowerDefaults1(bool temp, byte clock);
    bool setPowerDefaults2(bool aX, bool aY, bool aZ, bool gX, bool gY, bool gZ);
    bool disableInterrupts();
    bool resetSensors(bool gyro, bool accel);
    bool disableFIFO();
    private:
    float applyDrift(float val, float drift, bool inv);

    i2c_inst_t* i2c;
    int16_t address;
    float accelScale[4] = {1.0/(1 << 14), 1.0/(1 << 13), 1.0/(1 << 12), 1.0/(1 << 11)};
    float gyroScale[4] = {1.0/131, 1.0/65.5, 1.0/32.8, 1.0/16.4};
    byte aRange = 3;
    byte gRange = 3;
    float drift[6] = {0, 0, 0, 0, 0, 0};
    byte inverse = 0b0u;
    byte singleMeasure[2] = {0, 0};
    byte fullMeasure[6] = {0, 0, 0, 0, 0, 0};
    byte calVals[6] = {0, 0, 0, 0, 0, 0};
};
#endif