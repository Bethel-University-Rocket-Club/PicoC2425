#ifndef WRITER_H
#define WRITER_H
#include "common.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include "sdwriter.h"

#define STARTADDR (XIP_BASE + 320*1024) //overshooting program size
#define ENDADDR (XIP_BASE + PICO_FLASH_SIZE_BYTES)
#define DATABUFFERSTORESIZE 21

class Writer {
    public:
        Writer();
        bool writeData(DataBuffer* db, bool core0);
        bool printData();
        bool writeDataTo(SDWriter* sdw);
        bool checkConnection();
        bool writeHeader();
        bool close(bool core0);
        bool open(const char* name);
        bool flush();
        bool unmount();
        bool clearFlash();
        void dumpFlash(uint32_t startAddr, uint32_t length);
    private:
        bool flush(uint16_t start, uint32_t size);
        bool core0flush(uint16_t start, uint32_t size);
        bool parseData(DataBuffer* db, byte* buffer);
        bool printDataBuffer(DataBuffer* db);

        uint32_t nextWriteSector;
        byte writeBuf[65536];
        byte parseBuf[DATABUFFERSTORESIZE];
        uint16_t flushSize = 256;
        uint16_t nextWriteStartIndex = 0;
        uint16_t nextWriteEndIndex = 0;
        uint16_t BMPcount = 0;
        uint16_t ADXcount = 0;
        uint16_t GTUcount = 0;
        uint16_t MPXcount = 0;
        uint32_t aggCount = 0;
};

#endif