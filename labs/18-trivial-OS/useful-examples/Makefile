###########################################################
# you should look through these 5 programs and understand 
# how they work
PROGS += wait-ex 
PROGS += pipe-ex 
PROGS += pipe-simple-ex
# PROGS += pipe-detect-parent-died 
# PROGS += kill-ex 

###########################################################
# these are additional examples but not as well commented.
#

#PROGS +=  pipe-read-exit-ex 
#PROGS += local-socket 
#PROGS += test-redirect 
##PROGS += pty-ex
##PROGS += select-ex 
##PROGS += signal-ex
# PROGS += demand-ex 
#PROGS += is-fd-open 
#PROGS += dup2-redirect 

CFLAGS = -Wall -Og -Werror -std=gnu99
CC = gcc
LIBS = -lm -lutil

RUN_PROGS     := $(PROGS:=.run)
EMIT_TARGETS  := $(PROGS:=.out)
CHECK_TARGETS := $(PROGS:=.check)

TRACE_CMD = sed -n 's/\[[^]]*\]//g;/TRACE:/p'

all : $(PROGS) $(RUN_PROGS)

emit: $(EMIT_TARGETS)

check: $(CHECK_TARGETS)

# this builds each program from its %.c
%:  %.c
	$(CC) $(CFLAGS) $^  -o $@ $(LIBS)

# run each program.
$(RUN_PROGS): %.run: %  FORCE
	@printf 'about to run <%s>\n' "$<"
	@./$<

# not needed for PRELAB but example of how we strip 
# out a deterministic stable reference output (a stable residue) 
# from non-determistic.  
# mechanical rule:
#  - only grab TRACE lines and strip out everything in "[..]".
# user:
#  1. only uses <trace> for stuff that stays the same from run to run.
#  2. or wraps up non-deterministic values (e.g. pids) in "[..]"
$(EMIT_TARGETS): %.out: %
	@printf 'emitting $*.out\n' 
	@./$< 2>&1 | $(TRACE_CMD) > $*.out

# cross check output against older .out files.
$(CHECK_TARGETS): %.check: %
	@printf 'checking $<: if diff fails look at <$*.tmp> and <$*.out>\n\t'
	@./$< 2>&1 | $(TRACE_CMD) > $*.tmp
	diff -u $*.out $*.tmp
	@printf '\tSUCCESS: $< matches $*.out\n'
	@rm -f $*.tmp

# used to force fake targets to get made.
FORCE:;

clean :
	rm -f *.o *~ Makefile.bak $(PROGS)

# declare all rules that don't generate files with their name as phoney
.PHONY: $(RUN_PROGS) $(CHECK_TARGETS) clean all FORCE emit check

