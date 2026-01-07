// simple routine to measure cost of multiplication.
// if you look at machine code: what happened to
// our measurement?
int mul(int *time, int a, int b) {
    // read usec timer
    unsigned start = *(volatile unsigned *) 0x20003004;

    volatile int c = a * b;

    // read usec timer
    unsigned end = *(volatile unsigned *) 0x20003004;

    // compute how many usec.
    *time = end - start;
    return c;
}
