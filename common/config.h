#ifndef CONFIG_H
#define CONFIG_H
#define LED_PIN PICO_DEFAULT_LED_PIN // Pico's onboard LED pin
#define BUZZ_PIN 28
#define PRINT_PIN 14
#define SDWRITE_PIN 15

#define THRESHOLD (3*9.8) //2g's
#define SEAPRESSURE 1013.25
#define AIRDENSITY 1.3

#define SDBUFSIZE 8192 //128 for data, *4 to get sdcard sector size, *16 for bulk writing - faster than any other power of 2 i've tried
#define USTIMESIZE 12 //how many positions time in microseconds is given when written
#define AGGSAMPLESIZE 10 //how many positions aggregate sample count is given when written
#define INDSAMPLESIZE 10 //how many positions sensor specific sample count is given when written
#define FLOATNUMSIZE 8 //how many positions a float number is given when written

#define SDCARDCONFIG
#define SDCARDCS 22 //gp
#define SDCARDBUS 0 //spi0
#define SDCARDSCK 2 //gp
#define SDCARDMOSI 3 //gp
#define SDCARDMISO 4 //gp

#define GPSBUFCOUNT 2
//Writer - DMA IRQ 0
//GPS - DMA IRQ 1
//BMP280 I2C pin config
#define BMP280CONFIG
#define BMP280BUS 1 //i2c1
#define BMP280SDA 18 //gp
#define BMP280SCL 19 //gp
#define BMP280FREQUENCY 400000 //hz
//MPU6050 I2C pin config
#define MPU6050CONFIG
#define MPU6050BUS 0 //i2c0
#define MPU6050SDA 12 //gp
#define MPU6050SCL 13 //gp
#define MPU6050FREQUENCY 400000 //hz
//SDCard CS/SPI pin config
#define SDCARDCONFIG
#define SDCARDCS 22 //gp
#define SDCARDBUS 0 //spi0
#define SDCARDSCK 2 //gp
#define SDCARDMOSI 3 //gp
#define SDCARDMISO 4 //gp
//GTU7 UART pin config
#define GPSCONFIG
#define GPSBUS 0 //uart0
#define GPSTX 0 //gp
#define GPSRX 1 //gp
#define GPSBAUDRATE 9600
//MPX5700GP analog pin config
#define MPX5700GPCONFIG
#define MPX5700GPANALOG 26 //gp

#define ADXL377CONFIG
#define ADXL377ANALOG 27 //gp

#define CALCULATORMAXSIZE 4 //sensor count
#endif