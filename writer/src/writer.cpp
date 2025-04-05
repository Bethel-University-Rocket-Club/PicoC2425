#include "writer.h"
#include <stdio.h>

Writer::Writer(sd_card_t* sdCard) {
    this->sdCard = sdCard;
    FRESULT fr = FR_OK;
    for(int i = 0; i < 5; i++) {
        fr = f_mount(&sdCard->fatfs, sdCard->pcName, 1);
        if (FR_OK != fr) {
            blink(5, 100);
        } else break;
    }
    fr = f_open(&fileOut, "data.csv", FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr) {
        blink(10, 100);
    }
}

bool Writer::writeHeader() {
    int length;
    const char* portion = headerTime(length);
    writeData(portion, length);
    writeData(",", 1);
    portion = headerSample(length);
    writeData(portion, length);
    writeData(",", 1);
    portion = headerBMP(length);
    writeData(portion, length);
    writeData(",", 1);
    portion = headerMPU(length);
    writeData(portion, length);
    writeData(",", 1);
    portion = headerGT(length);
    writeData(portion, length);
    writeData(",", 1);
    portion = headerPitot(length);
    writeData(portion, length);
    writeData("\n", 1);
    return true;
}

bool Writer::close() {
    f_close(&fileOut) == FR_OK;
    return f_unmount(sdCard->pcName);
}

bool Writer::writeData(DataBuffer *data) {
    if(latestUnwrittenIndex + 200 > SDBUFSIZE) {
        flush();
    }
    uint8_t len = 0;
    formatData(&writeBuf[latestUnwrittenIndex], len, data);
    latestUnwrittenIndex += len;
    return true;
}

bool Writer::flush() {
    uint bw = 0;
    FRESULT fr = FR_OK;
    fr = f_write(&fileOut, writeBuf, latestUnwrittenIndex, &bw);
    if(bw != latestUnwrittenIndex) {
        latestUnwrittenIndex = 0;
        return false;
    } else {
        latestUnwrittenIndex = 0;
        return fr == FR_OK;
    }
}

bool Writer::writeData(const char *data, int length) {
    if(latestUnwrittenIndex + length > SDBUFSIZE - 200) {
        flush();
    }
    for(int i = 0; i < length; i++) {
        writeBuf[latestUnwrittenIndex+i] = data[i];
    }
    latestUnwrittenIndex += length;
    return true;
}

const char *Writer::headerTime(int &length) {
    length = 4;
    return "Time";
}

const char *Writer::headerSample(int &length) {
    length = 28;
    return "AggregateSample,SensorSample";
}

const char *Writer::headerBMP(int &length) {
    length = 29;
    return "BMP280Alt,BMP280Vel,BMP280Acc";
}

const char *Writer::headerMPU(int &length) {
    length = 32;
    return "MPU6050Alt,MPU6050Vel,MPU6050Acc";
}

const char *Writer::headerGT(int &length) {
    length = 26;
    return "GT-U7Alt,GT-U7Vel,GT-U7Acc";
}

const char *Writer::headerPitot(int &length) {
    length = 26;
    return "PitotAlt,PitotVel,PitotAcc";
}

bool Writer::formatData(char* startLocation, uint8_t& length, DataBuffer *data) {
    length = 0;
    uint8_t length2 = 0;
    formatTime(startLocation, length2, data);
    length += length2;
    formatSamples(&startLocation[length], length2, data);
    length += length2;
    formatNullBeforeSamples(&startLocation[length], length2, data);
    length += length2;
    formatDataSample(&startLocation[length], length2, data);
    length += length2;
    formatNullAfterSamples(&startLocation[length], length2, data);
    length += length2;
    startLocation[length] = '\n';
    length++;
    return true;
}

bool Writer::formatTime(char *startLocation, uint8_t &length, DataBuffer *data) {
    length = 0;
    formatInt64(startLocation, length, data->elapsedTime);
    startLocation[length] = ',';
    length++;
    return true;
}

bool Writer::formatSamples(char *startLocation, uint8_t &length, DataBuffer *data) {
    length = 0;
    formatInt32(startLocation, length, data->aggSampleNum);
    startLocation[length] = ',';
    length++;
    uint8_t length2 = 0;
    formatInt32(&startLocation[length], length2, data->sensorSampleNum);
    length += length2;
    startLocation[length] = ',';
    length++;
    return true;
}

bool Writer::formatNullBeforeSamples(char *startLocation, uint8_t &length, DataBuffer *data) {
    length = (data->sensorNum)*3;
    startLocation += length;
    uint8_t count = length;
    while(count-- > 0) {
        *--startLocation = ',';
    }
    return true;
}

