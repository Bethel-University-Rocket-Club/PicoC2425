#ifndef COMMON_H
#define COMMON_H
#include "pico/stdlib.h"
typedef unsigned char byte;
struct FloatArray3 {
    float values[3];
};
struct FloatArray2 {
    float values[2];
};
struct DataBuffer {
    uint64_t elapsedTime;
    uint32_t aggSampleNum;
    uint32_t sensorSampleNum;
    byte sensorNum;
    byte sensorMax; //max value for sensorNum
    FloatArray3 data;
};
uint16_t inline bytesToUShort(byte* data);
int16_t inline bytesToShort(byte* data);
void inline blink(int amt, int msDuration) {
    //blink the on board led amt times, with msDuration
    for(int i = 0; i < amt; i++) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(msDuration);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(msDuration);
    }
}
#endif