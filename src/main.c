#include "../include/log.h"

#include <stdio.h>

int main(void) {
    printf("Hello, world!\n");

    PLG_INFO("This is an INFO log.");
    PLG_WARN("This is a WARN log.");
    PLG_ERROR("This is an ERROR log.");
    PLG_FATAL("This is a FATAL log.");

    return 0;
}
