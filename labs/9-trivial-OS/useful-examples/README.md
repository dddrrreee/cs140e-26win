### Useful examples

This directory has a variety of small example programs demonstrating how to
use Unix operations to accomplish  goals related to this lab and others.

You should go through main ones listed in the `Makefile` and see what it
does and how it does it.  I guarantee not skipping through this part will
save you a lot of time later!  Often they are pretty straight-forward.
A few are a bit opaque, somewhat intentionally, so you have to reason
about what is going on.  There are various bits in the examples that
will be useful to steal for the next lab and some others.

There's a reasonable chance some of these tricks serve you for the next
couple of decades, at least here and there.


You should port the following code over the Unix since we will be writing
an OS that implements these routines in lab:
```c
int fib(int l) { return (l < 2) ? 1 : fib(l-1)+fib(l-2); }

int fork_fib(int l) {
    if(l < 2)
        return 1;

    int expected = fib(l);

    int pid1 = sys_fork();
    if(!pid1)
        sys_exit(fork_fib(l-1));

    int pid2 = sys_fork();
    if(!pid2)
        sys_exit(fork_fib(l-2));

    int res1 = sys_waitpid(pid1);
    int res2 = sys_waitpid(pid2);

    int res = res1+res2;
    if(res != expected)
        panic("got: %d+%d = %d (expected=%d)\n", res1,res2,res,expected);
    return res;
}

void notmain(void) {
    int n = 7;

    int exp = fib(n);
    int got = fork_fib(n);

    output("got=%d, expected=%d\n", got, exp);
    assert(got == exp);
    // output("SUCCESS: sum=%d\n", sum);
    sys_exit(0);
}
```
