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

//pointers to objects used
Devices* d; //creates sensor objects
SDWriter* sdw; //writes to a connected sd card
Writer* w; //writes to flash memory
Calculator* c; //does calculations on received sensor data
BMP280* bmp; //interfaces with the BMP280 sensor (pressure sensor)
ADXL377* adx; //interfaces with the ADXL377 sensor (analog high G accelerometer)
GTU7* gtu; //interfaces with the GTU7 sensor (gps)
MPX5700GP* mpx; //interfaces with the MPX5700GP sensor (pitot tube)
//how information is passed between core 0 and 1 (collecting and writing)
CircularQueue* queue; //allows for batch writing

bool END = false; //if the time for collecting data is over
uint64_t STARTTIMEMICRO = 0.0; //start time of data collection
int BMPID; //for the calculator to identify
int ADXID; //for the calculator to identify
int GTUID; //for the calculator to identify
int MPXID; //for the calculator to identify
//if initialization failed
bool BMPFail = false;
bool ADXFail = false;
bool GTUFail = false;
bool MPXFail = false;

/*
 * validateSensors - creates and validates peripheral objects/connections
 * Returns - true, see above flags for failed initializations
*/
bool validateSensors() {
    //sd card library setup
    sd_card_t* sd = sd_get_by_num(0);
    sd->spi->hw_inst = spi0;
    sd->spi->miso_gpio = SDCARDMISO;
    sd->spi->mosi_gpio = SDCARDMOSI;
    sd->spi->sck_gpio = SDCARDSCK;
    sd->ss_gpio = SDCARDCS;
    //object to create peripheral management objects
    d = new Devices();
    c = new Calculator();
    queue = new CircularQueue();

    bmp = d->GetPressureSensor();
    adx = d->GetAccelerometer();
    gtu = d->GetGPS();
    mpx = d->GetPitotTube();

    w = new Writer();
    sdw = new SDWriter(sd);
    //check if the BMP280 has a good connection
    uint8_t failCount = 0;
    while(!bmp->checkConnection() && failCount < 2) {
        printf("BMP error\n");
        //buzz(10000); //pressure sensor problem
        //blink(1, 1000); //pressure sensor problem
        //blink(10, 250); //trying again
        failCount++;
    }
    if(!bmp->checkConnection() && failCount == 2) BMPFail = true;
    failCount = 0;
    sleep_ms(10);
    //gpio_put(PICO_DEFAULT_LED_PIN, 1);
    //check if the ADXL377 has a good connection
    while(!adx->checkConnection() && failCount < 2) {
        //buzz(20000); //accelerometer problem
        //blink(2, 1000); //accelerometer problem
        //blink(10, 250); //trying again
        failCount++;
    }
    if(!adx->checkConnection() && failCount == 2) ADXFail = true;
    failCount = 0;
    sleep_ms(10);
    //gpio_put(PICO_DEFAULT_LED_PIN, 1);
    //check if the GTU7 has a good connection
    while(!gtu->checkConnection() && failCount < 2) {
        printf("GTU error\n");
        //buzz(30000); //gps problem
        //blink(3, 1000); //gps problem
        //blink(10, 250); //trying again
        failCount++;
    }
    if(!gtu->checkConnection() && failCount == 2) GTUFail = true;
    failCount = 0;
    sleep_ms(10);
    //gpio_put(PICO_DEFAULT_LED_PIN, 1);
    //check if the MPX5700GP has a good connection
    while(!mpx->checkConnection()) {
        printf("MPX error\n");
        //buzz(4000);
        //blink(4, 1000);
        //blink(10, 250);
    }
    sleep_ms(10);
    //gpio_put(PICO_DEFAULT_LED_PIN, 1);
    //check if the sd card has a good connection
    while(!sdw->checkConnection()) {
        printf("WRITER error\n");
        //buzz(50000); //sdcard problem
        //blink(5, 1000); //sdcard problem
        //blink(10, 250); //trying again
    }
    sleep_ms(10);
    gpio_put(PICO_DEFAULT_LED_PIN, 1); //indicates validation is done
    return true;
}

/*
 * setOffsets - sets value to be subtracted from future sensor measurements that
 * need to be zeroed at start.
*/
void setOffsets() {
    float measure = 0.0;
    bmp->getAltitude(measure);
    c->configureInitialOffset(BMPID, measure);
    gtu->getAltitude(measure);
    c->configureInitialOffset(GTUID, measure);
    mpx->getDrift(measure);
    c->configureInitialOffset(MPXID, measure);
}

