#include "rpi.h"
#include "fat32.h"
#include "fat32-helpers.h"
#include "pi-sd.h"

// Print extra tracing info when this is enabled.  You can and should add your
// own.
static int dump_p = 0;
static int trace_p = 1; 
static int init_p = 0;

fat32_boot_sec_t boot_sector;
struct fsinfo fs_sector;

static uint32_t get_cluster_chain_length(fat32_fs_t *fs, uint32_t start_cluster);

fat32_fs_t fat32_mk(mbr_partition_ent_t *partition)  /**/ { 
    demand(!init_p, "the fat32 module is already in use\n");

    mbr_t* mbr = mbr_read();

    // ----- Get boot partition entry -----
    mbr_partition_ent_t boot_sector_entry = mbr_get_partition(mbr, 0);
    if (dump_p)
        mbr_partition_print("Partition", &boot_sector_entry); // Print debugging

    // ----- Read and check boot sector -----
    assert(pi_sd_read(&boot_sector, boot_sector_entry.lba_start, 1));
    fat32_volume_id_check(&boot_sector);
    if (dump_p)
        fat32_volume_id_print("msg", &boot_sector);

    // ----- Read and check filesystem sector -----
    struct fsinfo fs_sector;
    assert(boot_sector.info_sec_num == 1);
    assert(pi_sd_read(&fs_sector, boot_sector_entry.lba_start + boot_sector.info_sec_num, 1));
    fat32_fsinfo_check(&fs_sector);
    if (dump_p)
        fat32_fsinfo_print("msg", &fs_sector);

    // END OF PART 2
    // The rest of this is for Part 3:

    // TODO: calculate the fat32_fs_t metadata, which we'll need to return.
    unsigned lba_start = boot_sector_entry.lba_start;
    unsigned fat_begin_lba = lba_start + boot_sector.reserved_area_nsec; // the start LBA + the number of reserved sectors
    unsigned cluster_begin_lba = fat_begin_lba + (boot_sector.nfats * boot_sector.nsec_per_fat); // the beginning of the FAT, plus the combined length of all the FATs
    unsigned sec_per_cluster = boot_sector.sec_per_cluster; // from the boot sector
    unsigned root_first_cluster = boot_sector.first_cluster; // from the boot sector
    unsigned n_entries = boot_sector.nsec_per_fat; // from the boot sector

    /*
    * TODO: Read in the entire fat (one copy: worth reading in the second and
    * comparing).
    *
    * The disk is divided into clusters. The number of sectors per
    * cluster is given in the boot sector byte 13. <sec_per_cluster>
    *
    * The File Allocation Table has one entry per cluster. This entry
    * uses 12, 16 or 28 bits for FAT12, FAT16 and FAT32.
    *
    * Store the FAT in a heap-allocated array.
    */
    uint32_t *fat = kmalloc(boot_sector.nfats * boot_sector.nsec_per_fat * NBYTES_PER_SECTOR);
    // pi_sd_read(fat, fat_begin_lba, 3);
    pi_sd_read(fat, fat_begin_lba, boot_sector.nfats * boot_sector.nsec_per_fat);

    // Create the FAT32 FS struct with all the metadata
    fat32_fs_t fs = (fat32_fs_t) {
    .lba_start = lba_start,
        .fat_begin_lba = fat_begin_lba,
        .cluster_begin_lba = cluster_begin_lba,
        .sectors_per_cluster = sec_per_cluster,
        .root_dir_first_cluster = root_first_cluster,
        .fat = fat,
        .n_entries = n_entries,
    };

    if (trace_p) {
        trace("begin lba = %d\n", fs.fat_begin_lba);
        trace("cluster begin lba = %d\n", fs.cluster_begin_lba);
        trace("sectors per cluster = %d\n", fs.sectors_per_cluster);
        trace("root dir first cluster = %d\n", fs.root_dir_first_cluster);
    }

    init_p = 1;
    return fs;
}

// Given cluster_number, get lba.  Helper function.
static uint32_t cluster_to_lba(fat32_fs_t *f, uint32_t cluster_num)  /**/ { 
    assert(cluster_num >= 2);

    unsigned lba = f->cluster_begin_lba + (cluster_num - 2) * f->sectors_per_cluster;
    if (trace_p) trace("cluster %d to lba: %d\n", cluster_num, lba);

    return lba;
}

