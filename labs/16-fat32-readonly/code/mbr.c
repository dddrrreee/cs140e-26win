#include "rpi.h"
#include "pi-sd.h"
#include "mbr.h"

#include "demand.h"

mbr_t *mbr_read() {
    // Be sure to call pi_sd_init() before calling this function!
    // TODO: Read the MBR into a heap-allocated buffer.  Use `pi_sd_read` or
    // `pi_sec_read` to read 1 sector from LBA 0 into memory.
    // unimplemented();
    mbr_t* mbr = kmalloc(NBYTES_PER_SECTOR);
    assert(pi_sd_read(mbr, 0, 1)); // MBR starts at 0, reading 1 sector
    
    // Verify that the MBR signature is valid (most important)
    // assert(mbr->sigval == 0xAA55);
    mbr_check(mbr);

    // Verify 

    // trace("Type code: %x\n", mbr->code[0x0B + 1]);
    // uint16_t bytes_per_sector = *(uint16_t*)(mbr->code + 0x0B);
    // trace("Bytes per sector: %x\n", bytes_per_sector);
    // // trace("Bytes per sector: %x %x\n", mbr->code[0x0B], mbr->code[0x0B + 1]);
    // assert(mbr->sigval == 0xAA55);

    // for (int i = 0; i < NBYTES_PER_SECTOR; i++) {
    //     trace("%x/%d: %x\n", i, i, *(uint8_t*)(mbr->code + i));
    // }

    // // not aligned so have to copy.
    // struct partition_entry p;
    // memcpy(&p, mbr->part_tab1, sizeof p);

    // assert(mbr_part_is_fat32(p.part_type));

    // // verify only the first partition is non-empty.
    // // this could fail if you were fancy and re-partitioned your
    // // sd card.  (we should actually do so so that we can make a
    // // custom file system).
    // assert(!mbr_partition_empty_raw(mbr->part_tab1));
    // assert(mbr_partition_empty_raw(mbr->part_tab2));
    // assert(mbr_partition_empty_raw(mbr->part_tab3));
    // assert(mbr_partition_empty_raw(mbr->part_tab4));

    // TODO: Return the MBR.
    return mbr;
}
