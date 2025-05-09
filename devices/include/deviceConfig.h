#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "devices.h"

#ifndef BMP280CONFIG
//BMP280 I2C pin config
#define BMP280BUS 1
#define BMP280SDA 18 //gp
#define BMP280SCL 19 //gp
#define BMP280FREQUENCY 400000 //hz
#endif

#ifndef ADXL377CONFIG
#define ADXL377ANALOG 27
#endif

#ifndef GPSCONFIG
//GTU7 UART pin config
#define GPSBUS 0
#define GPSTX 0 //gp
#define GPSRX 1 //gp
#define GPSBAUDRATE 9600
#endif

#ifndef MPX5700GPCONFIG
//MPX5700GP analog pin config
#define MPX5700GPANALOG 26 //GPIO Analog number
#endif

class Devices {
    public:
    BMP280* GetPressureSensor();
    ADXL377* GetAccelerometer();
    GTU7* GetGPS();
    MPX5700GP* GetPitotTube();
};
#endif