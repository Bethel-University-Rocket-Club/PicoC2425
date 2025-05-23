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
    uint16_t nextTail = tail.load(std::memory_order_relaxed) + 1;
    if((nextTail >> 11) > 0) nextTail = 0;
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
    uint16_t nextHead = head.load(std::memory_order_relaxed) + 1;
    if((nextHead >> 11) > 0) nextHead = 0;
    head.store(nextHead, std::memory_order_release);
    count.fetch_sub(1, std::memory_order_acq_rel);
    return;
}

bool CircularQueue::isEmpty() {
    return count.load(std::memory_order_acquire) == 0;
}

bool CircularQueue::isFull() {
    return count.load(std::memory_order_acquire) == 2048;
}

uint16_t CircularQueue::size() {
    return count.load(std::memory_order_acquire);
}