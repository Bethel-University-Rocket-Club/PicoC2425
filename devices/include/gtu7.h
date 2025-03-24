#ifndef GTU7_H
#define GTU7_H
#include "common.h"

class GTU7 {
    public:
    GTU7(uart_inst_t* uart);
    float getAltitude();

    private:
    uart_inst_t* uart;
    bool throwSentence;
    float recentAltitude;
    byte buffer[82]; //?
};
#endif