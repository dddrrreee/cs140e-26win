// We need to keep this in another file or gcc inlines / removes it.
#include <stdio.h>
#include "foo.h"

// Simple function that adds two numbers.
// This is the "real" `foo` that will be renamed to `__real_foo` by `ld`
// when we use `--wrap=foo`.
unsigned foo(unsigned arg0, unsigned arg1) {
    printf("in real foo: have arguments (%d,%d)\n", arg0, arg1);
    return arg0 + arg1;
}
