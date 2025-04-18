#include "circularQueue.h"
#include <stdio.h>

CircularQueue::CircularQueue() {
}

DataBuffer *CircularQueue::startEnqueue() {
    if(isFull()) {
        return nullptr;
    }
    return &queue[tail];
}

void CircularQueue::finishEnqueue() {
    uint8_t nextTail = tail.load(std::memory_order_relaxed) + 1;
    tail.store(nextTail, std::memory_order_release);
    count.fetch_add(1, std::memory_order_acq_rel);
    return;
}

DataBuffer *CircularQueue::dequeue() {
    if(isEmpty()) {
        return nullptr;
    }
    return &queue[head];
}

void CircularQueue::finishDequeue() {
    uint8_t nextHead = head.load(std::memory_order_relaxed) + 1;
    head.store(nextHead, std::memory_order_release);
    count.fetch_sub(1, std::memory_order_acq_rel);
    return;
}

bool CircularQueue::isEmpty() {
    return count.load(std::memory_order_acquire) == 0;
}

bool CircularQueue::isFull() {
    return count.load(std::memory_order_acquire) == 256;
}

uint16_t CircularQueue::size() {
    return count.load(std::memory_order_acquire);
}