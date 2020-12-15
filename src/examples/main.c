#include <discoid/circular_buffer.h>

int main(int argc, char *argv[])
{
    DiscoidBuffer buffer;

    discoidBufferInit(&buffer, 1024);
    discoidBufferWrite(&buffer, "hello", 6);
    uint8_t buf[10];

    int errorCode = discoidBufferRead(&buffer, buf, 6);
    if (errorCode < 0)
    {
        return errorCode;
    }

    printf("I read back '%s'\n", buf);

    return 0;
}