bool Writer::formatDataSample(char *startLocation, uint8_t &length, DataBuffer *data) {
    length = 0;
    float* floats = data->data.values;
    for(uint8_t i = 0, length2 = 0; i < 3; i++) {
        formatFloat(&startLocation[length], length2, floats[i]);
        length += length2;
        startLocation[length] = ',';
        length++;
    }
    return true;
}

bool Writer::formatNullAfterSamples(char *startLocation, uint8_t &length, DataBuffer *data) {
    uint8_t sensorsAfter = (data->sensorMax - data->sensorNum);
    length = sensorsAfter*3-(sensorsAfter != 0);
    startLocation += length;
    uint8_t count = length;
    while(count-- > 0) {
        *--startLocation = ',';
    }
    return true;
}
//5 decimal places of precision
bool Writer::formatFloat(char *startLocation, uint8_t &length, float val) {
    static const uint32_t pow10[] = {10, 100, 1000, 10000, 100000};
    length = 0;
    // --- Handle Negative Numbers ---
    bool negative = (val < 0.0f) && !(*(uint32_t*)&val == (1 << 31)); // Exclude -0.0f
    *startLocation = '-';
    startLocation += negative;
    // branchless make it positive
    uint32_t intval = *(uint32_t *) &val;
    intval ^= negative << 31;
    val = *(float *)&intval;
    // --- Split Integer part ---
    uint32_t integer = (uint32_t)val;
    //split fraction part
    float frac = val - integer;
    uint32_t fractional = (uint32_t)(frac * 10000 + 0.5f);
    integer += (fractional >= 10000);
    fractional = fractional * !(fractional >= 10000);
    // --- Write Integer Part ---
    formatInt32(startLocation, length, integer);
    startLocation += length;
    length += negative;

    // --- Write Fractional Part with Leading Zeros ---
    uint8_t leadingZCount = 4 - countDigits10(fractional);
    leadingZCount = leadingZCount * (fractional != 0);
    //only write '.' if decimal portion != 0, same lgic for length
    *startLocation = '.';
    startLocation += (fractional != 0);
    length += (fractional != 0) + leadingZCount;
    
    //write leading zeros
    for (uint8_t i = 0; i < leadingZCount; i++) {
        *(startLocation++) = '0';      // Write '0' and always increment
    }
    uint8_t length2 = 0;
    //write fractional part
    formatInt32(startLocation, length2, fractional);
    
    //modify length2 by how many zeros we find
    for(bool notZero = false; length2 > 0; length2--) {
        notZero = !(startLocation[length2-1] == '0') | notZero;
        length += notZero;
    }
    
    return true;
}

bool Writer::formatInt32(char *startLocation, uint8_t &length, uint32_t val) {
    uint8_t count = countDigits10(val);
    length = count;
    startLocation += length;
    while(count-- > 0) {
        *--startLocation = '0' + val%10;
        val /= 10;
    }
    return true;
}

bool Writer::formatInt64(char *startLocation, uint8_t &length, uint64_t val) {
    uint8_t count = countDigits20(val);
    length = count;
    startLocation += length;
    while(count-- > 0) {
        *--startLocation = '0' + val%10;
        val /= 10;
    }
    return true;
}
//quickly counts how many digits (up to 20) are in a 64bit uint
inline uint8_t Writer::countDigits20(uint64_t num) {
    static const uint64_t pow10[] = {0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 
        10000000000, 100000000000, 1000000000000, 10000000000000, 100000000000000, 1000000000000000, 10000000000000000,
        100000000000000000, 1000000000000000000, (uint64_t)10000000000000000000};
    uint8_t log2 = 64 - __builtin_clzll(num | 1);  // Step 1: Bit length
    uint8_t d = (log2 * 1233) >> 12;              // Step 2: log10 approximation
    d -= (num < pow10[d]);                        // Step 3: Adjust overestimation
    return d + 1;                                 // Step 4: Digit count
}
//quickly counts how many digits (up to 10) are in a 32bit uint
inline uint8_t Writer::countDigits10(uint32_t num) {
    static const uint32_t pow10[] = {0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
    uint8_t log2 = 32 - __builtin_clz(num | 1);  // Step 1: Bit length
    uint8_t d = (log2 * 1233) >> 12;              // Step 2: log10 approximation
    d -= (num < pow10[d]);                        // Step 3: Adjust overestimation
    return d + 1;    
}