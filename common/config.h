#ifndef CONFIG_H
#define CONFIG_H
#define LED_PIN PICO_DEFAULT_LED_PIN // Pico's onboard LED pin
#undef THRESHOLD
#undef SDBUFSIZE
#undef USTIMESIZE
#undef AGGSAMPLESIZE
#undef INDSAMPLESIZE
#undef FLOATNUMSIZ
#undef BMP280BUS
#undef BMP280SDA
#undef BMP280SCL
#undef BMP280FREQUENCY
#undef MPU6050BUS
#undef MPU6050SDA
#undef MPU6050SCL
#undef MPU6050FREQUENCY
#undef SDCARDCS
#undef SDCARDBUS
#undef SDCARDSCK
#undef SDCARDMOSI
#undef SDCARDMISO
#undef GPSBUS
#undef GPSTX
#undef GPSRX
#undef GPSBAUDRATE
#undef MPX5700GPANALOG

#define THRESHOLD (2*9.8) //2g's
#define SDBUFSIZE 8192 //128 for data, *4 to get sdcard sector size, *16 for bulk writing - faster than any other power of 2 i've tried
#define USTIMESIZE 12 //how many positions time in microseconds is given when written
#define AGGSAMPLESIZE 10 //how many positions aggregate sample count is given when written
#define INDSAMPLESIZE 10 //how many positions sensor specific sample count is given when written
#define FLOATNUMSIZE 8 //how many positions a float number is given when written

//BMP280 I2C pin config
#define BMP280BUS 1
#define BMP280SDA 18 //gp
#define BMP280SCL 19 //gp
#define BMP280FREQUENCY 400000 //hz
//MPU6050 I2C pin config
#define MPU6050BUS 0
#define MPU6050SDA 12 //gp
#define MPU6050SCL 13 //gp
#define MPU6050FREQUENCY 400000 //hz
//SDCard CS/SPI pin config
#define SDCARDCS 22 //gp
#define SDCARDBUS 0
#define SDCARDSCK 2 //gp
#define SDCARDMOSI 3 //gp
#define SDCARDMISO 4 //gp
//GTU7 UART pin config
#define GPSBUS 0
#define GPSTX 0 //gp
#define GPSRX 1 //gp
#define GPSBAUDRATE 9600
//MPX5700GP analog pin config
#define MPX5700GPANALOG 26 //gp
#endif