pi_dirent_t fat32_get_root(fat32_fs_t *fs)  /**/ { 
    demand(init_p, "fat32 not initialized!");

    return (pi_dirent_t) {
      .name = "",
          .raw_name = "",
          .cluster_id = fs->root_dir_first_cluster,
          .is_dir_p = 1,
          .nbytes = 0,
      };
}

// Given the starting cluster index, get the length of the chain.  Helper
// function.
static uint32_t get_cluster_chain_length(fat32_fs_t *fs, uint32_t start_cluster) /**/ {
    // TODO: Walk the cluster chain in the FAT until you see a cluster where
    // `fat32_fat_entry_type(cluster) == LAST_CLUSTER`.  Count the number of
    // clusters.
    uint32_t length = 0;
    uint32_t cluster = start_cluster;

    // while (fat32_fat_entry_type(cluster) == USED_CLUSTER) {
    //     if (dump_p)
    //         trace_nofn("Cluster %d: %x\n", length, cluster);
            
    //     length++;
            
    //     if (length == MAX_CHAIN_LENGTH)
    //         panic("CHAIN BIGGER THAN %x\n", MAX_CHAIN_LENGTH);

    //     cluster = fs->fat[cluster];
    // } 
    // while (fat32_fat_entry_type(cluster) == USED_CLUSTER);

    do {
        length++;

        if (dump_p)
            trace_nofn("Cluster %d: %x\n", length, cluster);

        // End condition, otherwise just keep incrementing
        if (length == MAX_CHAIN_LENGTH)
            panic("CHAIN BIGGER THAN %x\n", MAX_CHAIN_LENGTH);

        cluster = fs->fat[cluster];

    } while (fat32_fat_entry_type(cluster) == USED_CLUSTER);

    return length;
}

// Given the starting cluster index, read a cluster chain into a contiguous
// buffer.  Assume the provided buffer is large enough for the whole chain.
// Helper function.
static void read_cluster_chain(fat32_fs_t *fs, uint32_t start_cluster, uint8_t *data)  /**/ {
    // TODO: Walk the cluster chain in the FAT until you see a cluster where
    // fat32_fat_entry_type(cluster) == LAST_CLUSTER.  For each cluster, copy it
    // to the buffer (`data`).  Be sure to offset your data pointer by the
    // appropriate amount each time.

    uint32_t cluster = start_cluster;
    uint32_t data_offset = 0;

    uint8_t buffer[NBYTES_PER_SECTOR * fs->sectors_per_cluster]; // for one cluster

    // while( fat32_fat_entry_type(cluster) == USED_CLUSTER ) {
    //     pi_sd_read(buffer, cluster_to_lba(fs, cluster), fs->sectors_per_cluster);
    //     memcpy256(data + data_offset, buffer, NBYTES_PER_SECTOR * fs->sectors_per_cluster);
    //     data_offset += NBYTES_PER_SECTOR;
    //     cluster = fs->fat[cluster];
    // }

    do {

        pi_sd_read(buffer, cluster_to_lba(fs, cluster), fs->sectors_per_cluster);
        memcpy256(data + data_offset, buffer, NBYTES_PER_SECTOR * fs->sectors_per_cluster);
        data_offset += NBYTES_PER_SECTOR;
        cluster = fs->fat[cluster];

    } while (fat32_fat_entry_type(cluster) == USED_CLUSTER);

}

// Converts a fat32 internal dirent into a generic one suitable for use outside
// this driver.
static pi_dirent_t dirent_convert(fat32_dirent_t *d) /**/ {
    pi_dirent_t e = {
        .cluster_id = fat32_cluster_id(d),
        .is_dir_p = d->attr == FAT32_DIR,
        .nbytes = d->file_nbytes,
    };
    // can compare this name
    memcpy(e.raw_name, d->filename, sizeof d->filename);
    // for printing.
    fat32_dirent_name(d,e.name);
    return e;
}

