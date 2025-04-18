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
    return true;
}

bool setup() {
    // Configure the onboard LED GPIO as an output.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    if(!validateSensors()) {
        blink(1000, 100);
    }
    blink(3, 500); //remove eventually
    w->writeHeader();
    w->flush();
    bmp->setSeaPressure(1013.25);
    mpx->setAirDensity(1.225);
    float measure = 0.0;
    BMPID = c->addSensor(&Calculator::bmp280Calculations);
    MPUID = c->addSensor(&Calculator::mpu6050Calculations);
    MPXID = c->addSensor(&Calculator::mpx5700gpCalculations);
    GTUID = c->addSensor(&Calculator::gtu7Calculations);
    bmp->getAltitude(measure);
    c->configureInitialOffset(BMPID, measure);
    gtu->getAltitude(measure);
    c->configureInitialOffset(GTUID, measure);
    mpx->getDrift(measure);
    c->configureInitialOffset(MPXID, measure);
    printf("setup\n");
    return true;
}

bool startCondition() {
    float accelUp = 0.0;
    while(true) {
        mpu->getAccelZ(accelUp);
        if(accelUp > THRESHOLD) {
            STARTTIMEMICRO = time_us_64();
            c->setStartTime(STARTTIMEMICRO * 0.000001);
            printf("Starting\n");
            return true;
        }
    }
    return false;
}

bool endCondition(float& altitude) {
    static float apogee = 0.0;
    static uint32_t apogeeTime = STARTTIMEMICRO;
    if(time_us_64() - STARTTIMEMICRO > 1000000) {
        printf("end\n");
        return true;
    }
    if(altitude < apogee) {
        //1 second of the measurement being below apogee
        if(time_us_64() - apogeeTime > 1000000) {
            return true;
        }
        return false;
    } else {
        apogeeTime = time_us_64() - STARTTIMEMICRO;
        apogee = altitude;
        return false;
    }
}

void calcWriteLoop() {
    printf("startCore1");
    DataBuffer* curRead = nullptr;
    float altitude = 0.0;
    while(!endCondition(altitude)) {
        if(queue->size() > 0) {
            printf("QSize:%d\n", queue->size());
            curRead = queue->dequeue();
            c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
            w->writeData(curRead);
            if(curRead->sensorNum == 0) {
                //altitude = curRead->sensorSample;
            }
            queue->finishDequeue();
        }
    }
    END = true;
    while(queue->size() > 0) {
        curRead = queue->dequeue();
        c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
        w->writeData(curRead);
        if(curRead->sensorNum == 0) {
            altitude = curRead->sensorSample;
        }
        queue->finishDequeue();
    }
    w->flush();
    w->close();
    DONEWRITE = true;
}

void samplingLoop() {
    printf("startCore0\n");
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
            aggSampleCount++;
            countBMP++;
            curWrite = queue->startEnqueue();
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
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        if(mpu->getAccelZ(measure) && !queue->isFull()) {
            aggSampleCount++;
            countMPU++;
            curWrite = queue->startEnqueue();
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
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        if(gtu->getAltitude(measure) && !queue->isFull()) {
            aggSampleCount++;
            countGTU++;
            curWrite = queue->startEnqueue();
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
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        if(mpx->getVelocity(measure) && !queue->isFull()) {
            aggSampleCount++;
            countMPX++;
            curWrite = queue->startEnqueue();
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

void writeToUART0(const char* sentence, uint32_t length) {
    for(uint32_t i = 0; i < length; i++) {
        uart_putc(uart0, sentence[i]);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(100);
    setup();
    while(!startCondition()) {
        tight_loop_contents();
    }
    multicore_launch_core1(calcWriteLoop);
    samplingLoop();
    while(!DONEWRITE) {
        tight_loop_contents();
    }
    printf("done\n");
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
    return 0;
}