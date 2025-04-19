#ifndef MPU6050_H
#define MPU6050_H
#include "mpu6050Settings.h"
#include "common.h"

#ifdef MPU6050NOAX
#define MPU6050_AX 1
#else
#define MPU6050_AX 0
#endif

#ifdef MPU6050NOAY
#define MPU6050_AY 1
#else
#define MPU6050_AY 0
#endif

#ifdef MPU6050NOAZ
#define MPU6050_AZ 1
#else
#define MPU6050_AZ 0
#endif

#ifdef MPU6050NOGX
#define MPU6050_GX 1
#else
#define MPU6050_GX 0
#endif

#ifdef MPU6050NOGY
#define MPU6050_GY 1
#else
#define MPU6050_GY 0
#endif

#ifdef MPU6050NOGZ
#define MPU6050_GZ 1
#else
#define MPU6050_GZ 0
#endif

#define MPU6050_SENSORS_DISABLED ( \
    MPU6050_AX << 5 | \
    MPU6050_AY << 4 | \
    MPU6050_AZ << 3 | \
    MPU6050_GX << 2 | \
    MPU6050_GY << 1 | \
    MPU6050_GZ << 0 | 0 \
)

class MPU6050 {
    private:
    float inline applyDrift(float val, float drift) {
        return val - drift;
    }
    public:
    MPU6050(uint16_t address, i2c_inst_t* i2c);
    bool setDefaults();
    bool setSampleRate(byte recipValue);
    bool setConfig(byte fsync, byte dlpf);
    bool setAccelRange(byte range);
    bool setGyroRange(byte range);
    bool setMotDetect(byte msens);
    bool getAccelX(float& out) {
        #ifdef MPU6050NOAX
        return false;
        #else
        readI2C(i2c, address, MPU6050_REGISTER_ACCEL_X, 2, &singleMeasure[0]);
        if(*(uint16_t*)&singleMeasure[0] != oldRawData->ax) {
            oldRawData->ax = *(uint16_t*)&singleMeasure[0];
            float ax = applyDrift(BEbytesToShort(&singleMeasure[0]) * accelScale[aRange], drift[0]);
            #ifdef MPU6050INVAX
            ax *= -1;
            #endif
            out = ax;
            oldCompData->ax = out;
            return true;
        }
        out = oldCompData->ax;
        return false;
        #endif
    }
    bool getAccelY(float& out) {
        #ifdef MPU6050NOAY
        return false;
        #else
        readI2C(i2c, address, MPU6050_REGISTER_ACCEL_Y, 2, &singleMeasure[0]);
        if(*(uint16_t*)&singleMeasure[0] != oldRawData->ay) {
            oldRawData->ay = *(uint16_t*)&singleMeasure[0];
            float ay = applyDrift(BEbytesToShort(&singleMeasure[0]) * accelScale[aRange], drift[1]);
            #ifdef MPU6050INVAY
            ay *= -1;
            #endif
            out = ay;
            oldCompData->ay = out;
            return true;
        }
        out = oldCompData->ay;
        return false;
        #endif
    }    
    bool getAccelZ(float& out) {
        #ifdef MPU6050NOAZ
        return false;
        #else
        readI2C(i2c, address, MPU6050_REGISTER_ACCEL_Z, 2, &singleMeasure[0]);
        if(*(uint16_t*)&singleMeasure[0] != oldRawData->az) {
            oldRawData->az = *(uint16_t*)&singleMeasure[0];
            float az = applyDrift(BEbytesToShort(&singleMeasure[0]) * accelScale[aRange], drift[2]);
            #ifdef MPU6050INVAZ
            az *= -1;
            #endif
            out = az;
            oldCompData->az = out;
            return true;
        }
        out = oldCompData->az;
        return false;
        #endif
    }
    bool getGyroX(float& out) {
        #ifdef MPU6050NOGX
        return false;
        #else
        readI2C(i2c, address, MPU6050_REGISTER_GYRO_X, 2, &singleMeasure[0]);
        if(*(uint16_t*)&singleMeasure[0] != oldRawData->gx) {
            oldRawData->gx = *(uint16_t*)&singleMeasure[0];
            float gx = applyDrift(BEbytesToShort(&singleMeasure[0]) * gyroScale[gRange], drift[3]);
            #ifdef MPU6050INVGX
            gx *= -1;
            #endif
            out = gx;
            oldCompData->gx = out;
            return true;
        }
        out = oldCompData->gx;
        return false;
        #endif
    }
    bool getGyroY(float& out) {
        #ifdef MPU6050NOGY
        return false;
        #else
        readI2C(i2c, address, MPU6050_REGISTER_GYRO_Y, 2, &singleMeasure[0]);
        if(*(uint16_t*)&singleMeasure[0] != oldRawData->gy) {
            oldRawData->gy = *(uint16_t*)&singleMeasure[0];
            float gy = applyDrift(BEbytesToShort(&singleMeasure[0]) * gyroScale[gRange], drift[4]);
            #ifdef MPU6050INVGY
            gy *= -1;
            #endif
            out = gy;
            oldCompData->gy = out;
            return true;
        }
        out = oldCompData->gy;
        return false;
        #endif
    }
    bool getGyroZ(float& out) {
        #ifdef MPU6050NOGZ
        return false;
        #else
        readI2C(i2c, address, MPU6050_REGISTER_GYRO_Z, 2, &singleMeasure[0]);
        if(*(uint16_t*)&singleMeasure[0] != oldRawData->gz) {
            oldRawData->gz = *(uint16_t*)&singleMeasure[0];
            float gz = applyDrift(BEbytesToShort(&singleMeasure[0]) * gyroScale[gRange], drift[5]);
            #ifdef MPU6050INVGZ
            gz *= -1;
            #endif
            out = gz;
            oldCompData->gz = out;
            return true;
        }
        out = oldCompData->gz;
        return false;
        #endif
    }    
    bool getTemp(float& out) {
        #ifdef MPU6050NOTEMP
        return false;
        #else
        readI2C(i2c, address, MPU6050_REGISTER_TEMP, 2, &singleMeasure[0]);
        if(*(uint16_t*)&singleMeasure[0] != oldRawData->t) {
            oldRawData->t = *(uint16_t*)&singleMeasure[0];
            out = BEbytesToShort(&singleMeasure[0])/340.0 + 36.53;
            oldCompData->t = out;
            return true;
        }
        out = oldCompData->t;
        return false;
        #endif
    }    
    bool getAccel(FloatArray3 *out) {
        readI2C(i2c, address, MPU6050_REGISTER_ACCEL_X, 6, &fullMeasure[0]);
        if(*(uint16_t*)&fullMeasure[0] != oldRawData->ax || *(uint16_t*)&fullMeasure[2] != oldRawData->ay || *(uint16_t*)&fullMeasure[4] != oldRawData->az) {
            #ifdef MPU6050NOAX
            out->values[0] = 0.0;
            #else
            float ax = applyDrift(BEbytesToShort(&fullMeasure[0]) * accelScale[aRange], drift[5]);
            #ifdef MPU6050INVAX
            ax *= -1;
            #endif
            out->values[0] = ax;
            oldCompData->ax = out->values[0];
            #endif
            #ifdef MPU6050NOAY
            out->values[1] = 0.0;
            #else
            float ay = applyDrift(BEbytesToShort(&fullMeasure[2]) * accelScale[aRange], drift[1]);
            #ifdef MPU6050INVAY
            ay *= -1;
            #endif
            out->values[1] = ay;
            oldCompData->ay = out->values[1];
            #endif
            #ifdef MPU6050NOAZ
            out->values[2] = 0.0;
            #else
            float az = applyDrift(BEbytesToShort(&fullMeasure[4]) * accelScale[aRange], drift[2]);
            #ifdef MPU6050INVAZ
            az *= -1;
            #endif
            out->values[2] = az;
            oldCompData->az = out->values[2];
            #endif
            return true;
        }
        out->values[0] = oldCompData->ax;
        out->values[1] = oldCompData->ay;
        out->values[2] = oldCompData->az;
        return false;
    }
    bool getGyro(FloatArray3 *out) {
        readI2C(i2c, address, MPU6050_REGISTER_GYRO_X, 6, &fullMeasure[0]);
        if(*(uint16_t*)&fullMeasure[0] != oldRawData->gx || *(uint16_t*)&fullMeasure[2] != oldRawData->gy || *(uint16_t*)&fullMeasure[4] != oldRawData->gz) {
            #ifdef MPU6050NOGX
            out->values[0] = 0.0;
            #else
            float gx = applyDrift(BEbytesToShort(&fullMeasure[0]) * gyroScale[gRange], drift[3]);
            #ifdef MPU6050INVGX
            gx *= -1;
            #endif
            out->values[0] = gx;
            oldCompData->gx = out->values[0];
            #endif
            #ifdef MPU6050NOGY
            out->values[1] = 0.0;
            #else
            float gy = applyDrift(BEbytesToShort(&fullMeasure[2]) * gyroScale[gRange], drift[4]);
            #ifdef MPU6050INVGY
            gy *= -1;
            #endif
            out->values[1] = gy; 
            oldCompData->gy = out->values[1];
            #endif
            #ifdef MPU6050NOGZ
            out->values[2] = 0.0;
            #else
            float gz = applyDrift(BEbytesToShort(&fullMeasure[4]) * gyroScale[gRange], drift[5]);
            #ifdef MPU6050INVGZ
            gz *= -1;
            #endif
            out->values[2] = gz;
            oldCompData->gz = out->values[2];
            #endif
            return true;
        }
        out->values[0] = oldCompData->gx;
        out->values[1] = oldCompData->gy;
        out->values[2] = oldCompData->gz;
        return false;
    }
    bool calibrate();
    bool reset();
    bool wake();
    bool setPowerDefaults1(bool temp, byte clock);
    //bitwise, 1 is disabled. ax, ay, az, gx, gy, gz
    bool setPowerDefaults2(byte sensorsDisabled);
    bool disableInterrupts();
    bool resetSensors(bool gyro, bool accel);
    bool disableFIFO();
    bool checkConnection();

    private:
    struct OldRawData;
    struct OldCompensatedData;
    i2c_inst_t* i2c;
    int16_t address;
    float accelScale[4] = {1.0/(1 << 14), 1.0/(1 << 13), 1.0/(1 << 12), 1.0/(1 << 11)};
    float gyroScale[4] = {1.0/131.0, 1.0/65.5, 1.0/32.8, 1.0/16.4};
    byte aRange = 3;
    byte gRange = 3;
    //ax, ay, az, gx, gy, gz
    float drift[6] = {0, 0, 0, 0, 0, 0};
    float driftCal[6] = {0, 0, 0, 0, 0, 0};
    byte singleMeasure[2] = {0, 0};
    byte fullMeasure[6] = {0, 0, 0, 0, 0, 0};
    OldRawData* oldRawData = new OldRawData();
    OldCompensatedData* oldCompData = new OldCompensatedData();
    struct OldRawData {
        uint16_t ax;
        uint16_t ay;
        uint16_t az;
        uint16_t gx;
        uint16_t gy;
        uint16_t gz;
        uint16_t t;
    };
    struct OldCompensatedData {
        float ax;
        float ay;
        float az;
        float gx;
        float gy;
        float gz;
        float t;
    };
};

#endif