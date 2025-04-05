#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "devices.h"

    //BMP280 I2C pin config
    #define BMP280BUS = 1;
    #define BMP280SDA = 18; //gp
    #define BMP280SCL = 19; //gp
    #define BMP280FREQUENCY = 400000; //hz

    //MPU6050 I2C pin config
    #define MPU6050BUS = 0;
    #define MPU6050SDA = 12; //gp
    #define MPU6050SCL = 13; //gp
    #define MPU6050FREQUENCY = 400000; //hz

    //SDCard CS/SPI pin config
    #define SDCARDCS = 22; //gp
    #define SDCARDBUS = 0;
    #define SDCARDSCK = 2; //gp
    #define SDCARDMOSI = 3; //gp
    #define SDCARDMISO = 4; //gp

    //GTU7 UART pin config
    #define GPSBUS = 0;
    #define GPSTX = 0; //gp
    #define GPSRX = 1; //gp
    #define GPSBAUDRATE = 9600;

    //MPX5700GP analog pin config
    #define MPX5700GPANALOG = 26; //gp

class Devices {
    public:
    BMP280 GetPressureSensor();
    MPU6050 GetAccelerometer();
    GTU7 GetGPS();
    MPX5700GP GetPitotTube();
};
#endif