// Gets all the dirents of a directory which starts at cluster `cluster_start`.
// Return a heap-allocated array of dirents.
static fat32_dirent_t *get_dirents(fat32_fs_t *fs, uint32_t cluster_start, uint32_t *dir_n)  /**/ { 

    // Get lengths
    uint32_t n_clusters = get_cluster_chain_length(fs, cluster_start);
    uint32_t total_root_dir_bytes = NBYTES_PER_SECTOR * fs->sectors_per_cluster * n_clusters;
    *dir_n = total_root_dir_bytes / DIR_ENTRY_BYTES;

    if (trace_p)
        trace("Chain of %x\n", n_clusters);

    // Get data with buffer
    fat32_dirent_t* directories = kmalloc(total_root_dir_bytes);
    read_cluster_chain(fs, cluster_start, (uint8_t*)directories);

    return directories;
}

pi_directory_t fat32_readdir(fat32_fs_t *fs, pi_dirent_t *dirent)  /**/ { 
    demand(init_p, "fat32 not initialized!");
    demand(dirent->is_dir_p, "tried to readdir a file!");
    
    uint32_t n_dirents;
    fat32_dirent_t *dirents = get_dirents(fs, dirent->cluster_id, &n_dirents);

    // Count valid entries (no free() so we want to allocate minimum space)
    uint32_t n_valid = 0;
    for (int i = 0; i < n_dirents; i++) {
        if (fat32_dirent_free(&dirents[i])) continue; // free space
        if (fat32_dirent_is_lfn(&dirents[i])) continue; // LFN version of name
        if (dirents[i].attr & FAT32_VOLUME_LABEL) continue; // volume label
        n_valid++;
    }
    
    // Allocate and store valid `pi_dirent_t` (not empty dirents, LFNs, or Volume IDs)
    pi_dirent_t* dir_structs = kmalloc(n_valid * sizeof(pi_dirent_t));
    uint32_t dir_struct_idx = 0;
    for (int i = 0; i < n_dirents; i++) {
        if (fat32_dirent_free(&dirents[i])) continue; // free space
        if (fat32_dirent_is_lfn(&dirents[i])) continue; // LFN version of name
        if (dirents[i].attr & FAT32_VOLUME_LABEL) continue; // volume label
        
        dir_structs[dir_struct_idx] = dirent_convert(&dirents[i]);
        dir_struct_idx++;
    }

    return (pi_directory_t) {
        .dirents = dir_structs,
        .ndirents = n_valid
    };
}

// Uses strcmp to entry number in a directory with specific filename
static int find_dirent_with_name(fat32_dirent_t *dirents, int n, char *filename) /**/ {    

    int file_number = -1;
    char dirent_name[16];
    for (uint32_t i = 0; i < n; i++) {
        fat32_dirent_name(dirents + i, dirent_name);
        
        if (dump_p)
            trace("Filename match (%d): %s with %s\n", strcmp(filename, dirent_name), dirent_name, filename);
        if (strcmp(filename, dirent_name) == 0) {
            file_number = i;
            break;
        }
    }
  
    return file_number;
}

pi_dirent_t *fat32_stat(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) /**/ {    
    demand(init_p, "fat32 not initialized!");
    demand(directory->is_dir_p, "tried to use a file as a directory");

    // Read the raw dirent structures from the disk
    uint32_t dir_n;
    fat32_dirent_t* raw_dirents = get_dirents(fs, directory->cluster_id, &dir_n);

    // Find a dirent with the provided name.  Return NULL if no such dirent exists.
    int entry_num = -1;
    for (uint32_t i = 0; i < dir_n; i++) {
        entry_num = find_dirent_with_name(raw_dirents + i, dir_n, filename);
        if (entry_num != -1)
            break;
    }
    if (trace_p)
        trace("File number: %d\n", entry_num);

    // :( not found
    if (entry_num == -1)
        return NULL;

    // Convert raw entry into cool awesome struct (abstraction yayyy)
    pi_dirent_t *dirent = kmalloc(sizeof(pi_dirent_t));
    *dirent = dirent_convert(raw_dirents + entry_num);
    return dirent;
}