/*
 * setup - performs all setup work to verify all sensors and architecture are ready to be used
 * Return - true if everything is usable, false otherwise
*/
bool setup() {
    //configure GPIO pins
    gpio_init(LED_PIN);
    gpio_init(BUZZ_PIN);
    gpio_init(PRINT_PIN);
    gpio_init(SDWRITE_PIN);
    gpio_set_dir(SDWRITE_PIN, GPIO_IN);
    gpio_set_dir(PRINT_PIN, GPIO_IN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(BUZZ_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    //buzz(50); //first buzz, indicating pico has power and is starting to do stuff
    if(gpio_get(PRINT_PIN) || gpio_get(SDWRITE_PIN)) {
        return false;
    }
    validateSensors();
    printf("validated\n");
    gpio_put(PICO_DEFAULT_LED_PIN, 1); //indicates validation is done
    sdw->writeHeader(); //writes the header of the sd card data file
    printf("header written\n");
    //set initial conditions for sensors that need them
    bmp->setSeaPressure(SEAPRESSURE);
    mpx->setAirDensity(AIRDENSITY);

    float measure = 0.0;
    //setup the calculator to be ready to calculate for each sensor
    BMPID = c->addSensor(&Calculator::bmp280Calculations);
    ADXID = c->addSensor(&Calculator::mpu6050Calculations);
    GTUID = c->addSensor(&Calculator::gtu7Calculations);
    MPXID = c->addSensor(&Calculator::mpx5700gpCalculations);
    //configures initial state for the calculator for future calculations
    //will be subtracted from all future measurements. So only used for altitude and drift
    //so both are zeroed
    setOffsets();
    return true;
}

/*
 * startCondition - blocking. checks if it is time to start collecting data - rocket launch
 * Return - true when data should begin to be collection
*/
bool startCondition() {
    float accelUp = 0.0;
    float alt = 0.0;
    float alt2 = 0.0;
    //until launch
    while(true) {
        adx->getAccelY(accelUp);
        bmp->getAltitude(alt);
        gtu->getAltitude(alt2);
        //checks if the accelerometer is usable to determine when launch occurs
        if(!ADXFail) {
            if(accelUp*9.8 > THRESHOLD || accelUp*9.8 < THRESHOLD*-1){
                //if(accelUp*9.8 > 2) {
                STARTTIMEMICRO = time_us_64();
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
        //backup check uses the pressure sensor to determine when launch occurs
        } else if(ADXFail && !BMPFail) {
            //printf("barometric\n");
            if(alt - c->getInitialOffset(BMPID) > 15) {
                STARTTIMEMICRO = time_us_64();
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
        //final backup check uses the gps to determine when launch occurs
        } else if(ADXFail && BMPFail && !GTUFail) {
            //printf("gps\n");
            if(alt2 - c->getInitialOffset(GTUID) > 15) {
                STARTTIMEMICRO = time_us_64();
                c->setStartTime(STARTTIMEMICRO * 0.000001);
                return true;
            }
        }
    }
    return false;
}

/*
 * endCondition - non-blocking. Checks if it is time to stop collecting data - apogee.
 * Returns - true if at/after apogee, false otherwise
*/
bool endCondition(float altitude) {
    //for persistent use across function calls
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
    //if the new measurement indicates we are going down
    //-5 for a small buffer for extra security
    if(altitude < apogee-5) {
        //1 second of the measurement being below apogee
        //so a single altitude value doesn't cause premature ending
        if(time_us_64() - apogeeTime > 1000000) {
            printf("endApogee\n");
            fflush(stdout);
            return true;
        }
    //checks if the rocket is assuredly not going down
    } else if(altitude >= apogee) {
        //update persistent variables
        apogeeTime = time_us_64() - STARTTIMEMICRO;
        apogee = altitude;
    } 
    return false;
}

/*
 * closeDown - writes the remaining batched data, closes use of flash memory and the sdcard
*/
void closeDown() {
    DataBuffer* curRead = nullptr;
    //empty the queue of un saved data
    while(queue->size() > 0) {
        curRead = queue->dequeue();
        c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
        w->writeData(curRead, true);
        queue->finishDequeue();
    }
    //ensure all data is written
    printf("closeDown\n");
    w->flush();
    sdw->flush();
    printf("flushed\n");
    //properly close the file
    w->close(true);
    sdw->close();
    printf("closed\n");
    //properly ensure the resources are dismounted
    w->unmount();
    sdw->unmount();
    printf("unmounted\n");
    fflush(stdout);
}

/*
 * calcWriteLoop - calculates altitude, velocity, acceleration for each sample. 
 * Writes the data calculated to flash memory and the sd card.
*/
void calcWriteLoop() {
    DataBuffer* curRead = nullptr;
    float altitude = 0.0;
    //while data should still be collected
    while(!endCondition(altitude)) {
        //if the queue is not empty
        if(queue->size() > 0) {
            //get a pointer to the next location that needs to be written
            curRead = queue->dequeue();
            //calculate altitude, velocity, acceleration
            c->newSample(curRead->sensorNum, curRead->sensorSample, curRead->elapsedTime * 0.000001, curRead->data);
            sdw->writeData(curRead); //writes the data to the sdcard
            bool hasSpace = w->writeData(curRead, false); //tries to write data to flash memory from core 1
            //if flash memory cannot contain more data, stop trying to write fully
            if(!hasSpace) {
                printf("noSPace\n");
                break;
            }
            //update altitude 
            if(!BMPFail && curRead->sensorNum == 0) { //based on bmp altitude
                altitude = curRead->data.values[0];
            } else if(BMPFail && !GTUFail && curRead->sensorNum == 2) {
                altitude = curRead->data.values[0];
            }
            queue->finishDequeue(); //signify that position can be written to again
            if(BMPFail && GTUFail) { //rely on endCondition ending after 300 seconds
                if(time_us_64() - STARTTIMEMICRO > 300000000) {
                    break;
                }
            }
        }
    }
    END = true; //signify to core0 (data collection) to stop
}

/*
 * samplingLoop - collects samples from all sensors at as high a rate as possible
 * sending each sample into the queue
*/
void samplingLoop() {
    DataBuffer* curWrite = nullptr;
    uint64_t elapsedTime = time_us_64() - STARTTIMEMICRO;
    uint32_t aggSampleCount = 0;
    uint32_t countBMP = 0;
    uint32_t countMPU = 0;
    uint32_t countGTU = 0;
    uint32_t countMPX = 0;
    uint64_t adxSampleTime = STARTTIMEMICRO;
    uint64_t mpuSampleTime = STARTTIMEMICRO;
    float measure = 0.0;
    while(!END) {
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        //check if the bmp has a new sample, and if the queue is not full
        if(bmp->getAltitude(measure) && !queue->isFull()) {
            //get the next writable position
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                //write the sensor value and other necessary data
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
                //the sample is ready to be written
                queue->finishEnqueue();
            }
        }
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        //check if the gtu has a new sample and if the queue is not full
        if(gtu->getAltitude(measure) && !queue->isFull()) {
            //get the next writable position
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                //write the sensor value and other necessary data
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
                //the sample is ready to be written
                queue->finishEnqueue();
            }
        }
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        //its analog, limit sampling rate to preserve power use
        //collect a sample from the adx once every 5000 microseconds, if the queue is not full
        if(elapsedTime - adxSampleTime > 5000 && adx->getAccelY(measure) && !queue->isFull()) {
            adxSampleTime = elapsedTime;
            //get the next writable position
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                //write the sensor value and other necessary data
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
                //the sample is ready to be written
                queue->finishEnqueue();
            }
        }
        elapsedTime = time_us_64() - STARTTIMEMICRO;
        //its analog, limit sampling rate to preserve power use
        //collect a sample from the adx once every 1000 microseconds, if the queue is not full
        if(elapsedTime - mpuSampleTime > 1000 && mpx->getVelocity(measure) && !queue->isFull()) {
            mpuSampleTime = elapsedTime;
            //get the next writable position
            curWrite = queue->startEnqueue();
            if(curWrite != nullptr) {
                //write the sensor value and other necessary data
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
                //the sample is ready to be written
                queue->finishEnqueue();
            }
        }
    }
}

/*
 * main - entrypoint for collecting and writing sensor samples
*/
int main() {
    stdio_init_all();
    adc_init();
    sleep_ms(1000); //so everything initializes and has time to settle
    gpio_put(PICO_DEFAULT_LED_PIN, 1); //indicates the pico is on and active
    if(setup()) { //only false if we are reading from flash memory
        printf("collecting\n");
        //blink(20, 100); /signifies sensors are validated
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        //buzz(5000); //warning that flash is about to be erased
        printf("beginClearFlash\n");
        w->clearFlash(); //reset so it is ready to be written to
        //printf("endClearFlash\n");
        //buzz(50); //third buzz, ready for launch
        while(!startCondition()) { //wait for start
            tight_loop_contents();
        }
        //printf("started\n");
        multicore_lockout_victim_init(); //core1 writes to flash, so core0 needs to be locked at time
        multicore_launch_core1(calcWriteLoop); //launch the calculating and writing loop on core1
        samplingLoop(); //begin sampling on core0
        closeDown(); //ensure everything ends gracefully
        //printf("ended\n");
        fflush(stdout);
    } else { //if we are reading from flash memory
        if(gpio_get(PRINT_PIN)) { //print data from flash memory
            //blink(20, 200); //signifies reading from flash, and doing something on user input
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            printf("printing\n");
            char received = getchar(); //on user input
            w->printData(); //print to serial
        } else { //copies data from flash to an sd card
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            w = new Writer();
            sd_card_t* sd = sd_get_by_num(0);
            sd->spi->hw_inst = spi0;
            sd->spi->miso_gpio = SDCARDMISO;
            sd->spi->mosi_gpio = SDCARDMOSI;
            sd->spi->sck_gpio = SDCARDSCK;
            sd->ss_gpio = SDCARDCS;
            sdw = new SDWriter(sd);
            //blink(20, 200); //signifies reading from flash, and doing something on user input
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            printf("sdWriting\n");
            char received = getchar(); //on user input
            if(!w->writeDataTo(sdw)) { //copy data
                printf("Failed to write\n");
                //buzz(50000); //sdcard not ready somehow
            }
        }
    }
    gpio_put(PICO_DEFAULT_LED_PIN, 0); //indicates the pico is done working
    fflush(stdout);
    return 0;
}