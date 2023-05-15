/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <discoid/circular_buffer.h>
#include <imprint/default_setup.h>
#include <clog/clog.h>
#include <clog/console.h>

clog_config g_clog;

int main(int argc, char* argv[])
{
    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_DEBUG;

    DiscoidBuffer buffer;

    ImprintDefaultSetup imprintDefault;

    imprintDefaultSetupInit(&imprintDefault, 2 * 1024);

    discoidBufferInit(&buffer, &imprintDefault.tagAllocator.info, 1024);
    discoidBufferWrite(&buffer, "hello", 6);
    uint8_t buf[10];

    int errorCode = discoidBufferRead(&buffer, buf, 6);
    if (errorCode < 0) {
        return errorCode;
    }

    printf("I read back '%s'\n", buf);

    return 0;
}