#ifndef GTU7_H
#define GTU7_H
#include "common.h"

#ifndef GPSBUFCOUNT
#define GPSBUFCOUNT 2
#endif


class GTU7 {
    public:
    GTU7(uart_inst_t* uart);
    bool getAltitude(float& getAltitude);

    private:
    bool getSentence();
    bool parseAlt(uint8_t startPos, uint8_t bufNum);
    static GTU7* instance;
    static void dmaHandler() {
        if(instance) {
            instance->DMAGPSHandler();
        }
    }
    void DMAGPSHandler();

    uart_inst_t* uart;
    float recentAltitude;
    byte newAlt = 0; //represents recently completed buffer - 1. If val is 0, outdated info. if 1, buffer 0 was completed
    byte volatile curBuf = 0;
    uint32_t length = 0;
    byte dmaChanNum;
    bool throwSentence = false;
    char buffer[GPSBUFCOUNT][82] = {0}; //maximum sentence length is 82
    struct CommaInfo {
        byte commaNum = 0;
        byte commaPos = 0;
    };
    CommaInfo commaBuffer[GPSBUFCOUNT];
};
#endif