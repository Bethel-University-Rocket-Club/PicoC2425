#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "config.h"
#include "common.h"
#include "devices.h"
#include "deviceConfig.h"
#include "calculator.h"
#include "writer.h"
#include "circularQueue.h"
#include "hw_config.h"

Devices* d;
Writer* w;
Calculator* c;
BMP280* bmp;
MPU6050* mpu;
GTU7* gtu;
MPX5700GP* mpx;
CircularQueue* queue;

bool END = false;
bool DONEWRITE = false;
uint64_t STARTTIMEMICRO = 0.0;
int BMPID;
int MPUID;
int GTUID;
int MPXID;

bool validateSensors() {
    sd_card_t* sd = sd_get_by_num(0);
    sd->spi->hw_inst = spi0;
    sd->spi->miso_gpio = SDCARDMISO;
    sd->spi->mosi_gpio = SDCARDMOSI;
    sd->spi->sck_gpio = SDCARDSCK;
    sd->ss_gpio = SDCARDCS;
    d = new Devices();
    w = new Writer(sd);
    c = new Calculator();
    queue = new CircularQueue();
    bmp = d->GetPressureSensor();
    mpu = d->GetAccelerometer();
    gtu = d->GetGPS();
    mpx = d->GetPitotTube();
    while(!bmp->checkConnection()) {
        blink(1, 1000);
        blink(10, 250);
    }
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!mpu->checkConnection()) {
        blink(2, 1000);
        blink(10, 250);
    }
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!gtu->checkConnection()) {
        blink(3, 1000);
        blink(10, 250);
    }
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    /*
    while(!mpx->checkConnection()) {
        blink(4, 1000);
        blink(10, 250);
    }
        */
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!w->checkConnection()) {
        blink(5, 1000);
        blink(10, 250);
    }
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    return true;
}

bool setup() {
    // Configure the onboard LED GPIO as an output.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    validateSensors();
    blink(3, 500); //remove eventually
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    w->writeHeader();
    bmp->setSeaPressure(SEAPRESSURE);
    mpx->setAirDensity(AIRDENSITY);
    float measure = 0.0;
    BMPID = c->addSensor(&Calculator::bmp280Calculations);
    MPUID = c->addSensor(&Calculator::mpu6050Calculations);
    GTUID = c->addSensor(&Calculator::gtu7Calculations);
    MPXID = c->addSensor(&Calculator::mpx5700gpCalculations);
    bmp->getAltitude(measure);
    c->configureInitialOffset(BMPID, measure);
    gtu->getAltitude(measure);
    c->configureInitialOffset(GTUID, measure);
    mpx->getDrift(measure);
    c->configureInitialOffset(MPXID, measure);
    return true;
}

bool startCondition() {
    float accelUp = 0.0;
    while(true) {
        mpu->getAccelY(accelUp);
        if(accelUp*9.8 > THRESHOLD) {
        //if(accelUp*9.8 > 2) {
            STARTTIMEMICRO = time_us_64();
            c->setStartTime(STARTTIMEMICRO * 0.000001);
            return true;
        }
    }
    return false;
}

void setOffsets() {
    float measure = 0.0;
    bmp->getAltitude(measure);
    c->configureInitialOffset(BMPID, measure);
    gtu->getAltitude(measure);
    c->configureInitialOffset(GTUID, measure);
    mpx->getDrift(measure);
    c->configureInitialOffset(MPXID, measure);
}

bool endCondition(float altitude) {
    static float apogee = 0.0;
    static uint32_t apogeeTime = STARTTIMEMICRO;
    //1 second test
    /*
    if(time_us_64() - STARTTIMEMICRO > 1000000) {
        printf("end\n");
        return true;
    }
        */
    if(altitude < apogee-5) {
        //1 second of the measurement being below apogee
        if(time_us_64() - apogeeTime > 1000000) {
            return true;
        }
    } else if(altitude >= apogee) {
        apogeeTime = time_us_64() - STARTTIMEMICRO;
        apogee = altitude;
    }
    return false;
}

void closeDown() {
    DataBuffer* curRead = nullptr;
    while(queue->size() > 0) {
        curRead = queue->dequeue();
        c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
        w->writeData(curRead);
        queue->finishDequeue();
    }
    w->flush();
    w->close();
}

void calcWriteLoop() {
    DataBuffer* curRead = nullptr;
    float altitude = 0.0;
    while(!endCondition(altitude)) {
        if(queue->size() > 0) {
            curRead = queue->dequeue();
            c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
            w->writeData(curRead);
            if(curRead->sensorNum == 0) { //based on bmp altitude
                altitude = curRead->data.values[0];
            }
            queue->finishDequeue();
        }
    }
    END = true;
}

void samplingLoop() {
    DataBuffer* curWrite = nullptr;
    uint64_t elapsedTime = time_us_64() - STARTTIMEMICRO;
    uint32_t aggSampleCount = 0;
    uint32_t countBMP = 0;
    uint32_t countMPU = 0;
    uint32_t countGTU = 0;
    uint32_t countMPX = 0;
    float measure = 0.0;
    while(!END) {
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        if(bmp->getAltitude(measure) && !queue->isFull()) {
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                aggSampleCount++;
                countBMP++;
                curWrite->aggSampleNum = aggSampleCount;
                curWrite->sensorSampleNum = countBMP;
                curWrite->elapsedTime = elapsedTime;
                curWrite->sensorSample = measure;
                curWrite->sensorMax = 3;
                curWrite->sensorNum = 0;
                curWrite->sigDecimalDigits = 2;
                curWrite->data.values[0] = measure;
                queue->finishEnqueue();
            }
        }
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        if(mpu->getAccelY(measure) && !queue->isFull()) {
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                measure *= 9.8;
                aggSampleCount++;
                countMPU++;
                curWrite->aggSampleNum = aggSampleCount;
                curWrite->sensorSampleNum = countMPU;
                curWrite->elapsedTime = elapsedTime;
                curWrite->sensorSample = measure;
                curWrite->sensorMax = 3;
                curWrite->sensorNum = 1;
                curWrite->sigDecimalDigits = 3;
                curWrite->data.values[2] = measure;
                queue->finishEnqueue();
            }
        }
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        if(gtu->getAltitude(measure) && !queue->isFull()) {
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                aggSampleCount++;
                countGTU++;
                curWrite->aggSampleNum = aggSampleCount;
                curWrite->sensorSampleNum = countGTU;
                curWrite->elapsedTime = elapsedTime;
                curWrite->sensorSample = measure;
                curWrite->sensorMax = 3;
                curWrite->sensorNum = 2;
                curWrite->sigDecimalDigits = 2;
                curWrite->data.values[0] = measure;
                queue->finishEnqueue();
            }
        }
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        if(mpx->getVelocity(measure) && !queue->isFull()) {
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                aggSampleCount++;
                countMPX++;
                curWrite->aggSampleNum = aggSampleCount;
                curWrite->sensorSampleNum = countMPX;
                curWrite->elapsedTime = elapsedTime;
                curWrite->sensorSample = measure;
                curWrite->sensorMax = 3;
                curWrite->sensorNum = 3;
                curWrite->sigDecimalDigits = 2;
                curWrite->data.values[1] = measure;
                queue->finishEnqueue();
            }
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(100);
    setup();
    blink(20, 100);
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!startCondition()) {
        tight_loop_contents();
    }
    multicore_launch_core1(calcWriteLoop);
    samplingLoop();
    closeDown();
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    return 0;
}