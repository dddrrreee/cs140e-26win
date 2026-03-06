#include "rpi.h"
#include "../pi-sd.h"
#include "../fat32.h"

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

    printk("Listing files:\n");
    pi_directory_t files = fat32_readdir(&fs, &root);
    printk("Got %d files.\n", files.ndirents);
    for (int i = 0; i < files.ndirents; i++) {
        pi_dirent_t *dirent = &files.dirents[i];
        if (dirent->is_dir_p) {
            printk("\tD: %s (cluster %d)\n", dirent->name, dirent->cluster_id);
        
            // ----- For more nests -----
            // pi_directory_t files_nested = fat32_readdir(&fs, dirent);

            // for (int i = 0; i < files_nested.ndirents; i++) {
            //     pi_dirent_t *nested_dirent = &files_nested.dirents[i];
            //     if (nested_dirent->is_dir_p) {
            //         printk("\tD: %s (cluster %d)\n", nested_dirent->name, nested_dirent->cluster_id);
                
            //         pi_directory_t files_nested_2 = fat32_readdir(&fs, nested_dirent);

            //         for (int i = 0; i < files_nested_2.ndirents; i++) {
            //             pi_dirent_t *nested_dirent_2 = &files_nested_2.dirents[i];
            //             if (nested_dirent_2->is_dir_p) {
            //                 printk("\tD: %s (cluster %d)\n", nested_dirent_2->name, nested_dirent_2->cluster_id);
                        
            //             } else {
            //                 printk("\tF: %s (cluster %d; %d bytes)\n", nested_dirent_2->name, nested_dirent_2->cluster_id, nested_dirent_2->nbytes);
            //             }
            //         }
            //     } else {
            //         printk("\tF: %s (cluster %d; %d bytes)\n", nested_dirent->name, nested_dirent->cluster_id, nested_dirent->nbytes);
            //     }
            // }
            
        } else {
            printk("\tF: %s (cluster %d; %d bytes)\n", dirent->name, dirent->cluster_id, dirent->nbytes);
        }
    }
    printk("PASS: %s\n", __FILE__);
}
