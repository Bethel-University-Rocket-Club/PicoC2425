#ifndef WRITER_H
#define WRITER_H
#include "common.h"
#include "sd_card.h"
#include "ff.h"
#include "f_util.h"

#ifndef SDBUFSIZE
#define SDBUFSIZE 8192
#endif
#ifndef USTIMESIZE
#define USTIMESIZE 12
#endif
#ifndef ADDSAMPLESIZE
#define AGGSAMPLESIZE 10
#endif
#ifndef INDSAMPLESIZE
#define INDSAMPLESIZE 10
#endif
#ifndef FLOATNUSMIZE
#define FLOATNUMSIZE 8
#endif

class Writer {
    public:
    Writer(sd_card_t* sdCard);
    bool writeHeader();
    bool close();
    bool writeData(DataBuffer* data);
    bool flush();
    bool checkConnection();

    private:
    bool writeData(const char* data, int length);
    const char* headerTime(int& length);
    const char* headerSample(int& length);
    const char* headerBMP(int& length);
    const char* headerMPU(int& length);
    const char* headerGT(int& length);
    const char* headerPitot(int& length);
    bool formatData(char* startLocation, uint8_t& length, DataBuffer* data);
    bool formatTime(char* startLocation, uint8_t& length, DataBuffer* data);
    bool formatSamples(char* startLocation, uint8_t& length, DataBuffer* data);
    bool formatNullBeforeSamples(char* startLocation, uint8_t& length, DataBuffer* data);
    bool formatDataSample(char* startLocation, uint8_t& length, DataBuffer* data);
    bool formatNullAfterSamples(char* startLocation, uint8_t& length, DataBuffer* data);
    bool formatFloat(char* startLocation, uint8_t& length, float val, uint8_t decimalPrecision);
    bool formatInt32(char* startLocation, uint8_t& length, uint32_t val);
    bool formatInt64(char* startLocation, uint8_t& length, uint64_t val);
    uint8_t countDigits20(uint64_t num);
    uint8_t countDigits10(uint32_t num);
    sd_card_t* sdCard;
    FIL fileOut;
    /*128 (12 positions for time in microseconds, 10 positions for aggregate sample count, 10 positions for individual sample count
    //8 positions per number, for 12 fields)
    //since its writing as a csv, these estimates for length of numbers (12 for time, 8 for float) are not going to be
    //strictly followed
    //*4 to get to 512 - sdcards typically have 512 byte sectors, writing in multiples of this is good
    //*16 to minimize constant switching between not writing and writing
    //8196 bytes (8kB) buffer size
    */
    char writeBuf[SDBUFSIZE];
    uint16_t latestUnwrittenIndex = 0;
};
#endif