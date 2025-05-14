#include "writer.h"

Writer::Writer() {
}

bool Writer::writeData(DataBuffer *db, bool core0) {
    //printf("%d %d\n", nextWriteEndIndex, nextWriteStartIndex);
    uint16_t startIndex = nextWriteEndIndex;
    byte *sNum = (byte *)&db->sensorNum;
    byte *d1 = (byte *)&db->data.values[0];
    byte *d2 = (byte *)&db->data.values[1];
    byte *d3 = (byte *)&db->data.values[2];
    byte *time = (byte *)&db->elapsedTime;
    writeBuf[nextWriteEndIndex++] = sNum[0];
    for(int i = 0; i < 4; i++) {
        writeBuf[nextWriteEndIndex++] = d1[i];
    }
    for(int i = 0; i < 4; i++) {
        writeBuf[nextWriteEndIndex++] = d2[i];
    }
    for(int i = 0; i < 4; i++) {
        writeBuf[nextWriteEndIndex++] = d3[i];
    }
    for(int i = 0; i < 8; i++) {
        writeBuf[nextWriteEndIndex++] = time[i];
    }
    if(startIndex > nextWriteEndIndex) {
        bool success = false;
        if(core0) {
            success = core0flush(nextWriteStartIndex, flushSize);
        } else {
            success = flush(nextWriteStartIndex, flushSize);
        }
        nextWriteStartIndex = 0;
        return success;
    } else if(nextWriteEndIndex - nextWriteStartIndex >= flushSize) {
        bool success = false;
        if(core0) {
            success = core0flush(nextWriteStartIndex, flushSize);
        } else {
            success = flush(nextWriteStartIndex, flushSize);
        }
        nextWriteStartIndex += 256;
        return success;
    }
    return true;
}

bool Writer::printData() {
    writeHeader();
    uint32_t beginning = STARTADDR & ~(FLASH_SECTOR_SIZE - 1);
    beginning += FLASH_SECTOR_SIZE;
    DataBuffer *transient = new DataBuffer();
    for(uint32_t curAddr = beginning; curAddr < ENDADDR; curAddr += DATABUFFERSTORESIZE) {
        if(curAddr+DATABUFFERSTORESIZE >= ENDADDR) {
            break;
        }
        uint8_t *binDataBuffer = (uint8_t *)(curAddr);
        bool end = true;
        for(int i = 0; i < DATABUFFERSTORESIZE; i++) {
            if(binDataBuffer[i] != 0xFF) {
                end = false;
            }
        }
        if(end) {
            break;
        }
        if(parseData(transient, &binDataBuffer[0])) {
            printDataBuffer(transient);
            sleep_ms(1);
        }

    }
    delete transient;
    return true;
}

bool Writer::writeDataTo(SDWriter *sdw)
{
    if(!sdw->checkConnection()) return false;
    sdw->writeHeader();
    uint32_t beginning = STARTADDR & ~(FLASH_SECTOR_SIZE - 1);
    beginning += FLASH_SECTOR_SIZE;
    DataBuffer *transient = new DataBuffer();
    for(uint32_t curAddr = beginning; curAddr < ENDADDR; curAddr += DATABUFFERSTORESIZE) {
        if(curAddr+DATABUFFERSTORESIZE >= ENDADDR) {
            break;
        }
        uint8_t *binDataBuffer = (uint8_t *)(curAddr);
        bool end = true;
        for(int i = 0; i < DATABUFFERSTORESIZE; i++) {
            if(binDataBuffer[i] != 0xFF) {
                end = false;
            }
        }
        if(end) {
            break;
        }
        if(parseData(transient, &binDataBuffer[0])) {
            sdw->writeData(transient);
            sleep_ms(1);
        }

    }
    sdw->flush();
    sdw->close();
    sdw->unmount();
    delete transient;
    return true;
}

bool Writer::checkConnection() {
    return true;
}

bool Writer::writeHeader() {
    printf("Time(us),AggregateSample,SensorSample,BMP280Alt(m),BMP280Vel(m/s),BMP280Acc(m/s/s),MPU6050Alt(m),MPU6050Vel(m/s),MPU6050Acc(m/s/s),GT-U7Alt(m),GT-U7Vel(m/s),GT-U7Acc(m/s/s),PitotAlt(m),PitotVel(m/s),PitotAcc(m/s/s)\n");
    fflush(stdout);
    return true;
}

bool Writer::close(bool core0) {
    static byte closeIndicator[256] = {0xFF};
    if(nextWriteSector >= ENDADDR) return false;
    uint32_t interrupts = save_and_disable_interrupts();
    if(!core0) multicore_lockout_start_blocking();
    flash_range_program(nextWriteSector - XIP_BASE, closeIndicator, 256);
    if(!core0) multicore_lockout_end_blocking();
    restore_interrupts(interrupts);
    nextWriteSector += 256;
    return true;
}

bool Writer::open(const char *name) {
    return false;
}

bool Writer::flush() {
    uint32_t writeSize = nextWriteEndIndex - nextWriteStartIndex;
    if(nextWriteSector >= ENDADDR) return false;
    if(writeSize % 256 != 0) {
        uint16_t toAdd = 256 - writeSize % 256;
        for(int i = 0; i < toAdd; i++) {
            writeBuf[nextWriteEndIndex + i] = 0xFF;
        }
        writeSize += toAdd;
    }
    return core0flush(nextWriteStartIndex, writeSize);
}

