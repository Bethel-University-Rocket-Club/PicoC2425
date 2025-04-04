#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "devices.h"

    //BMP280 I2C pin config
    constexpr int BMP280BUS = 1;
    constexpr int BMP280SDA = 18; //gp
    constexpr int BMP280SCL = 19; //gp
    constexpr int BMP280FREQUENCY = 400000; //hz

    //MPU6050 I2C pin config
    constexpr int MPU6050BUS = 0;
    constexpr int MPU6050SDA = 12; //gp
    constexpr int MPU6050SCL = 13; //gp
    constexpr int MPU6050FREQUENCY = 400000; //hz

    //SDCard CS/SPI pin config
    constexpr int SDCARDCS = 22; //gp
    constexpr int SDCARDBUS = 0;
    constexpr int SDCARDSCK = 2; //gp
    constexpr int SDCARDMOSI = 3; //gp
    constexpr int SDCARDMISO = 4; //gp

    //GTU7 UART pin config
    constexpr int GPSBUS = 0;
    constexpr int GPSTX = 0; //gp
    constexpr int GPSRX = 1; //gp
    constexpr int GPSBAUDRATE = 9600;

    //MPX5700GP analog pin config
    constexpr int MPX5700GPANALOG = 26; //gp

class Devices {
    public:
    BMP280 GetPressureSensor();
    MPU6050 GetAccelerometer();
    GTU7 GetGPS();
    MPX5700GP GetPitotTube();
};
#endif