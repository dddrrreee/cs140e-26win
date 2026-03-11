#include "rpi.h"
#include "../pi-sd.h"
#include "../fat32.h"
#include "libc/fast-hash32.h"
    
#include "../fat32-helpers.h"

#include "asm-helpers.h"

extern void fat32_jump_trampoline(uint32_t addr);

void notmain() {
  kmalloc_init_mb(FAT32_HEAP_MB);
  pi_sd_init();

  printk("Reading the MBR.\n");
  mbr_t *mbr = mbr_read();

  printk("Loading the first partition.\n");
  mbr_partition_ent_t partition;
  memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
  assert(mbr_part_is_fat32(partition.part_type));

  printk("Loading the FAT.\n");
  fat32_fs_t fs = fat32_mk(&partition);

  printk("Loading the root directory.\n");
  pi_dirent_t root = fat32_get_root(&fs);

#if 0
  printk("Listing files:\n");
  uint32_t n;
  pi_directory_t files = fat32_readdir(&fs, &root);
  printk("Got %d files.\n", files.ndirents);
  for (int i = 0; i < files.ndirents; i++) {
    if (files.dirents[i].is_dir_p) {
      printk("\tD: %s (cluster %d)\n", files.dirents[i].name, files.dirents[i].cluster_id);
    } else {
      printk("\tF: %s (cluster %d; %d bytes)\n", files.dirents[i].name, files.dirents[i].cluster_id, files.dirents[i].nbytes);
    }
  }
#endif

    char *name = "BOOTCODE.BIN";
    printk("Looking for %s.\n", name);
    pi_dirent_t *d = fat32_stat(&fs, &root, name);
    demand(d, "bootcode.bin not found!\n");

    printk("Reading %s\n", name);
    pi_file_t *f = fat32_read(&fs, &root, name);
    assert(f);

    printk("crc of %s (nbytes=%d) = %x\n", name, f->n_data, 
            fast_hash(f->data,f->n_data));

    name = "HELLO-F.BIN";
    printk("Looking for %s.\n", name);
    d = fat32_stat(&fs, &root, name);
    demand(d, "%s: not found!  Did you copy to microSD?\n", name);

    printk("Reading %s\n", name);
    f = fat32_read(&fs, &root, name);
    assert(f);

    printk("crc of %s (nbytes=%d) = %x\n", name, f->n_data, 
            fast_hash(f->data,f->n_data));

    uint32_t *p = (void*)f->data;
    for(int i = 0; i < 4; i++) 
        printk("p[%d]=%x (%d)\n", i,p[i],p[i]);

    // magic cookie at offset 0.
    assert(p[0] == 0x12345678);

    // address to copy at is at offset 2
    uint32_t addr = p[2];
    assert(addr == 0x9000000);
    
    uint32_t header_size = p[1];
    assert(header_size == 0x10);

    uint32_t program_start = addr + header_size;


    trace("about to call <%s>\n", name);

    trace("Heap addr: %x (%d)\n", f->data, f->data);
    trace("Program Addr: %x (%d)\n", addr, addr);
    trace("Header size: %x (%d)\n", header_size, header_size);
    trace("Actual start: %x (%d)\n", program_start, program_start);

    // Copy data to where the program thinks it is in the heap
    memcpy((void*)addr, f->data, f->n_data);

    
    // jump to it using BRANCHTO.  make sure
    // you skip the header!  (see in hello-f.list
    // and memmap.fixed in 13-fat32/hello-fixed
    // unimplemented();
    // // LINK_ADDR+SIZEOF(.crt0_header)


    // uint32_t header_size = p[1];
    // uint32_t program_start = header_size + addr;
    // uint8_t* actual_start = f->data + program_start; 
    // trace("Calling at %x\n", actual_start);

    // Verify program is loaded into the heap correctly
    print_bytes("Program bytes", (uint8_t*)f->data, 64);
    print_bytes("Program bytes", (uint8_t*)program_start, 64);

    void(*cursed)(void) = (void (*)(void))(program_start);

    cursed();
    // BRANCHTO(addr + header_size);
    // fat32_jump_trampoline(program_start);


    trace("returned from <%s>!\n", name);

    printk("PASS: %s\n", __FILE__);
}