//expects size to be divisible by 256
bool Writer::flush(uint16_t start, uint32_t size) {
    if (nextWriteSector >= ENDADDR) {
        return false;
    }
    uint32_t interrupts = save_and_disable_interrupts();
    multicore_lockout_start_blocking();
    flash_range_program(nextWriteSector - XIP_BASE, &writeBuf[start], size);
    multicore_lockout_end_blocking();
    restore_interrupts(interrupts);
    nextWriteSector += size;
    return true;
}

bool Writer::core0flush(uint16_t start, uint32_t size)
{
    if (nextWriteSector >= ENDADDR) {
        return false;
    }
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program(nextWriteSector - XIP_BASE, &writeBuf[start], size);
    restore_interrupts(interrupts);
    nextWriteSector += size;
    return true;
}

bool Writer::unmount() {
    return true;
}

bool Writer::clearFlash() {
    nextWriteSector = STARTADDR & ~(FLASH_SECTOR_SIZE - 1);
    nextWriteSector += FLASH_SECTOR_SIZE;
    uint32_t endSectorAddress = ENDADDR & ~(FLASH_SECTOR_SIZE - 1);
    uint32_t interrupts = save_and_disable_interrupts();
    for (uint32_t addr = nextWriteSector; addr < endSectorAddress; addr += FLASH_SECTOR_SIZE) {
        flash_range_erase(addr - XIP_BASE, FLASH_SECTOR_SIZE);
    }
    restore_interrupts(interrupts);
    return true;
}

bool Writer::parseData(DataBuffer *db, byte *buffer) {
    byte sensorNum = *(byte *)&buffer[0];
    uint32_t ud1 = 0;
    for (int i = 0; i < 4; i++) {
        ud1 |= (uint32_t)buffer[1 + i] << (i * 8); // Little-endian
    }
    uint32_t ud2 = 0;
    for (int i = 0; i < 4; i++) {
        ud2 |= (uint32_t)buffer[5 + i] << (i * 8); // Little-endian
    }
    uint32_t ud3 = 0;
    for (int i = 0; i < 4; i++) {
        ud3 |= (uint32_t)buffer[9 + i] << (i * 8); // Little-endian
    }
    float d1 = *(float *)&ud1;
    float d2 = *(float *)&ud2;
    float d3 = *(float *)&ud3;
    uint64_t microTime = 0;
    for (int i = 0; i < 8; i++) {
        microTime |= (uint64_t)buffer[13 + i] << (i * 8); // Little-endian
    }
    if(microTime >= 0xFFFFFFFF00000000) {
        return false;
    }
    db->sensorSample = 0;
    db->data.values[0] = d1;
    db->data.values[1] = d2;
    db->data.values[2] = d3;
    db->elapsedTime = microTime;
    db->sensorNum = sensorNum;
    db->sensorMax = 3;
    switch(sensorNum) {
        case 0:
        BMPcount++;
        aggCount++;
        db->sensorSampleNum = BMPcount;
        db->sigDecimalDigits = 2;
        break;
        case 1:
        ADXcount++;
        aggCount++;
        db->sensorSampleNum = ADXcount;
        db->sigDecimalDigits = 3;
        break;
        case 2:
        GTUcount++;
        aggCount++;
        db->sensorSampleNum = GTUcount;
        db->sigDecimalDigits = 2;
        break;
        case 3:
        MPXcount++;
        aggCount++;
        db->sensorSampleNum = MPXcount;
        db->sigDecimalDigits = 2;
        break;
    }
    db->aggSampleNum = aggCount;
    return true;
}

bool Writer::printDataBuffer(DataBuffer *db) {
    switch(db->sensorNum) {
        case 0:
        printf("%llu,%u,%u,%f,%f,%f,,,,,,,,,\n", db->elapsedTime, db->aggSampleNum, db->sensorSampleNum, 
            db->data.values[0], db->data.values[1], db->data.values[2]); fflush(stdout);
        break;
        case 1:
        printf("%llu,%u,%u,,,,%f,%f,%f,,,,,,\n", db->elapsedTime, db->aggSampleNum, db->sensorSampleNum, 
            db->data.values[0], db->data.values[1], db->data.values[2]); fflush(stdout);
        break;
        case 2:
        printf("%llu,%u,%u,,,,,,,%f,%f,%f,,,\n", db->elapsedTime, db->aggSampleNum, db->sensorSampleNum, 
            db->data.values[0], db->data.values[1], db->data.values[2]); fflush(stdout);
        break;
        case 3:
        printf("%llu,%u,%u,,,,,,,,,,%f,%f,%f\n", db->elapsedTime, db->aggSampleNum, db->sensorSampleNum, 
            db->data.values[0], db->data.values[1], db->data.values[2]); fflush(stdout);
        break;
    }
    return true;
}

void Writer::dumpFlash(uint32_t startAddr, uint32_t length) {
    printf("Dumping flash from 0x%08x for %u bytes:\n", startAddr, length);
    volatile uint8_t *flashData = (volatile uint8_t *)startAddr;
    for (uint32_t i = 0; i < length; i++) {
        if (i % 16 == 0) printf("\n0x%08x: ", startAddr + i);
        printf("%02x ", flashData[i]);
    }
    printf("\n");
    fflush(stdout);
}