/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <discoid/circular_buffer.h>
#include <imprint/allocator.h>

void discoidBufferReset(DiscoidBuffer* self)
{
    self->writeIndex = 0;
    self->readIndex = 0;
    self->size = 0;
}

void discoidBufferInit(DiscoidBuffer* self, struct ImprintAllocator* allocator, size_t maxSize)
{
    self->buffer = IMPRINT_ALLOC_TYPE_COUNT(allocator, uint8_t, maxSize);
    self->capacity = maxSize;
    discoidBufferReset(self);
}

size_t discoidBufferReadAvailable(const DiscoidBuffer* self)
{
    return self->size;
}

size_t discoidBufferWriteAvailable(const DiscoidBuffer* self)
{
    return self->capacity - self->size;
}

int discoidBufferWrite(DiscoidBuffer* self, const uint8_t* data, size_t sampleCountInTarget)
{
    const uint8_t* source = data;
    int sampleCount = sampleCountInTarget;

    int availableWriteCount = self->capacity - self->size;
    if (sampleCount > availableWriteCount) {
        CLOG_ERROR("discoid buffer. Out of capacity. wanted to write %d but we are at (%zu / %zu)", sampleCount,
                  self->size, self->capacity)
        return -2;
    }
    if (sampleCount == 0) {
        return 0;
    }
    int firstAvailable = self->capacity - self->writeIndex;
    int firstRun = sampleCount;
    if (firstRun > firstAvailable) {
        firstRun = firstAvailable;
    }
    tc_memcpy_type(uint8_t, self->buffer + self->writeIndex, source, firstRun);
    sampleCount -= firstRun;
    source += firstRun;
    self->size += firstRun;
    self->writeIndex += firstRun;
    self->writeIndex %= self->capacity;
    if (sampleCount == 0) {
        return 0;
    }
    tc_memcpy_type(uint8_t, self->buffer + self->writeIndex, source, sampleCount);
    self->writeIndex += sampleCount;
    self->writeIndex %= self->capacity;
    self->size += sampleCount;
    return 0;
}

int discoidBufferSkip(DiscoidBuffer* self, size_t octetCount)
{
    if (octetCount > self->size) {
        CLOG_SOFT_ERROR("you tried to discard more than is in the discoid buffer. %zu vs %zu", octetCount, self->size)
        return -1;
    }

    self->readIndex += octetCount;
    self->readIndex %= self->capacity;
    self->size -= octetCount;

    return 0;
}

int discoidBufferPeek(const DiscoidBuffer* self, size_t readIndex, uint8_t* target, size_t readCount)
{
    size_t availableFirstRun = self->capacity - readIndex;
    size_t firstRun = readCount;
    size_t secondRun = 0;
    if (readCount > availableFirstRun) {
        firstRun = availableFirstRun;
        secondRun = readCount - availableFirstRun;
    }
    // CLOG_INFO("peeking at readIndex %zu. First run:%d second:%d", readIndex, firstRun, secondRun);

    tc_memcpy_type(uint8_t, target, self->buffer + readIndex, firstRun);
    if (secondRun > 0) {
        tc_memcpy_type(uint8_t, target + firstRun, self->buffer, secondRun);
    }
    return secondRun;
}

int discoidBufferRead(DiscoidBuffer* self, uint8_t* output, size_t requiredCount)
{
    if (requiredCount > self->size) {
        return -2;
    }
    int secondRun = discoidBufferPeek(self, self->readIndex, output, requiredCount);
    self->size -= requiredCount;
    if (secondRun > 0) {
        self->readIndex = secondRun;
    } else {
        self->readIndex += requiredCount;
    }
    return 0;
}
