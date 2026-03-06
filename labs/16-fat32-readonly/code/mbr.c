#include "rpi.h"
#include "pi-sd.h"
#include "mbr.h"

#include "demand.h"

mbr_t *mbr_read() { // **
    // Be sure to call pi_sd_init() before calling this function!

    // Read MBR from sector 0
    mbr_t* mbr = kmalloc(NBYTES_PER_SECTOR);
    assert(pi_sd_read(mbr, 0, 1)); 
    
    // Verify mbr
    mbr_check(mbr);

    return mbr;
}
