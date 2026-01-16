// Trivial program to show how `ld` function wrapping works on Linux.
#include <stdio.h>
#include "foo.h"

// Flag to verify that wrapping actually happened
static int wrapped_foo_p;

// Gross: `ld` creates these symbols but we have to declare them by hand.
// Be very careful! You could do some macro tricks, but we keep it simple.
//
// `__wrap_foo`: our wrapper function (defined below)
// `__real_foo`: the original `foo` function (in foo.c), renamed by `ld`
unsigned __wrap_foo(unsigned arg0, unsigned arg1);
unsigned __real_foo(unsigned arg0, unsigned arg1);

// Wrapper inserted by `ld` (see `Makefile`)
// This intercepts all calls to `foo` in the program.
unsigned __wrap_foo(unsigned arg0, unsigned arg1) {
    printf("in __wrap_foo(%u,%u)\n", arg0, arg1);

    // Show that we can mess with the arguments before forwarding them.
    arg0++;
    arg1++;
    printf("calling __real_foo(%u,%u)\n", arg0, arg1);
    unsigned ret = __real_foo(arg0, arg1);
    printf("returning from wrap_foo\n");

    // Set flag to verify wrapping worked
    wrapped_foo_p = 1;
    return ret;
}

int main(void) {
    unsigned a0 = 1, a1 = 2;
    printf("about to call foo(%d,%d)\n", a0, a1);
    unsigned ret = foo(a0, a1);
    printf("returned  %u\n", ret);

    // Sanity check: make sure wrapping actually happened
    if(!wrapped_foo_p) {
        printf("did not wrap foo??\n");
        return 1;
    }
    return 0;
}
