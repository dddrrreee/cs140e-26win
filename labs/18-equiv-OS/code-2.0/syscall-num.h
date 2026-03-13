#ifndef __SYSCALL_NUMS_H__
#define __SYSCALL_NUMS_H__

// some calls needed for Unix.  some are needed for
// equiv checking.   we have the latter at the end.

#define SYS_EXIT        1
#define SYS_FORK        2
#define SYS_EXEC        3
#define SYS_WAITPID     4
#define SYS_SBRK        5

#define SYS_OPEN        6
#define SYS_CLOSE       7
#define SYS_READ        8
#define SYS_WRITE       9
#define SYS_DUP         10

#define SYS_ABORT       11


// not Unix core syscalls.

#define SYS_PUTC            128
#define SYS_GET_CPSR        129

// printing: provided so the test binary is small.
#define SYS_PUT_HEX         130 
#define SYS_PUT_INT         131
#define SYS_PUT_PID         132

#define SYS_MAX             256


#endif
