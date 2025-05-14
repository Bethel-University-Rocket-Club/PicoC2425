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
#include "sdwriter.h"
#include "circularQueue.h"
#include "hw_config.h"

Devices* d;
SDWriter* sdw;
Writer* w;
Calculator* c;
BMP280* bmp;
ADXL377* adx;
GTU7* gtu;
MPX5700GP* mpx;
CircularQueue* queue;

bool END = false;
bool DONEWRITE = false;
uint64_t STARTTIMEMICRO = 0.0;
int BMPID;
int ADXID;
int GTUID;
int MPXID;
bool BMPFail = false;
bool ADXFail = false;
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
    c = new Calculator();
    queue = new CircularQueue();
    bmp = d->GetPressureSensor();
    adx = d->GetAccelerometer();
    gtu = d->GetGPS();
    mpx = d->GetPitotTube();
    w = new Writer();
    sdw = new SDWriter(sd);
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
    while(!adx->checkConnection() && failCount < 2) {
        printf("ADX error\n");
        //buzz(20000);
        blink(2, 1000);
        blink(10, 250);
        failCount++;
    }
    if(!adx->checkConnection() && failCount == 2) ADXFail = true;
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
    while(!sdw->checkConnection()) {
        printf("WRITER error\n");
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
    gpio_init(PRINT_PIN);
    gpio_init(SDWRITE_PIN);
    gpio_set_dir(SDWRITE_PIN, GPIO_IN);
    gpio_set_dir(PRINT_PIN, GPIO_IN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(BUZZ_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    buzz(50);
    if(gpio_get(PRINT_PIN) || gpio_get(SDWRITE_PIN)) {
        return false;
    }
    validateSensors();
    printf("validated\n");
    //blink(3, 500); //remove eventually
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sdw->writeHeader();
    printf("header written\n");
    bmp->setSeaPressure(SEAPRESSURE);
    mpx->setAirDensity(AIRDENSITY);
    float measure = 0.0;
    BMPID = c->addSensor(&Calculator::bmp280Calculations);
    ADXID = c->addSensor(&Calculator::mpu6050Calculations);
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
        //ADX arrow direction indicates that if oriented downwards, gravity is negative
        adx->getAccelY(accelUp);
        bmp->getAltitude(alt);
        gtu->getAltitude(alt2);
        if(!ADXFail) {
            if(accelUp*9.8 > THRESHOLD || accelUp*9.8 < THRESHOLD*-1){
                //if(accelUp*9.8 > 2) {
                STARTTIMEMICRO = time_us_64();
                c->configureInitialOffset(GTUID, alt2);
                c->configureInitialOffset(BMPID, alt);
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
        } else if(ADXFail && !BMPFail) {
            //printf("barometric\n");
            if(alt - c->getInitialOffset(BMPID) > 15) {
                STARTTIMEMICRO = time_us_64();
                c->configureInitialOffset(GTUID, alt2);
                c->configureInitialOffset(BMPID, alt);
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
        } else if(ADXFail && BMPFail && !GTUFail) {
            //printf("gps\n");
            if(alt2 - c->getInitialOffset(GTUID) > 15) {
                STARTTIMEMICRO = time_us_64();
                c->configureInitialOffset(GTUID, alt2);
                c->configureInitialOffset(BMPID, alt);
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
    //5 second test
    /*
    if(time_us_64() - STARTTIMEMICRO > 5000000) {
        printf("end\n");
        fflush(stdout);
        return true;
    }*/
    //printf("alt: %f, apg: %f end: %d\n", altitude, apogee, ended);
    if(altitude < apogee-5) {
        //1 second of the measurement being below apogee
        if(time_us_64() - apogeeTime > 1000000) {
            printf("endApogee\n");
            fflush(stdout);
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
        w->writeData(curRead, true);
        queue->finishDequeue();
    }
    printf("closeDown\n");
    w->flush();
    sdw->flush();
    printf("flushed\n");
    w->close(true);
    sdw->close();
    printf("closed\n");
    w->unmount();
    sdw->unmount();
    printf("unmounted\n");
    fflush(stdout);
}

void calcWriteLoop() {
    DataBuffer* curRead = nullptr;
    float altitude = 0.0;
    while(!endCondition(altitude)) {
        if(queue->size() > 0) {
            curRead = queue->dequeue();
            c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
            sdw->writeData(curRead);
            bool hasSpace = w->writeData(curRead, false);
            if(!hasSpace) {
                printf("noSPace\n");
                break;
            }
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
    uint64_t analogSampleTime = STARTTIMEMICRO;
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
        if(elapsedTime - analogSampleTime > 500) {
            analogSampleTime = elapsedTime;
            if(adx->getAccelY(measure) && !queue->isFull()) {
                curWrite = queue->startEnqueue();
                if(curWrite != nullptr) {
                    measure *= -9.8;
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
}

int main() {
    stdio_init_all();
    adc_init();
    sleep_ms(1000);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    if(setup()) {
        printf("collecting\n");
        blink(20, 100);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        buzz(5000);
        printf("beginClearFlash\n");
        w->clearFlash();
        printf("endClearFlash\n");
        buzz(50);
        while(!startCondition()) {
            tight_loop_contents();
        }
        printf("started\n");
        multicore_lockout_victim_init();
        multicore_launch_core1(calcWriteLoop);
        samplingLoop();
        closeDown();
        printf("ended\n");
        fflush(stdout);
    } else {
        if(gpio_get(PRINT_PIN)) {
            blink(20, 200);
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            printf("printing\n");
            char received = getchar();
            w->printData();
        } else {
            blink(20, 200);
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            w = new Writer();
            sd_card_t* sd = sd_get_by_num(0);
            sd->spi->hw_inst = spi0;
            sd->spi->miso_gpio = SDCARDMISO;
            sd->spi->mosi_gpio = SDCARDMOSI;
            sd->spi->sck_gpio = SDCARDSCK;
            sd->ss_gpio = SDCARDCS;
            sdw = new SDWriter(sd);
            blink(20, 200);
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            printf("sdWriting\n");
            char received = getchar();
            if(!w->writeDataTo(sdw)) {
                printf("Failed to write\n");
                buzz(50000);
            }
        }
    }
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    fflush(stdout);
    return 0;
}