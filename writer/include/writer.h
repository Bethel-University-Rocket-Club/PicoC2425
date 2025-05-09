#ifndef WRITER_H
#define WRITER_H
#include "common.h"
#include "hardware/flash.h"

class Writer {
    public:
        Writer();
        bool writeData(DataBuffer* db);
        bool printData();
        bool checkConnection();
        bool writeHeader();
        bool close();
        bool open(const char* name);
        bool flush();
        bool unmount();
    private:
        bool clearFlash();
        bool formatData(DataBuffer* db);
};

#endif