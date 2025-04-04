#ifndef COMMON_H
#define COMMON_H
typedef unsigned char byte;
struct FloatArray3 {
    float values[3];
};
struct FloatArray2 {
    float values[2];
};
struct DataBuffer {
    uint64_t elapsedTime;
    uint32_t aggSampleNum;
    uint32_t sensorSampleNum;
    byte sensorNum;
    byte sensorMax; //max value for sensorNum
    FloatArray3 data;
};
uint16_t bytesToUShort(byte* data);
int16_t bytesToShort(byte* data);
#endif