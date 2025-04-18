#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H
#include "common.h"
#include <atomic>

class CircularQueue {
    public:
    CircularQueue();
    DataBuffer* startEnqueue();
    void finishEnqueue();
    DataBuffer* dequeue();
    void finishDequeue();
    bool isEmpty();
    bool isFull();
    uint16_t size();

    private:
    DataBuffer queue[256] = {};
    std::atomic<uint8_t> head = 0;
    std::atomic<uint8_t> tail = 0;
    std::atomic<uint16_t> count = 0;

};

#endif