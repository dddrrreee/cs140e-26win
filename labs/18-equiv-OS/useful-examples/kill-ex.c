// engler, cs140e: show how to catch control-c and handle writes to a closed pipe.
// usage: run and control-c it.
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#include "demand.h"

static int parent_pid, kid_pid;

void catch_siguser(int sig) {
    int pid = getpid();
    trace("CHILD: pid=[%d], caught a siguser!\n", pid);
    assert(pid == kid_pid);
    exit(0);
}

static volatile int caught_sigint = 0;
void handler_sigint(int sig) {
    int pid = getpid();
    trace("PARENT: pid[%d]: caught a sigint!\n", pid);
    assert(parent_pid == pid);
    caught_sigint = 1;
}

void catch_cntrl_c(void (*handler)(int)) {
    // at this point need to catch control-c, otherwise will loose the log record.
    struct sigaction sig;
    sig.sa_handler = handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    if(sigaction(SIGINT, &sig, NULL) < 0)
        sys_die(sigaction, could not install);
}


int main(void) {
    trace("signal test: have seperate signal handling for parent and child\n");
    // 1. setup signal handler for SIGUSER1
    if(signal(SIGUSR1, catch_siguser) < 0)
        sys_die(signal, cannot install siguser);

    // 2. fork child.
    parent_pid = getpid();
	if(!(kid_pid=fork())) {
        kid_pid = getpid();
        // if you get rid of this both processes will get a control-c and the 
        // the child will die.
        if(setsid() < 0)
            sys_die(setsid, setsid-failed);
        // child: busy wait until killed.
        trace("child is about to infinite loop\n");
        while(1)
            sleep(1);
	}

    // 3 setup control C handler in parent.  (not in child) and show
    //   how it propogates.
    catch_cntrl_c(handler_sigint);
    // you should control-c this process.
    while(!caught_sigint) {
        output("USER: do a control-c to wake up parent!\n");
        sleep(1);
    }

    // 4. parent sends a SIGUSR1 to child: it catches it with
    //    the inherited signal mask above (1)
	trace("done waiting: about to kill child=[%d]\n", kid_pid);
    if(kill(kid_pid, SIGUSR1) < 0)
        sys_die(kill, kill failed?);

    // 5. wait for child: check its exit code and its status: we expect
    //    exitcode =0
    int status;
    if(waitpid(kid_pid, &status, 0) < 0)
        sys_die("waitpid", waitpid failed?);
    if(WIFSIGNALED(status))
        panic("kid killed by signal: %d (sig=%d)\n", status, WTERMSIG(status));

    if(!WIFEXITED(status))
        panic("status failed: status=%d, exitstatus=%d\n", status, WEXITSTATUS(status));
    trace("SUCCESS: status=[%d]\n", WEXITSTATUS(status));
	return 0;
}
