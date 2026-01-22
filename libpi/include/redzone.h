#ifndef __REDZONE_H__
#define __REDZONE_H__

// simple-stupid way to check memory corruption in the
// address range [0..4096)
//
// 1. set [0..4096) to 0.
// 2. check that its still 0 when called.
// 
// if this ever fails you are probably writing to null.
enum { RZ_NBYTES = 4096, RZ_SENTINAL = 0 };

static inline int 
redzone_check_fn(const char *file, 
    const char *fn, 
    int lineno, 
    const char *msg) 
{
    volatile uint32_t *rz = (volatile void*)0;
    if(msg)
        output("%s:%s:%d: <%s>\n", file,fn,lineno,msg);

    unsigned nfail = 0;
    for(unsigned i = 0; i < RZ_NBYTES/4; i++) {
        if(rz[i] == RZ_SENTINAL)
            continue;

        // don't spam error messages
        if(nfail++ < 10) {
            output("%s:%s:%d: ERROR: non-zero redzone!: off=%x, val=%x\n", 
                    file,fn,lineno,
                    i*4,
                    rz[i], 
                    RZ_SENTINAL);
        }
    }

    // die on failure: should fix so you get the location.
    if(nfail) {
        output("%s:%s:%d: redzone: total failures =%d\n", file,fn,lineno,nfail);
        // should probably have this be optional.
        clean_reboot();
    }

    return nfail == 0;
}

#define redzone_check(msg) \
        redzone_check_fn(__FILE__, __FUNCTION__, __LINE__, msg)

static inline void redzone_init(void) {
    volatile uint8_t *ptr =(void*)0;

    for(int i = 0; i < RZ_NBYTES; i++)
        ptr[i] = RZ_SENTINAL;

    // make sure it passes
    assert(redzone_check(0));
}
#endif