pi_file_t *fat32_read(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
    // This should be pretty similar to readdir, but simpler.
    demand(init_p, "fat32 not initialized!");
    demand(directory->is_dir_p, "tried to use a file as a directory!");

    pi_dirent_t* file_entry = fat32_stat(fs, directory, filename);
  // TODO: read the dirents of the provided directory and look for one matching the provided name

    // TODO: figure out the length of the cluster chain
    uint32_t cluster_length = get_cluster_chain_length(fs, file_entry->cluster_id);
    uint32_t n_sectors = fs->sectors_per_cluster * cluster_length;
    uint32_t total_bytes = n_sectors * NBYTES_PER_SECTOR;

    // TODO: allocate a buffer large enough to hold the whole file
    uint8_t* data = kmalloc(total_bytes);

    // TODO: read in the whole file (if it's not empty)
    pi_sd_read(data, cluster_to_lba(fs, file_entry->cluster_id), n_sectors);

    pi_file_t *file = kmalloc(sizeof(pi_file_t));
    *file = (pi_file_t) {
        .data = data,
        .n_data = file_entry->nbytes,
        .n_alloc = total_bytes,
    };

    #if 1
        trace("pi_file_t:\n");
        trace("\tdata: %x\n", file->data);
        trace("\tn_data: %x\n", file->n_data);
        trace("\tn_alloc: %x\n", file->n_data);
    #endif

    return file;
}

/******************************************************************************
 * Everything below here is for writing to the SD card (Part 7/Extension).  If
 * you're working on read-only code, you don't need any of this.
 ******************************************************************************/

static uint32_t find_free_cluster(fat32_fs_t *fs, uint32_t start_cluster) {
  // TODO: loop through the entries in the FAT until you find a free one
  // (fat32_fat_entry_type == FREE_CLUSTER).  Start from cluster 3.  Panic if
  // there are none left.
  if (start_cluster < 3) start_cluster = 3;
  unimplemented();
  if (trace_p) trace("failed to find free cluster from %d\n", start_cluster);
  panic("No more clusters on the disk!\n");
}

static void write_fat_to_disk(fat32_fs_t *fs) {
  // TODO: Write the FAT to disk.  In theory we should update every copy of the
  // FAT, but the first one is probably good enough.  A good OS would warn you
  // if the FATs are out of sync, but most OSes just read the first one without
  // complaining.
  if (trace_p) trace("syncing FAT\n");
  unimplemented();

}

// Given the starting cluster index, write the data in `data` over the
// pre-existing chain, adding new clusters to the end if necessary.
static void write_cluster_chain(fat32_fs_t *fs, uint32_t start_cluster, uint8_t *data, uint32_t nbytes) {
  // Walk the cluster chain in the FAT, writing the in-memory data to the
  // referenced clusters.  If the data is longer than the cluster chain, find
  // new free clusters and add them to the end of the list.
  // Things to consider:
  //  - what if the data is shorter than the cluster chain?
  //  - what if the data is longer than the cluster chain?
  //  - the last cluster needs to be marked LAST_CLUSTER
  //  - when do we want to write the updated FAT to the disk to prevent
  //  corruption?
  //  - what do we do when nbytes is 0?
  //  - what about when we don't have a valid cluster to start with?
  //
  //  This is the main "write" function we'll be using; the other functions
  //  will delegate their writing operations to this.

  // TODO: As long as we have bytes left to write and clusters to write them
  // to, walk the cluster chain writing them out.
  unimplemented();

  // TODO: If we run out of clusters to write to, find free clusters using the
  // FAT and continue writing the bytes out.  Update the FAT to reflect the new
  // cluster.
  unimplemented();

  // TODO: If we run out of bytes to write before using all the clusters, mark
  // the final cluster as "LAST_CLUSTER" in the FAT, then free all the clusters
  // later in the chain.
  unimplemented();

  // TODO: Ensure that the last cluster in the chain is marked "LAST_CLUSTER".
  // The one exception to this is if we're writing 0 bytes in total, in which
  // case we don't want to use any clusters at all.
  unimplemented();
}

int fat32_rename(fat32_fs_t *fs, pi_dirent_t *directory, char *oldname, char *newname) {
  // TODO: Get the dirents `directory` off the disk, and iterate through them
  // looking for the file.  When you find it, rename it and write it back to
  // the disk (validate the name first).  Return 0 in case of an error, or 1
  // on success.
  // Consider:
  //  - what do you do when there's already a file with the new name?
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("renaming %s to %s\n", oldname, newname);
  if (!fat32_is_valid_name(newname)) return 0;

  // TODO: get the dirents and find the right one
  unimplemented();

  // TODO: update the dirent's name
  unimplemented();

  // TODO: write out the directory, using the existing cluster chain (or
  // appending to the end); implementing `write_cluster_chain` will help
  unimplemented();
  return 1;

}

