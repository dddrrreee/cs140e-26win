### Linux example of `--wrap` flag

Linux-specific example of how to use the GNU linker `ld` to interpose on
function calls by using the `--wrap` flag.

**Note:** AFAIK, MacOS does not support this flag.

**What this shows:** Same mechanism as `01-tracing-pi` but running as a
regular Linux program (uses `printf` instead of `printk`, `main` instead
of `notmain`).

**To run:**

```bash
% make
if --wrap worked: should see calls to both 'wrap_foo' and 'real_foo'
./trace-foo
about to call foo(1,2)
in __wrap_foo(1,2)
calling __real_foo(2,3)
in real foo: have arguments (2,3)
returning from wrap_foo
returned  5
```

**Why this is useful:** You can test your wrapping logic on Linux as well.
The mechanism is identical, just the platform differs.
