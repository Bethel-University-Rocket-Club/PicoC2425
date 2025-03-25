#ifndef COMMON_H
#define COMMON_H
typedef unsigned char byte;
struct FloatArray3 {
    float values[3];
};
struct FloatArray2 {
    float values[2];
};
uint16_t bytesToUShort(byte* data);
int16_t bytesToShort(byte* data);
#endif