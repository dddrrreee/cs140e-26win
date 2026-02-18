// engler: trivial pipe example --- send hello world to ourself.
#include <assert.h>
#include <unistd.h>
#include "demand.h"

int main(void) {
	int fds[2];

    // fds[0] = readable fd, fds[1] = writeable fd
	if(pipe(fds) < 0)
		sys_die(pipe, cannot create pipe fds);

    unsigned char hello[] = "hello world!", *p, got;
    trace("about to send <%s> to ourself over a pipe\n", hello);

    for(p = hello; *p; p++) {
        if(write(fds[1], p, 1) != 1)
            sys_die(write, bad write);
        if(read(fds[0], &got, 1) != 1)
            sys_die(read, bad read);
        trace("wrote<%c>, read <%c>\n", *p,got);
        assert(*p == got);
    }
	return 0;
}
