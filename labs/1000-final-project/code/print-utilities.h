#ifndef __PRINT_UTILITIES_H__
#define __PRINT_UTILITIES_H__

#include <stdint.h>

#include "rpi.h"

/****************************************************************************************
 * general purpose utilities.
 */

// print [p, p+n) as a string: use for ascii filled files.
static inline void print_as_string(const char *msg, const uint8_t *p, int n) {
    printk("%s\n", msg);
    for(int i = 0; i < n; i++) {
        char c = p[i];
        printk("%c", c);
    }
    printk("\n");
}

static inline void print_dec(const char *msg, const void *p, int n) {
    printk("%s\n", msg);
    for(int i = 0; i < n; i++) {
        if(i % 16 == 0)
            printk("\n\t");
        printk("%d, ", *(uint8_t*)(p + i));
    }
    printk("\n");
}

static inline void print_bytes(const char *msg, const void *p, int n) {
    printk("%s\n", msg);
    for(int i = 0; i < n; i++) {
        if(i % 16 == 0)
            printk("\n\t");
        printk("%x, ", *(uint8_t*)(p + i));
    }
    printk("\n");
}
static inline void print_words(const char *msg, const uint32_t *p, int n) {
    printk("%s\n", msg);
    for(int i = 0; i < n; i++) {
        if(i % 16 == 0)
            printk("\n\t");
        printk("0x%x, ", p[i]);
    }
    printk("\n");
}


#endif