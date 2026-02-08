// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    // if your system uses another name, add it.
};
static const char* dev_prefix = "/dev/";

static int filter(const struct dirent *d) { // ** MODIFIED BUT CHECK
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.

    const char* name = d->d_name;

    unsigned n_prefixes = sizeof(ttyusb_prefixes)/sizeof(ttyusb_prefixes[0]);

    for (unsigned i = 0; i < n_prefixes; i++) {
        // String compare like in the first lab

        // if ( strncmp(ttyusb_prefixes[i], name, 
        //             strlen(ttyusb_prefixes[i])) == 0) {
        //     return 1; 
        // }
        if (prefix_cmp(name, ttyusb_prefixes[i]))
            return 1;
        
    }
    
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) { // ** MODIFIED BUT CHECK
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    // unimplemented();

    struct dirent **names;
    //           path      dirent  should include?   sorting func 
    unsigned n = scandir("/dev", &names, filter, alphasort);
    if (n == 0)
        panic("device number is 0");

    if (n != 1) {
        printf("More names than 1:\n");

        for (int i = 0; i < n; i++)
            printf("- %s\n", names[i]->d_name);
        printf("\n\n");
        panic("device number is %d, not 1", n);
    }
        

    char *name = malloc(strlen(names[0]->d_name) + strlen(dev_prefix) + 1); // for dev and 0 termination 
    if (!name)
        panic("malloc no space");

    strcpy(name, dev_prefix);
    strcpy(name + 5, names[0]->d_name);

    // free the dirent pointers allocated by scandir
    // since dirent actually allocates already
    // Thanks https://stackoverflow.com/questions/18402428/how-to-properly-use-scandir-in-c
    // This caught me 
    for (int i = 0; i < n; i++)
        free(names[i]);
    
    free(names);

    return name;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) { // ** MODIFIED BUT CHECK

    struct dirent **names;
    int n_files = scandir("/dev", &names, filter, alphasort);
    if (n_files <= 0) {
        printf("Could not find USB-UART connection in /dev/ with any of the prefixes: \n");
        for (unsigned i = 0; i < sizeof(ttyusb_prefixes) / sizeof(const char*); i++) {
            printf("  - %s\n", ttyusb_prefixes[i]);
        }
        panic("Plug in Pi?\n");
    }

    
    char *newest_name = NULL;
    time_t newest_modified_time = 0;

    for (int i = 0; i < n_files; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/dev/%s", names[i]->d_name);

        struct stat st;
        if (stat(path, &st) < 0) continue;

        if (!newest_name || st.st_mtime > newest_modified_time) {

            newest_name = strdup(names[i]->d_name);
            newest_modified_time = st.st_mtime;

            // // Making sure to delete the malloc DOES THIS WORK?
            // free(newest_name); 
            // printf("Saw %s\n", names[i]->d_name);
        }

        // printf("Freeing %s\n", names[i]->d_name);
        free(names[i]);
    }

    free(names);


    // char newest_name[256];
    // memset(newest_name, 0, 256);

    // time_t newest_modified_time = 0;

    // for (int i = 0; i < n_files; i++) {
    //     char path[256];
    //     snprintf(path, sizeof(path), "/dev/%s", names[i]->d_name);

    //     struct stat st;
    //     if (stat(path, &st) < 0) continue;

    //     if (st.st_mtime > newest_modified_time) {
            
    //         memset(newest_name, 0, 256);
    //         snprintf(newest_name, sizeof(newest_name), "/dev/%s", names[i]->d_name);
    //         newest_modified_time = st.st_mtime;

    //         // // Making sure to delete the malloc DOES THIS WORK?
    //         // free(newest_name); 
    //         printf("Saw %s\n", names[i]->d_name);
    //     }

    //     printf("Freeing %s\n", names[i]->d_name);
    //     free(names[i]);
    // }

    

    if (!newest_name || strlen(newest_name) == 0) {
        panic("No ttyusb found :(\t");
    }

    char *name = malloc(strlen(newest_name) + strlen(dev_prefix) + 1); // for dev and 0 termination 
    if (!name)
        panic("malloc no space");

    strcpy(name, dev_prefix);
    strcpy(name + strlen(dev_prefix), newest_name);


    return name;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) { // ** MODIFIED BUT CHECK

    struct dirent **names;
    int n_files = scandir("/dev", &names, filter, alphasort);
    if (n_files <= 0)
        panic("Bad number of files (%d)", n_files);

    char *oldest_name = NULL;
    time_t oldest_modified_time = 0;

    for (int i = 0; i < n_files; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/dev/%s", names[i]->d_name);

        struct stat st;
        if (stat(path, &st) < 0) continue;

        if (!oldest_name || st.st_mtime < oldest_modified_time) {

            oldest_name = strdup(names[i]->d_name);
            oldest_modified_time = st.st_mtime;

            // // Making sure to delete the malloc DOES THIS WORK
            // free(oldest_name);
        }
        printf("Freeing %s\n", names[i]->d_name);
        free(names[i]);
    }

    free(names);

    if (!oldest_name) {
        panic("no ttyusb");
    }

    return oldest_name;
}
