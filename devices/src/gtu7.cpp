#include "gtu7.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

GTU7* GTU7::instance = nullptr;

GTU7::GTU7(uart_inst_t *uart) {
    this->uart = uart;
    instance = this;
    dmaChanNum = dma_claim_unused_channel(false);
    dma_channel_config cfg = dma_channel_get_default_config(dmaChanNum);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, false); // UART RX FIFO
    channel_config_set_write_increment(&cfg, true); // Buffer
    channel_config_set_dreq(&cfg, DREQ_UART0_RX);

    dma_channel_configure(
        dmaChanNum,
        &cfg,
        &buffer[curBuf][0], // Write to buffer
        &uart0_hw->dr, // Read from UART1 RX
        1, // 1 byte
        false // Don’t start yet
    );
    dma_channel_set_irq1_enabled(dmaChanNum, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dmaHandler);
    irq_set_enabled(DMA_IRQ_1, true);

    dma_channel_start(dmaChanNum);
}

bool GTU7::getAltitude(float& getAltitude) {
    bool isNew = newAlt;
    newAlt = false;
    getAltitude = recentAltitude;
    return isNew;
}

void GTU7::DMAGPSHandler() {
    dma_hw->ints0 = 1u << dmaChanNum;
    if(buffer[curBuf][length] == '\n') {
        if(!throwSentence) {
            bool success = parseAlt(commaBuffer[curBuf].commaPos, curBuf);
            commaBuffer[curBuf].commaPos = 0;
            commaBuffer[curBuf].commaNum = 0;
            if(success) {
                curBuf++;
                curBuf %= GPSBUFCOUNT;
            }
        }
        throwSentence = false;
        length = 0;
        dma_channel_set_write_addr(dmaChanNum, &buffer[curBuf][length], false);
        dma_channel_set_trans_count(dmaChanNum, 1, false);
        dma_channel_start(dmaChanNum);
        return;    
    }
    if(buffer[curBuf][length] == ',') {
        if(commaBuffer[curBuf].commaNum != 9) { //the comma at the start of the altitude section
            commaBuffer[curBuf].commaNum += 1;
            commaBuffer[curBuf].commaPos = length;
        }
    }
    length++;
    if(length < 82) {
        dma_channel_set_write_addr(dmaChanNum, &buffer[curBuf][length], false);
        dma_channel_set_trans_count(dmaChanNum, 1, false);
        dma_channel_start(dmaChanNum);
    } else {
        throwSentence = true;
        length = 0;
        dma_channel_set_write_addr(dmaChanNum, &buffer[curBuf][length], false);
        dma_channel_set_trans_count(dmaChanNum, 1, false);
        dma_channel_start(dmaChanNum);
    }
    return;
}

bool GTU7::parseAlt(uint8_t startPos, uint8_t bufNum) {
    static const float pow10[6] = {1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f};
    char curChar = buffer[bufNum][++startPos];
    uint32_t big = 0;
    uint32_t small = 0;
    uint32_t bigDecCount = 0; //left of decimal
    uint32_t smallDecCount = 0; //right of decimal
    bool decimalSeen = false;
    while(curChar == '.' || (curChar <= '9' && curChar >= '0')) {
        if(curChar == '.') {
            decimalSeen = true;
            curChar = buffer[bufNum][++startPos];
            continue;
        }
        if(!decimalSeen) {
            big *= 10;
            big += curChar - '0';
            bigDecCount += 1;
        } else {
            small *= 10;
            small += curChar - '0';
            smallDecCount += 1;
        }
        curChar = buffer[bufNum][++startPos];
    }
    if((bigDecCount + smallDecCount) == 0 || curChar != ',') { //curChar should always be ',' if not, there's a problem. Throw.
        return false;
    }
    recentAltitude = (float)big + (float)small * pow10[smallDecCount];
    newAlt = bufNum + 1;
    return true;
}