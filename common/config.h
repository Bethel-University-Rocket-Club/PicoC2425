#ifndef CONFIG_H
#define CONFIG_H
#define LED_PIN PICO_DEFAULT_LED_PIN // Pico's onboard LED pin
#undef THRESHOLD
#undef SDBUFSIZE
#undef USTIMESIZE
#undef AGGSAMPLESIZE
#undef INDSAMPLESIZE
#undef FLOATNUMSIZ


#define THRESHOLD (2*9.8) //2g's
#define SDBUFSIZE = 8192 //128 for data, *4 to get sdcard sector size, *16 for bulk writing
#define USTIMESIZE 12 //how many positions time in microseconds is given when written
#define AGGSAMPLESIZE 10 //how many positions aggregate sample count is given when written
#define INDSAMPLESIZE 10 //how many positions sensor specific sample count is given when written
#define FLOATNUMSIZE 8 //how many positions a float number is given when written
#endif