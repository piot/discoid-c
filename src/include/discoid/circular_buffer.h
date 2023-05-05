/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef DISCOID_BUFFER_H
#define DISCOID_BUFFER_H

#include <stdint.h>
#include <stdlib.h>

struct ImprintAllocator;

typedef struct DiscoidBuffer {
    size_t capacity;
    uint8_t* buffer;
    size_t writeIndex;
    size_t readIndex;
    size_t size;
} DiscoidBuffer;

void discoidBufferInit(DiscoidBuffer* self, struct ImprintAllocator* allocator, size_t maxSize);
void discoidBufferReset(DiscoidBuffer* self);
size_t discoidBufferReadAvailable(const DiscoidBuffer* self);
size_t discoidBufferWriteAvailable(const DiscoidBuffer* self);
int discoidBufferSkip(DiscoidBuffer* self, size_t octetCount);
int discoidBufferWrite(DiscoidBuffer* self, const uint8_t* data, size_t sampleCountInTarget);
int discoidBufferRead(DiscoidBuffer* self, uint8_t* output, size_t requiredCount);
int discoidBufferPeek(const DiscoidBuffer* self, size_t readIndex, uint8_t* target, size_t readCount);

#endif
