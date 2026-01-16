// trivial program to show how ld function wrapping works.
#include "rpi.h"
#include "foo.h"

// Gross: <ld> creates these symbols but we have to declare them by hand.
// Be very careful!  You could do some macro tricks, but we keep it simple.
//
// <__wrap_foo>: our wrapper function (defined below)
// <__real_foo>: the original <foo> function (in foo.c), renamed by <ld>
unsigned __wrap_foo(unsigned arg0, unsigned arg1);
unsigned __real_foo(unsigned arg0, unsigned arg1);

// Wrapper inserted by <ld> (see <Makefile>)
// This intercepts all calls to <foo> in the program.
unsigned __wrap_foo(unsigned arg0, unsigned arg1) {
    printk("in wrap foo(%u,%u)\n", arg0, arg1);

    // Show that we can mess with the arguments before forwarding them.
    arg0++;
    arg1++;
    printk("calling real foo(%u,%u)\n", arg0, arg1);
    unsigned ret = __real_foo(arg0, arg1);
    printk("returning from wrap_foo\n");
    return ret;
}

void notmain(void) {
    unsigned a0 = 1, a1 = 2;
    printk("about to call foo(%d,%d)\n", a0, a1);
    unsigned ret = foo(a0, a1);
    printk("returned  %u\n", ret);
    clean_reboot();
}
