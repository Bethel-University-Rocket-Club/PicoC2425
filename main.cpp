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
bool BMPFail = false;
bool MPUFail = false;
bool GTUFail = false;
bool MPXFail = false;

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
    uint8_t failCount = 0;
    while(!bmp->checkConnection() && failCount < 2) {
        printf("BMP error\n");
        buzz(10000);
        blink(1, 1000);
        blink(10, 250);
        failCount++;
    }
    if(!bmp->checkConnection() && failCount == 2) BMPFail = true;
    failCount = 0;
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!mpu->checkConnection() && failCount < 2) {
        printf("MPU error\n");
        buzz(20000);
        blink(2, 1000);
        blink(10, 250);
        failCount++;
    }
    if(!mpu->checkConnection() && failCount == 2) MPUFail = true;
    failCount = 0;
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!gtu->checkConnection() && failCount < 2) {
        printf("GTU error\n");
        buzz(30000);
        blink(3, 1000);
        blink(10, 250);
        failCount++;
    }
    if(!gtu->checkConnection() && failCount == 2) GTUFail = true;
    failCount = 0;
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    /*
    while(!mpx->checkConnection()) {
        buzz(4000);
        blink(4, 1000);
        blink(10, 250);
    }
        */
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!w->checkConnection()) {
        printf("SDCARD error\n");
        buzz(50000);
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
    gpio_init(BUZZ_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(BUZZ_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    buzz(50);
    validateSensors();
    //blink(3, 500); //remove eventually
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
    float alt = 0.0;
    float alt2 = 0.0;
    while(true) {
        mpu->getAccelY(accelUp);
        bmp->getAltitude(alt);
        gtu->getAltitude(alt2);
        //printf("accel: %f\n", (accelUp*9.8));
        if(!MPUFail) {
            //printf("accel\n");
            if(accelUp*9.8 > THRESHOLD || accelUp*9.8 < THRESHOLD*-1){
                //if(accelUp*9.8 > 2) {
                STARTTIMEMICRO = time_us_64();
                c->configureInitialOffset(GTUID, alt2);
                c->configureInitialOffset(BMPID, alt);
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
        } else if(MPUFail && !BMPFail) {
            //printf("barometric\n");
            if(alt - c->getInitialOffset(BMPID) > 15) {
                STARTTIMEMICRO = time_us_64();
                //c->configureInitialOffset(GTUID, alt2);
                //c->configureInitialOffset(BMPID, alt);
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
        } else if(MPUFail && BMPFail && !GTUFail) {
            //printf("gps\n");
            if(alt2 - c->getInitialOffset(GTUID) > 15) {
                STARTTIMEMICRO = time_us_64();
                //c->configureInitialOffset(GTUID, alt2);
                //c->configureInitialOffset(BMPID, alt);
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
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
    static bool ended = false;
    //1 second test
    /*
    if(time_us_64() - STARTTIMEMICRO > 1000000) {
        printf("end\n");
        return true;
    }*/
    //printf("alt: %f, apg: %f end: %d\n", altitude, apogee, ended);
    if(!ended && altitude < apogee-5) {
        //1 second of the measurement being below apogee
        if(time_us_64() - apogeeTime > 10000000) {
            while(!w->close()) {
                tight_loop_contents();
            }
            while(!w->open("afterApogee.csv")) {
                tight_loop_contents();
            }
            //printf("new file\n");
            ended = true;
            return false;
        }
    } else if(altitude >= apogee) {
        apogeeTime = time_us_64() - STARTTIMEMICRO;
        apogee = altitude;
    } 
    if(time_us_64() - STARTTIMEMICRO > 300000000) {
        return true;
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
    w->unmount();
}

void calcWriteLoop() {
    DataBuffer* curRead = nullptr;
    float altitude = 0.0;
    while(!endCondition(altitude)) {
        if(queue->size() > 0) {
            curRead = queue->dequeue();
            c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
            w->writeData(curRead);
            if(!BMPFail && curRead->sensorNum == 0) { //based on bmp altitude
                altitude = curRead->data.values[0];
            } else if(BMPFail && !GTUFail && curRead->sensorNum == 2) {
                altitude = curRead->data.values[0];
            } else if(BMPFail && GTUFail) { //rely on endCondition ending after 300 seconds
                altitude = 0.0;
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
    sleep_ms(2500);
    setup();
    blink(20, 100);
    buzz(50);
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    while(!startCondition()) {
        tight_loop_contents();
    }
    printf("started\n");
    multicore_launch_core1(calcWriteLoop);
    samplingLoop();
    closeDown();
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    return 0;
}