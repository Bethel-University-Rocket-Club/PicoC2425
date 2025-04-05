#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "common.h"
#include "devices.h"
#include "deviceConfig.h"
#include "calculator.h"
#include "dataSampler.h"
#include "writer.h"
#include "dmaManager.h"
#include "config.h" //ensure it is last, as it redefines all the values
#include "hw_config.h"
#include <exception>

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
    setup();
    blink(5, 1000);
    sd_card_t* sd = sd_get_by_num(0);
    sd->spi->hw_inst = spi0;
    sd->spi->miso_gpio = SDCARDMISO;
    sd->spi->mosi_gpio = SDCARDMOSI;
    sd->spi->sck_gpio = SDCARDSCK;
    sd->ss_gpio = SDCARDCS;

    Writer* w = new Writer(sd);

    DataBuffer db1 = {12741223, 18732, 1723, 2, 3, {(float)123.1273, (float)87.2345, (float)1.93854}};
    DataBuffer db2 = {1827387, 92834, 4523, 1, 3, {(float)182.821273, (float)872.2342835, (float)21.1328}};
    DataBuffer db3 = {1932123, 34287, 1023, 3, 3, {(float)923.7123, (float)2.0, (float)81723.12893}};
    DataBuffer db4 = {1187234, 69753, 8734, 0, 3, {(float)1238.13287, (float)1732.456, (float)0.237412}};

    w->writeHeader();
    uint32_t startTime = time_us_32();
    for(int i = 0; i < 3000; i++) {
        w->writeData(&db1);
        w->writeData(&db2);
        w->writeData(&db3);
        w->writeData(&db4);
        //printf("%d\n", i);
    }
    w->flush();
    w->close();
uint32_t endTime = time_us_32();
    printf("elapsedTimeUS: %d", (endTime - startTime));
    return 0;
}