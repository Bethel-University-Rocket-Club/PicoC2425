#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/platform.h"
#include "hardware/i2c.h"
#include <cmath>
#include "config.h"

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
    uint8_t sensorNum;
    uint8_t sensorMax; //max value for sensorNum
    float sensorSample;
    FloatArray3 data;
    uint8_t sigDecimalDigits;
};
uint16_t inline LEbytesToUShort(byte* data) {
    return (data[1] << 8) | data[0];
}
int16_t inline LEbytesToShort(byte* data) {
    return (int16_t)((data[1] << 8) | data[0]);
}
int16_t inline BEbytesToShort(byte* data) {
    return (int16_t)((data[0] << 8) | data[1]);
}
void inline blink(int amt, int msDuration) {
    //blink the on board led amt times, with msDuration
    for(int i = 0; i < amt; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(msDuration);
        gpio_put(LED_PIN, 0);
        sleep_ms(msDuration);
    }
}

void inline buzz(int msDuration) {
    gpio_put(BUZZ_PIN, 1);
    sleep_ms(msDuration);
    gpio_put(BUZZ_PIN, 0);
}

bool inline writeI2C(i2c_inst_t* i2c, uint8_t addr, uint8_t reg, uint8_t byteCount, uint8_t* data) {
    uint8_t buf[byteCount + 1];  // Create a buffer of the required size
    buf[0] = reg;  // The first byte is the register address
    for (uint8_t i = 0; i < byteCount; i++) {
        buf[i + 1] = data[i];  // Copy data into the buffer starting from the second byte
    }
    return i2c_write_blocking(i2c, addr, buf, byteCount+1, false) == byteCount+1;
}

bool inline readI2C(i2c_inst_t* i2c, uint8_t addr, uint8_t reg, uint8_t byteCount, uint8_t* data) {
    i2c_write_blocking(i2c, addr, &reg, 1, true);
    return i2c_read_blocking(i2c, addr, data, byteCount, false) == byteCount;
}
#endif