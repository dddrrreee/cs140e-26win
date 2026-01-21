// process a single 64-byte (512-bit) chunk in <m32>.
//
// we could make an inline function for this, but then we'd have to pass
// the address of various things (e.g., <h>) --- i don't want to worry 
// that gcc gets confused so we do things in this gross way.

// NOTE: increments m32 at the end, so have to modify the loops if you pull back into
// the original sha implementations.
{

        // I don't know if it matters, but I think we can remove this copy 
        // and just destructively modify the input.
        //  - 16 32-bit entries = 512 bits.  
        for(unsigned i = 0; i < 16; i++)
            w[i] = (m32[i]);

        // Extend the first 16 words into the remaining 48 words w[16..63] of the message 
        // schedule array:
        for(unsigned i = 16; i < 64; i++)  {
            uint32_t s0 = rrot32(w[i-15], 7) ^ rrot32(w[i-15], 18) ^ (w[i-15] >> 3);
            uint32_t s1 = rrot32(w[i- 2], 17) ^ rrot32(w[i- 2],  19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }

        // Initialize working variables to current hash value:
        uint32_t a = h0,
                b = h1,
                c = h2,
                d = h3,
                e = h4,
                f = h5,
                g = h6,
                h = h7;

        // Compression function main loop:
        for(unsigned i = 0; i < 64; i++) {
            uint32_t S1 = rrot32(e, 6) ^ rrot32(e, 11) ^ rrot32(e, 25);
            uint32_t ch = (e & f) ^ ((~ e) & g);
            uint32_t temp1 = h + S1 + ch + k[i] + w[i];
            uint32_t S0 = rrot32(a, 2) ^ rrot32(a, 13) ^ rrot32(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;
     
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        // Add the compressed chunk to the current hash value:
        h0 = h0 + a;
        h1 = h1 + b;
        h2 = h2 + c;
        h3 = h3 + d;
        h4 = h4 + e;
        h5 = h5 + f;
        h6 = h6 + g;
        h7 = h7 + h;

        m32 += 16;
}
