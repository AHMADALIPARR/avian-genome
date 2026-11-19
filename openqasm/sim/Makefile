# Makefile — AVIAN-MORPH reference circuit simulator.
#
# Targets:
#   build  — compile the simulator and the test program.
#   test   — build and run the tests. Exits 0 only when every
#            check passes.
#   clean  — remove build artifacts (not the sources).
#
# The simulator verifies the OpenQASM circuit templates in
# openqasm/ by execution. It is a reference simulator, not a
# performance kernel. See CONTRACT.md.

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -O2 -std=c11
LDFLAGS := -lm

SIMSRC  := qsim.c
TESTSRC := test_qsim.c
TESTBIN := test_qsim
RUNSRC  := qsim_run.c
RUNBIN  := qsim_run

.PHONY: build test clean

build: $(TESTBIN) $(RUNBIN)

$(TESTBIN): $(SIMSRC) qsim.h $(TESTSRC)
	$(CC) $(CFLAGS) -o $@ $(SIMSRC) $(TESTSRC) $(LDFLAGS)

$(RUNBIN): $(SIMSRC) qsim.h $(RUNSRC)
	$(CC) $(CFLAGS) -o $@ $(SIMSRC) $(RUNSRC) $(LDFLAGS)

test: build
	./$(TESTBIN)

clean:
	rm -f $(TESTBIN) $(RUNBIN)