// Create a new directory entry for an empty file (or directory).
pi_dirent_t *fat32_create(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, int is_dir) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("creating %s\n", filename);
  if (!fat32_is_valid_name(filename)) return NULL;

  // TODO: read the dirents and make sure there isn't already a file with the
  // same name
  unimplemented();

  // TODO: look for a free directory entry and use it to store the data for the
  // new file.  If there aren't any free directory entries, either panic or
  // (better) handle it appropriately by extending the directory to a new
  // cluster.
  // When you find one, update it to match the name and attributes
  // specified; set the size and cluster to 0.
  unimplemented();

  // TODO: write out the updated directory to the disk
  unimplemented();

  // TODO: convert the dirent to a `pi_dirent_t` and return a (kmalloc'ed)
  // pointer
  unimplemented();
  pi_dirent_t *dirent = NULL;
  return dirent;
}

// Delete a file, including its directory entry.
int fat32_delete(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("deleting %s\n", filename);
  if (!fat32_is_valid_name(filename)) return 0;
  // TODO: look for a matching directory entry, and set the first byte of the
  // name to 0xE5 to mark it as free
  unimplemented();

  // TODO: free the clusters referenced by this dirent
  unimplemented();

  // TODO: write out the updated directory to the disk
  unimplemented();
  return 0;
}

int fat32_truncate(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, unsigned length) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("truncating %s\n", filename);

  // TODO: edit the directory entry of the file to list its length as `length` bytes,
  // then modify the cluster chain to either free unused clusters or add new
  // clusters.
  //
  // Consider: what if the file we're truncating has length 0? what if we're
  // truncating to length 0?
  unimplemented();

  // TODO: write out the directory entry
  unimplemented();
  return 0;
}

int fat32_write(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, pi_file_t *file) {
  demand(init_p, "fat32 not initialized!");
  demand(directory->is_dir_p, "tried to use a file as a directory!");

  // TODO: Surprisingly, this one should be rather straightforward now.
  // - load the directory
  // - exit with an error (0) if there's no matching directory entry
  // - update the directory entry with the new size
  // - write out the file as clusters & update the FAT
  // - write out the directory entry
  // Special case: the file is empty to start with, so we need to update the
  // start cluster in the dirent

  unimplemented();
}

int fat32_flush(fat32_fs_t *fs) {
  demand(init_p, "fat32 not initialized!");
  // no-op
  return 0;
}



/*

    // TODO: return the information corresponding to the root directory (just
    // cluster_id, in this case)
    uint8_t root_cluster[NBYTES_PER_SECTOR];
    print_bytes("FAT:", (uint8_t*)fs->fat, NBYTES_PER_SECTOR);
    trace("Cluster length: %x\n", get_cluster_chain_length(fs, fs->root_dir_first_cluster));


    // uint32_t clusters; = read_cluster_chain()
    // pi_sd_read(&root_cluster, cluster_to_lba(fs, fs->root_dir_first_cluster), 1);

    

    // print_bytes("Root cluster bytes:", (uint8_t*)root_dir, sizeof(fat32_dirent_t));
    // trace("%s\n", root_dir->filename);
    // fat32_dirent_t* root_dir = (fat32_dirent_t*)root_cluster;

    // fat32_dirent_print("Root:", (fat32_dirent_t*)root_cluster);
    // fat32_dirent_print("Root:", (fat32_dirent_t*)root_cluster + 0x20);
    // fat32_dirent_print("Root:", (fat32_dirent_t*)root_cluster + 0x40);
    // fat32_dirent_print("Root:", (fat32_dirent_t*)root_cluster + 0xC0);

    // print_bytes("Root cluster bytes:", (uint8_t*)&root_cluster, sizeof(root_cluster));
    // print_as_string("Filename:", root_cluster, NBYTES_PER_SECTOR);
    // print_as_string("Filename:", root_cluster + 0x20, 11);
    // print_as_string("Filename:", root_cluster + 0x40, 11);
    // print_as_string("Filename:", root_cluster + 0xC0, 11);
    
    unimplemented();

    

    pi_dirent_t dirent;





*/