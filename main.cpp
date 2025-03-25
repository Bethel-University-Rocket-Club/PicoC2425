#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "devices.h"
#include "calculator.h"
#include "dataSampler.h"
#include "writer.h"
#include "dmaManager.h"
#include "config.h"

void blink(int amt, int msDuration) {
    //blink the on board led amt times, with msDuration
    for(int i = 0; i < amt; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(msDuration);
        gpio_put(LED_PIN, 0);
        sleep_ms(msDuration);
    }
}

bool setup() {
    // Configure the onboard LED GPIO as an output.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    return false;
}

bool validateStart() {
    //validate bmp280
    //validate mpu6050
    //validate gtu7
    //validate pitot
    //validate sdcard
    return false;
}

bool startCondition() {
    //wait for mpu6050 to accelerate past the threshold, then return true
    return false;
}

bool end() {
    //closedown, prepare sdcard for removal.
    //potentially other after sampling activities, reorganizaing data, etc
    return false;
}

void mainLoop() {
    //main logic for what to do when to begin sampling
}

int main() {
    stdio_init_all();
    return 0;
}