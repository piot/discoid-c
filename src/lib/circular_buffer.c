#include <discoid/circular_buffer.h>
#include <tiny-libc/tiny_libc.h>
#include <clog/clog.h>

void discoidBufferInit(DiscoidBuffer *self, size_t maxSize)
{
	self->buffer = tc_malloc_type_count(uint8_t, maxSize);
	self->writeIndex = 0;
	self->readIndex = 0;
	self->capacity = maxSize;
	self->size = 0;
}

void discoidBufferDestroy(DiscoidBuffer *self)
{
	tc_free(self->buffer);
}

int discoidBufferReadAvailable(const DiscoidBuffer *self)
{
	return self->size;
}

int discoidBufferWrite(DiscoidBuffer *self, const uint8_t *data, size_t sampleCountInTarget)
{
	const uint8_t *source = data;
	int sampleCount = sampleCountInTarget;

	int availableWriteCount = self->capacity - self->size;
	if (sampleCount > availableWriteCount)
	{
		return -2;
	}

	if (sampleCount == 0)
	{
		return 0;
	}

	int firstAvailable = self->capacity - self->writeIndex;
	int firstRun = sampleCount;
	if (firstRun > firstAvailable)
	{
		firstRun = firstAvailable;
	}
	tc_memcpy_type(uint8_t, self->buffer + self->writeIndex, source, firstRun);

	sampleCount -= firstRun;
	source += firstRun;
	self->size += firstRun;
	self->writeIndex += firstRun;
	self->writeIndex %= self->capacity;
	if (sampleCount == 0)
	{
		return 0;
	}

	tc_memcpy_type(uint8_t, self->buffer + self->writeIndex, source, sampleCount);
	self->writeIndex += sampleCount;
	self->writeIndex %= self->capacity;
	self->size += sampleCount;

	return 0;
}

int discoidBufferSkip(DiscoidBuffer *self, size_t octetCount)
{
	if (octetCount > self->size)
	{
		return -1;
	}
	self->readIndex += octetCount;
	self->readIndex %= self->capacity;
	self->size -= octetCount;

	return 0;
}

int discoidBufferPeek(const DiscoidBuffer *self, size_t readIndex, uint8_t *target, size_t readCount)
{
	size_t availableFirstRun = self->capacity - readIndex;
	size_t firstRun = readCount;
	size_t secondRun = 0;

	if (readCount > availableFirstRun)
	{
		firstRun = availableFirstRun;
		secondRun = readCount - availableFirstRun;
	}
	//CLOG_INFO("peeking at readIndex %zu. First run:%d second:%d", readIndex, firstRun, secondRun);

	tc_memcpy_type(uint8_t, target, self->buffer + readIndex, firstRun);
	if (secondRun > 0)
	{
		tc_memcpy_type(uint8_t, target + firstRun, self->buffer, secondRun);
	}
	return secondRun;
}

int discoidBufferRead(DiscoidBuffer *self, uint8_t *output, size_t requiredCount)
{
	if (requiredCount > self->size)
	{
		return -2;
	}

	int secondRun = discoidBufferPeek(self, self->readIndex, output, requiredCount);

	self->size -= requiredCount;
	if (secondRun > 0)
	{
		self->readIndex = secondRun;
	}
	else
	{
		self->readIndex += requiredCount;
	}

	return 0;
}
