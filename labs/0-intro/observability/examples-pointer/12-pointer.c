// different examples of broken double-check lock.
#include <stdlib.h>

void lock(void);
void unlock(void);

static int *ptr = 0;

// basic double check lock.
//   <ptr> non-null before assignment.
int double_check0(void) {
        if(!ptr) {
            lock();
            if(!ptr) {
                ptr = malloc(sizeof *ptr);
                *ptr = 1;
            }
            unlock();
        }
        return *ptr;
}


// "fix" that does nothing: assign to a tmp,
// modify that, then assign to <ptr>.  compiler
// of course ignores <tmp>
int double_check1(void) {
        if(!ptr) {
            lock();
            if(!ptr) {
                int *tmp = malloc(sizeof *ptr);
                *tmp = 1;
                ptr = tmp;
            }
            unlock();
        }
        return *ptr;
}


int init;

// "fix" using an <init> variable: compiler can reorder.
int double_check2(void) {
        if(!init) {
            lock();
            if(!init) {
                int *tmp = malloc(sizeof *ptr);
                *tmp = 2;
                ptr = tmp;
                init = 1;
            }
            unlock();
        }
        return *ptr;
}










