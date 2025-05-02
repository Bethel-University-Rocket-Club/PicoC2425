#include "writer.h"

Writer::Writer(sd_card_t* sdCard) {
    this->sdCard = sdCard;
    curName = "data.csv";
    FRESULT fr = FR_OK;
    for(int i = 0; i < 5; i++) {
        if(mount()) {
            mounted = true;
        }
    }
    fileOpen = this->open(curName);
    //fr = f_open(&fileOut, "data.csv", FA_OPEN_APPEND | FA_WRITE);
    if(fr == FR_OK) fileOpen = true;
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
    uint64_t startClose = time_us_64();
    while(f_close(&fileOut) != FR_OK) {
        if(time_us_64() - startClose > 1000000) return false;
        sleep_ms(100);
    }
    return true;
}

bool Writer::open(const char* name) {
    uint64_t startClose = time_us_64();
    while(f_open(&fileOut, name, FA_OPEN_APPEND | FA_WRITE) != FR_OK) {
        if(time_us_64() - startClose > 1000000) return false;
        sleep_ms(100);
    }
    curName = name;
    return true;
}

bool Writer::unmount() {
    uint64_t startClose = time_us_64();
    while(f_unmount(sdCard->pcName) != FR_OK) {
        if(time_us_64() - startClose > 1000000) return false;
        sleep_ms(100);
    }
    return true;
}

bool Writer::mount() {
    uint64_t startClose = time_us_64();
    while(f_mount(&sdCard->fatfs, sdCard->pcName, 1) != FR_OK) {
        if(time_us_64() - startClose > 1000000) return false;
        sleep_ms(100);
    }
    return true;
}

bool Writer::writeData(DataBuffer *data)
{
    if(latestUnwrittenIndex > SDBUFSIZE / 2 - 200) {
        flush();
    }
    uint8_t len = 0;
    formatData(&writeBuf[latestUnwrittenIndex], len, data);
    latestUnwrittenIndex += len;
    return true;
}

bool Writer::flush() {
    if(!checkConnection()) {
        fileOpen = false;
        mounted = false;
        return false;
    } else {
        FRESULT fr = FR_OK;
        if(!mounted) {
            if(!mount()){
                f_unmount(sdCard->pcName);
                mounted = false;
                return false;
            }
            mounted = true;
        }
        if(!fileOpen) {
            if(!open(curName)) {
                close();
                fileOpen = false;
                f_unmount(sdCard->pcName);
                mounted = false;
                return false;
            }
            fileOpen = true;
        }
    }
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

bool Writer::checkConnection()
{
    return sdCard->sd_test_com(sdCard);
}

bool Writer::writeData(const char *data, int length) {
    if(latestUnwrittenIndex + length > SDBUFSIZE / 2 - 200) {
        flush();
    }
    for(int i = 0; i < length; i++) {
        writeBuf[latestUnwrittenIndex+i] = data[i];
    }
    latestUnwrittenIndex += length;
    return true;
}

const char *Writer::headerTime(int &length) {
    length = 8;
    return "Time(us)";
}

const char *Writer::headerSample(int &length) {
    length = 28;
    return "AggregateSample,SensorSample";
}

const char *Writer::headerBMP(int &length) {
    length = 44;
    return "BMP280Alt(m),BMP280Vel(m/s),BMP280Acc(m/s/s)";
}

const char *Writer::headerMPU(int &length) {
    length = 47;
    return "MPU6050Alt(m),MPU6050Vel(m/s),MPU6050Acc(m/s/s)";
}

const char *Writer::headerGT(int &length) {
    length = 41;
    return "GT-U7Alt(m),GT-U7Vel(m/s),GT-U7Acc(m/s/s)";
}

const char *Writer::headerPitot(int &length) {
    length = 41;
    return "PitotAlt(m),PitotVel(m/s),PitotAcc(m/s/s)";
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
        formatFloat(&startLocation[length], length2, floats[i], data->sigDecimalDigits);
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

bool Writer::formatFloat(char *startLocation, uint8_t &length, float val, uint8_t decimalPrecision) {
    if(decimalPrecision > 5) {
        decimalPrecision = 5;
    }
    static const uint32_t pow10[] = {1, 10, 100, 1000, 10000, 100000};
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
    uint32_t fractional = (uint32_t)(frac * pow10[decimalPrecision] + 0.5f);
    integer += (fractional >= pow10[decimalPrecision]);
    fractional = fractional * !(fractional >= pow10[decimalPrecision]);
    // --- Write Integer Part ---
    formatInt32(startLocation, length, integer);
    startLocation += length;
    length += negative;

    // --- Write Fractional Part with Leading Zeros ---
    uint8_t leadingZCount = decimalPrecision - countDigits10(fractional);
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
        (uint64_t)100000000000000000, (uint64_t)1000000000000000000, (uint64_t)10000000000000000000};
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