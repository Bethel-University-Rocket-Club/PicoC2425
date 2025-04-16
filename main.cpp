#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "config.h"
#include "common.h"
#include "devices.h"
#include "deviceConfig.h"
#include "calculator.h"
#include "dataSampler.h"
#include "writer.h"
#include "hw_config.h"

Devices* d;
Writer* w;
Calculator* c;
BMP280* bmp;
MPU6050* mpu;
GTU7* gtu;
MPX5700GP* mpx;

bool setup() {
    // Configure the onboard LED GPIO as an output.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    blink(3, 500);
    sd_card_t* sd = sd_get_by_num(0);
    sd->spi->hw_inst = spi0;
    sd->spi->miso_gpio = SDCARDMISO;
    sd->spi->mosi_gpio = SDCARDMOSI;
    sd->spi->sck_gpio = SDCARDSCK;
    sd->ss_gpio = SDCARDCS;
    d = new Devices();
    w = new Writer(sd);
    c = new Calculator();
    bmp = d->GetPressureSensor();
    mpu = d->GetAccelerometer();
    gtu = d->GetGPS();
    mpx = d->GetPitotTube();
    w->writeHeader();
    return true;
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

void writeToUART0(const char* sentence, uint32_t length) {
    for(uint32_t i = 0; i < length; i++) {
        uart_putc(uart0, sentence[i]);
    }
}

int main() {
    stdio_init_all();
    setup();
    DataBuffer datas[4] = {};
    datas[0].sensorNum = 0;
    datas[1].sensorNum = 1;
    datas[2].sensorNum = 2;
    datas[3].sensorNum = 3;
    datas[0].sensorMax = 3;
    datas[1].sensorMax = 3;
    datas[2].sensorMax = 3;
    datas[3].sensorMax = 3;
    datas[0].sigDecimalDigits = 2;
    datas[1].sigDecimalDigits = 3;
    datas[2].sigDecimalDigits = 2;
    datas[3].sigDecimalDigits = 1;

    bmp->setSeaPressure(1013.25);
    mpx->setAirDensity(1.225);
    float initAlt = 0.0;
    float ax = 0.0;
    float vel = 0.0;
    float gtuAlt = 0.0;
    bool success = bmp->getAltitude(initAlt);
    success = gtu->getAltitude(gtuAlt);
    int bmpID = c->addSensor(&Calculator::bmp280Calculations);
    c->configureInitialOffset(bmpID, initAlt);
    int mpuID = c->addSensor(&Calculator::mpu6050Calculations);
    int mpxID = c->addSensor(&Calculator::mpx5700gpCalculations);
    mpx->getDrift(vel);
    c->configureInitialOffset(mpxID, vel);
    int gtuID = c->addSensor(&Calculator::gtu7Calculations);
    c->configureInitialOffset(gtuID, gtuAlt);
    
    uint32_t aggSampleNum = 0;
    uint64_t startTime = time_us_64();
    uint64_t etime = time_us_64() - startTime;
    double dTime = etime/1000000.0;
    c->setStartTime(dTime);
    int countBMP = 0;
    int countMPU = 0;
    int countMPX = 0;
    int countGTU = 0;
    /*
    success = gtu->getAltitude(gtuAlt);
    printf("initAlt:%f, new:%d\n", gtuAlt, (int)success);

    const char* sentence1 = "$GPGGA,134658.00,5106.9792,N,11402.3003,W,2,09,1.0,1048.47,M,-16.27,M,08,AAAA*60\r\n";
    writeToUART0(sentence1, 82);
    sleep_ms(50);
    success = gtu->getAltitude(gtuAlt);
    printf("initAlt:%f, new:%d\n", gtuAlt, (int)success);

    //get alt, check success
    const char* sentence2 = "$GPGGA,134658.00,5106.9792,N,11402.3003,W,2,09,1.0,1048,M,-16.27,M,08,AAAA*60\r\n";
    writeToUART0(sentence2, 79);
    sleep_ms(50);
    success = gtu->getAltitude(gtuAlt);
    printf("initAlt:%f, new:%d\n", gtuAlt, (int)success);

    //get alt, check success
    const char* sentence3 = "$GPGGA,134658.00,5106.9792,N,11402.3003,W,2,09,1.0,1048,M,-16.27,M,08,AAAA*60\r\n";
    writeToUART0(sentence3, 79);
    sleep_ms(50);
    success = gtu->getAltitude(gtuAlt);
    printf("initAlt:%f, new:%d\n", gtuAlt, (int)success);

    //get alt, check success
    const char* sentence4 = "$GPGGA,134658.00,5106.9792,N,11402.3003,W,2,09,1.0,1048.47,M,-16.27,M,08,AAAA*60\r\n";
    writeToUART0(sentence4, 82);
    sleep_ms(50);
    success = gtu->getAltitude(gtuAlt);
    printf("initAlt:%f, new:%d\n", gtuAlt, (int)success);
    //get alt, check success
    */
    for(;dTime < 1.0;) {
        etime = time_us_64() - startTime;
        dTime = etime * 0.000001f;
        if(bmp->getAltitude(initAlt)) {
            countBMP++;
            aggSampleNum++;
            c->newSample(bmpID, initAlt, dTime, datas[0].data);
            datas[0].aggSampleNum = aggSampleNum;
            datas[0].elapsedTime = etime;
            datas[0].sensorSampleNum += 1;
            //printf("bmp: %f %f %f\n", datas[0].data.values[0], datas[0].data.values[1], datas[0].data.values[2]);
            w->writeData(&datas[0]);
        }
        etime = time_us_64() - startTime;
        dTime = etime * 0.000001f;
        if(mpu->getAccelZ(ax)) {
            countMPU++;
            aggSampleNum++;
            c->newSample(mpuID, ax, dTime, datas[1].data);
            datas[1].aggSampleNum = aggSampleNum;
            datas[1].elapsedTime = etime;
            datas[1].sensorSampleNum += 1;
            //printf("mpu: %f %f %f\n", datas[1].data.values[0], datas[1].data.values[1], datas[1].data.values[2]);
            w->writeData(&datas[1]);
        }
        etime = time_us_64() - startTime;
        dTime = etime * 0.000001f;
        if(mpx->getVelocity(vel)) {
            countMPX++;
            aggSampleNum++;
            c->newSample(mpxID, vel, dTime, datas[3].data);
            datas[3].aggSampleNum = aggSampleNum;
            datas[3].elapsedTime = etime;
            datas[3].sensorSampleNum += 1;
            //printf("mpx: %f %f %f\n", datas[3].data.values[0], datas[3].data.values[1], datas[3].data.values[2]);
            w->writeData(&datas[3]);
        }
        etime = time_us_64() - startTime;
        dTime = etime * 0.000001f;
        if(gtu->getAltitude(gtuAlt)) {
            //printf("gps sample\n");
            countGTU++;
            aggSampleNum++;
            c->newSample(gtuID, gtuAlt, dTime, datas[2].data);
            datas[2].aggSampleNum = aggSampleNum;
            datas[2].elapsedTime = etime;
            datas[2].sensorSampleNum += 1;
            //printf("gtu: %f %f %f\n", datas[2].data.values[0], datas[2].data.values[1], datas[2].data.values[2]);
            w->writeData(&datas[2]);
        }
            
    }
    printf("countBMP:%d countMPU:%d countGTU:%d countMPX:%d\n", countBMP, countMPU, countGTU, countMPX);
    w->flush();
    w->close();
    return 0;
}