#include "writer.h"

Writer::Writer() {
}

bool Writer::writeData(DataBuffer *db) {
    return false;
}

bool Writer::printData() {
    return false;
}

bool Writer::checkConnection() {
    return true;
}

bool Writer::writeHeader() {
    return false;
}

bool Writer::close() {
    return false;
}

bool Writer::open(const char *name) {
    return false;
}

bool Writer::flush() {
    return false;
}

bool Writer::unmount() {
    return false;
}

bool Writer::clearFlash() {
    return false;
}

bool Writer::formatData(DataBuffer *db) {
    return false;
}
