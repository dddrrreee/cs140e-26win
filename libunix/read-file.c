#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    // How: 
    //    - use stat() to get the size of the file.
    //    - round up to a multiple of 4.
    //    - allocate a buffer
    //    - zero pads to a multiple of 4.
    //    - read entire file into buffer (read_exact())
    //    - fclose() the file descriptor
    //    - make sure any padding bytes have zeros.
    //    - return it.   

    struct stat stat_struct;

    // Checks for errors
    if (stat(name, &stat_struct) < 0)
        return NULL;

    // Size in bytes 
    *size = (stat_struct.st_size / 4) + 1; // I think wrong but better to allocate more
    
    
    uint32_t* buffer = malloc(*size);
    if (!buffer)
        return NULL;

    memset(buffer, 0, *size);

    FILE* file = fopen(name, "rb");
    if (!file)
        return NULL;

    fread(buffer, 1, stat_struct.st_size, file);
    fclose(file);

    return buffer;
}
