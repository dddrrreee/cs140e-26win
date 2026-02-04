/*
 * search for <todo> and implement: 
 *  - vector_base_set  
 *  - vector_base_get
 *  - vector_base_reset
 * 
 * these all use vector base address register:
 *   arm1176.pdf:3-121 --- lets us control where the 
 *   exception jump table is!  makes it easy to switch
 *   tables and also make exceptions faster.
 *
 * once this works, move it to: 
 *   <libpi/include/vector-base.h>
 * and make sure it still works.
 */

#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "libc/bit-support.h"

// use inline assembly to get and return the vector base's 
// current value.
static inline void *vector_base_get(void) {
    todo("implement using inline assembly to get the vec base reg");
}

// set vector base register: use inline assembly.  there's only
// one caller so you can also get rid of this if you want.  we
// use to illustrate a common pattern.
static inline void vector_base_set_raw(uint32_t v) {
    // make sure you use prefetch flush!
    todo("implement using inline assembly to set the vec base reg");
}

// check that not null and alignment is good.
static inline int vector_base_chk(void *vector_base) {
    if(!vector_base)
        return 0;

    // IMPORTANT: most common mistake is to not check alignment
    // correctly. very easy way to get intermittent but violent
    // bugs.
    todo("check alignment is correct: look at the instruction def!");
    return 1;
}

// set vector base to <vec> and return old value: could have
// been previously set (i.e., vector_base_get returns non-null).
static inline void *
vector_base_reset(void *vec) {
    void *old_vec = 0;

    if(!vector_base_chk(vec))
        panic("illegal vector base %p\n", vec);

    todo("get old vector base, set new one\n");

    // double check that what we set is what we have.
    // 
    // NOTE: common safety net pattern: read back what 
    // you wrote to make sure it is indeed what got set.
    // catches *many* bugs in this class.  (in this case:
    // alignment issues.)
    assert(vector_base_get() == vec);
    return old_vec;
}

// set vector base: must not have been set already.
// if you want to forcibly overwrite the previous
// value use <vector_base_reset>
static inline void vector_base_set(void *vec) {
    // if already set to the same vector, just return.
    void *v = vector_base_get();
    if(v == vec)
        return;
    if(v) 
        panic("vector base register already set=%p\n", v);

    // this is not on the critical path do just call reset.
    vector_base_reset(vec);
}
#endif
