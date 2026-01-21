#ifndef __SHA256_FIXED_H__
#define __SHA256_FIXED_H__
// two specialized SHA routines to handle one-chunk and two chunks.

#if 0
// any other headers have to be included before us (try to make it easier to compile
// for riscv symex).
#include <endian.h>
_Static_assert(__BYTE_ORDER == __LITTLE_ENDIAN, "need to fix the code");
#endif

// Result of a sha run: 8 32-bit words. 
//  We make it a structure so we can return it.
typedef struct sha256 {
    uint32_t digest[8];
} sha256_t;

// Initialize array of round constants:
// (first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311):
static const uint32_t k[64] = {
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
_Static_assert(sizeof k == 64*4, "Wrong size for k");

// from the sha specification (see ../docs)
static inline uint32_t rrot32(uint32_t x, unsigned n) {
    return (x >> n) | (x << (32 - n));
}

// must already be preprocessed: we don't have sanity checking.
// input is 16*4 bytes
static inline sha256_t sha256_one_block(const void *blk) {
    const uint32_t *m32 = blk;

    // Process the message in successive 512-bit chunks:
    // break message into 512-bit chunks
    // for each chunk
    // create a 64-entry message schedule array w[0..63] of 32-bit words
    // (The initial values in w[0..63] do not matter, so many implementations zero them here)
    // copy chunk into first 16 words w[0..15] of the message schedule array
    uint32_t w[64];

    // Initialize hash values:
    // (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):
    uint32_t h0 = 0x6a09e667,
             h1 = 0xbb67ae85,
             h2 = 0x3c6ef372,
             h3 = 0xa54ff53a,
             h4 = 0x510e527f,
             h5 = 0x9b05688c,
             h6 = 0x1f83d9ab,
             h7 = 0x5be0cd19;


#   include "sha256-inner-loop.h"

    // Produce the final hash value (big-endian):
    // digest := hash := h0 append h1 append h2 append h3 append h4 append h5 append h6 append h7
    sha256_t s;
    s.digest[0] = h0 ;
    s.digest[1] = h1 ;
    s.digest[2] = h2 ;
    s.digest[3] = h3 ;
    s.digest[4] = h4 ;
    s.digest[5] = h5 ;
    s.digest[6] = h6 ;
    s.digest[7] = h7 ;
    return s;
}

// must already be preprocessed: input is 16*64 bytes
static inline sha256_t sha256_two_block(const void *blk_ptr) {
    const uint32_t *m32 = blk_ptr;

    // Process the message in successive 512-bit chunks:
    // break message into 512-bit chunks
    // for each chunk
    // create a 64-entry message schedule array w[0..63] of 32-bit words
    // (The initial values in w[0..63] do not matter, so many implementations zero them here)
    // copy chunk into first 16 words w[0..15] of the message schedule array
    uint32_t w[64];

    // Initialize hash values:
    // (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):
    uint32_t h0 = 0x6a09e667,
             h1 = 0xbb67ae85,
             h2 = 0x3c6ef372,
             h3 = 0xa54ff53a,
             h4 = 0x510e527f,
             h5 = 0x9b05688c,
             h6 = 0x1f83d9ab,
             h7 = 0x5be0cd19;


#   include "sha256-inner-loop.h"
#   include "sha256-inner-loop.h"

    // Produce the final hash value (big-endian):
    // digest := hash := h0 append h1 append h2 append h3 append h4 append h5 append h6 append h7
    sha256_t s;
    s.digest[0] = h0 ;
    s.digest[1] = h1 ;
    s.digest[2] = h2 ;
    s.digest[3] = h3 ;
    s.digest[4] = h4 ;
    s.digest[5] = h5 ;
    s.digest[6] = h6 ;
    s.digest[7] = h7 ;
    return s;
}
#endif
