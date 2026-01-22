#ifndef __TEST_INIT_H__
#define __TEST_INIT_H__

#include "rpi.h"
#include "rpi-thread.h"
#include "redzone.h"

static void inline test_init(void) {
    unsigned oneMB = 1024*1024;
    kmalloc_init_set_start((void*)oneMB, oneMB);

    redzone_init();
    // this first one should pass.
    redzone_check("initialized redzone");
}
#endif
