/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <discoid/circular_buffer.h>
#include <imprint/allocator.h>

/// Clear the buffer
/// The buffer is cleared, but the capacity remains
/// @param self
void discoidBufferReset(DiscoidBuffer* self)
{
    self->writeIndex = 0U;
    self->readIndex = 0U;
    self->size = 0U;
}

/// initialize circular DiscoidBuffer
/// @param self
/// @param allocator used to allocate memory for the buffer
/// @param maxSize the maximum capacity of the buffer
void discoidBufferInit(DiscoidBuffer* self, struct ImprintAllocator* allocator, size_t maxSize)
{
    self->buffer = IMPRINT_ALLOC_TYPE_COUNT(allocator, uint8_t, maxSize);
    self->capacity = maxSize;
    discoidBufferReset(self);
}

/// The number of octets available to read
/// @param self
/// @return octet count that can be read
size_t discoidBufferReadAvailable(const DiscoidBuffer* self)
{
    return self->size;
}

/// The maximum number of octets that can be written
/// @param self
/// @return octet count that can be written
size_t discoidBufferWriteAvailable(const DiscoidBuffer* self)
{
    return self->capacity - self->size;
}

/// Write octets to the circular buffer
/// @param self
/// @param data
/// @param dataOctetCount
/// @return negative on error
int discoidBufferWrite(DiscoidBuffer* self, const uint8_t* data, size_t dataOctetCount)
{
    const uint8_t* source = data;
    int sampleCount = dataOctetCount;

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

/// Skips octet count instead of reading
/// @param self
/// @param octetCount
/// @return negative on error
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

/// Peeks into the raw circular buffer
/// The data is not removed from the buffer and is still available on next discoidBufferRead().
/// Use it carefully since value of the octets that has not been written previously are undefined.
/// @param self
/// @param readIndex
/// @param target
/// @param readCount
/// @return the number of octets for the second run
int discoidBufferPeek(const DiscoidBuffer* self, size_t readIndex, uint8_t* target, size_t readCount)
{
    if (readIndex > self->capacity) {
        return -1;
    }
    size_t availableFirstRun = self->capacity - readIndex;
    size_t firstRun = readCount;
    size_t secondRun = 0U;

    if (readCount > availableFirstRun) {
        firstRun = availableFirstRun;
        secondRun = readCount - availableFirstRun;
    }
    // CLOG_INFO("peeking at readIndex %zu. First run:%d second:%d", readIndex, firstRun, secondRun);

    tc_memcpy_type(uint8_t, target, self->buffer + readIndex, firstRun);
    if (secondRun > 0U) {
        tc_memcpy_type(uint8_t, target + firstRun, self->buffer, secondRun);
    }

    return secondRun;
}

/// Read octets from the buffer
/// @param self
/// @param output
/// @param octetCountToRead
/// @return negative on error
int discoidBufferRead(DiscoidBuffer* self, uint8_t* output, size_t octetCountToRead)
{
    if (octetCountToRead > self->size) {
        return -2;
    }

    int secondRun = discoidBufferPeek(self, self->readIndex, output, octetCountToRead);
    self->size -= octetCountToRead;

    if (secondRun > 0) {
        self->readIndex = secondRun;
    } else {
        self->readIndex += octetCountToRead;
    }

    return 0;
}
