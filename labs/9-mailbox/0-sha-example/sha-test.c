// take this simple test, make a copy of the timer code.
//  1. compute sha before interrupts on.
//  2. turn interrupts on.
//  3. recompute sha 1000x.
//  4. combines its input so aggressively: very hard to make any
//     mistake and have it pass.  
#include "rpi.h"
#include "sha256-fixed.h"

static void sha256_print(const char *msg, sha256_t sha) {
    output("%s: sha= {\n", msg);
    for(int i = 0; i < 8; i++) 
        output("\t[%d]=%x\n", i, sha.digest[i]);
    output("}\n");
}

// we panic to catch as soon as possible: you could just return
// 0.
static int sha256_cmp(sha256_t sha1, sha256_t sha2, int die_p) {
    for(int i = 0; i < 8; i++)  {
        if(sha1.digest[i] != sha2.digest[i]) {
            if(die_p) 
                panic("sha1.digest[%d] = %x, sha2.digest[%d] = %x\n", 
                    i, sha1.digest[i], i, sha2.digest[i]);
            return 0;
        }
    }
    return 1;
}

static void sha_test(int x, int verbose_p) {
    uint32_t input[16];

    // should set to something more interesting.
    for(int i = 0; i < 8; i++)
        input[i] = i*x;

    let sha_1 = sha256_one_block(input);
    let sha_2 = sha256_one_block(input);

    if(verbose_p) 
        sha256_print("one block", sha_1);

    if(!sha256_cmp(sha_1, sha_2,0))
        panic("sha mismatch\n");

    if(verbose_p)
        output("single sha matched!\n");

}

void notmain(void) {
    enum { N = 100 };
    for(int i = 0; i < N; i++)
        sha_test(i,0);
    output("SUCCESS: %d sha's matched\n", N);
}
