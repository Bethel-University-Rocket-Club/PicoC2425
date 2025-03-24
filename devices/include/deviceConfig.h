#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "devices.h"

class Devices {
    public:
    BMP280 GetPressureSensor();
    MPU6050 GetAccelerometer();
    GTU7 GetGPS();
    MPX5700GP GetPitotTube();
    SDCARD GetSDCard();
    
    private:
    //BMP280 I2C pin config
    int BMP280BUS = 1;
    int BMP280SDA = 18; //gp
    int BMP280SCL = 19; //gp
    int BMP280FREQUENCY = 400000; //hz

    //MPU6050 I2C pin config
    int MPU6050BUS = 0;
    int MPU6050SDA = 12; //gp
    int MPU6050SCL = 13; //gp
    int MPU6050FREQUENCY = 400000; //hz

    //SDCard CS/SPI pin config
    int SDCARDCS = 22; //gp
    int SDCARDBUS = 0;
    int SDCARDSCK = 2; //gp
    int SDCARDMOSI = 3; //gp
    int SDCARDMISO = 4; //gp

    //GTU7 UART pin config
    int GPSBUS = 0;
    int GPSTX = 0; //gp
    int GPSRX = 1; //gp
    int GPSBAUDRATE = 9600;

    //MPX5700GP analog pin config
    int MPX5700GPANALOG = 26; //gp
};
#endif