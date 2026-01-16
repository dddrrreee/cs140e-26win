#include "rpi.h"
#include "trace.h"

void __wrap_notmain(void);
void __real_notmain(void);

void __wrap_notmain(void) {
    // trace_start(1);
    __real_notmain();
    // trace_stop();
    // implement this function!
}
