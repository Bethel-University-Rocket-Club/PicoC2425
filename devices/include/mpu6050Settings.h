#ifndef MPU6050SETTINGS_H
#define MPU6050SETTINGS_H

// MPU6050 I2C Address
#define MPU6050_I2CADDR 0x68

// MPU6050 Registers
#define MPU6050_REGISTER_SMPLE_RATE_DIV 0x19 // 8 bit unsigned value, sample rate = 1/value
#define MPU6050_REGISTER_CONFIG 0x1A // 00 ext_sync_set (0-7) dlpf (0-7)
#define MPU6050_REGISTER_GYRO_CONFIG 0x1B // self test x, y, z range (0-4)
#define MPU6050_REGISTER_ACCEL_CONFIG 0x1C // self test x, y, z range (0-4)
#define MPU6050_REGISTER_INT_CONFIG 0x37 // check docs
#define MPU6050_REGISTER_INT_ENABLE 0x38 // check docs
#define MPU6050_REGISTER_ACCEL_X 0x3B // 2 bytes, 2's comp
#define MPU6050_REGISTER_ACCEL_Y 0x3D // 2 bytes, 2's comp
#define MPU6050_REGISTER_ACCEL_Z 0x3F // 2 bytes, 2's comp
#define MPU6050_REGISTER_TEMP 0x41 // 2 bytes, 2's comp
#define MPU6050_REGISTER_GYRO_X 0x43 // 2 bytes, 2'comp
#define MPU6050_REGISTER_GYRO_Y 0x45 // 2 bytes, 2'comp
#define MPU6050_REGISTER_GYRO_Z 0x47 // 2 bytes, 2'comp
#define MPU6050_REGISTER_START_ATEST 0x1C // x, y, z, range, range, reserved...
#define MPU6050_REGISTER_START_GTEST 0x1B // x, y, z, range, range, reserved...
#define MPU6050_REGISTER_XTEST_RESULTS 0x0D // 3 a, 5 g
#define MPU6050_REGISTER_YTEST_RESULTS 0x0E // 3 a, 5 g
#define MPU6050_REGISTER_ZTEST_RESULTS 0x0F // 3 a, 5 g
#define MPU6050_REGISTER_ALOW_RESULTS 0x10 // 3 reserved, 2 x a low, 2 y a low, 2 z a low
#define MPU6050_REGISTER_PATH_RESET 0x68 // 00000 gyro accel temp
#define MPU6050_REGISTER_USER_CONTROL 0x6A // check docs
#define MPU6050_REGISTER_POWER_MGMNT1 0x6B // check docs
#define MPU6050_REGISTER_POWER_MGMNT2 0x6C // check docs

#endif // MPU6050SETTINGS_H