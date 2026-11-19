# Makefile — AVIAN-MORPH C11 kernel ABI (src/c11).
#
# Targets:
#   build  — compile the kernels and the test programs.
#   test   — regenerate the CBQN fixtures, build, and run the tests.
#            Exits 0 only when every check passes.
#   clean  — remove build artifacts (not the fixtures or sources).
#
# The CBQN reference is the oracle: fixtures are regenerated from
# cbqn/scale.bqn, cbqn/conv1d.bqn, cbqn/reduce_segments.bqn,
# cbqn/gather_scatter.bqn, cbqn/geom_transform.bqn,
# cbqn/complex_rotate.bqn, and cbqn/normalize.bqn on every `make test`,
# so the C kernels are always compared against a fresh CBQN run,
# never a stale file.

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -O2 -std=c11 -fno-builtin
LDFLAGS := -lm
CBQN    := $(HOME)/workspace/.tools/cbqn/cbqn
ROOT    := ../..
FIXDIR  := $(ROOT)/fixtures/arrays

SCALE_KERNEL := avian_scale.c
SCALE_TEST   := test_avian_scale.c
SCALE_BIN    := test_avian_scale

CONV_KERNEL := avian_conv1d.c
CONV_TEST   := test_avian_conv1d.c
CONV_BIN    := test_avian_conv1d

REDUCE_KERNEL := avian_reduce.c
REDUCE_TEST   := test_avian_reduce.c
REDUCE_BIN    := test_avian_reduce

GATHER_KERNEL := avian_gather.c
GATHER_TEST   := test_avian_gather.c
GATHER_BIN    := test_avian_gather

GEOM_KERNEL := avian_geom.c
GEOM_TEST   := test_avian_geom.c
GEOM_BIN    := test_avian_geom

COMPLEX_KERNEL := avian_complex.c
COMPLEX_TEST   := test_avian_complex.c
COMPLEX_BIN    := test_avian_complex

NORM_KERNEL := avian_norm.c
NORM_TEST   := test_avian_norm.c
NORM_BIN    := test_avian_norm

.PHONY: build test fixtures clean

build: $(SCALE_BIN) $(CONV_BIN) $(REDUCE_BIN) $(GATHER_BIN) $(GEOM_BIN) $(COMPLEX_BIN) $(NORM_BIN)

$(SCALE_BIN): $(SCALE_KERNEL) avian_array.h $(SCALE_TEST)
	$(CC) $(CFLAGS) -o $@ $(SCALE_KERNEL) $(SCALE_TEST) $(LDFLAGS)

$(CONV_BIN): $(CONV_KERNEL) avian_conv1d.h avian_array.h $(CONV_TEST)
	$(CC) $(CFLAGS) -o $@ $(CONV_KERNEL) $(CONV_TEST) $(LDFLAGS)

$(REDUCE_BIN): $(REDUCE_KERNEL) avian_reduce.h avian_array.h $(REDUCE_TEST)
	$(CC) $(CFLAGS) -o $@ $(REDUCE_KERNEL) $(REDUCE_TEST) $(LDFLAGS)

$(GATHER_BIN): $(GATHER_KERNEL) avian_gather.h avian_array.h $(GATHER_TEST)
	$(CC) $(CFLAGS) -o $@ $(GATHER_KERNEL) $(GATHER_TEST) $(LDFLAGS)

$(GEOM_BIN): $(GEOM_KERNEL) avian_geom.h avian_array.h $(GEOM_TEST)
	$(CC) $(CFLAGS) -o $@ $(GEOM_KERNEL) $(GEOM_TEST) $(LDFLAGS)

$(COMPLEX_BIN): $(COMPLEX_KERNEL) avian_complex.h avian_array.h $(COMPLEX_TEST)
	$(CC) $(CFLAGS) -o $@ $(COMPLEX_KERNEL) $(COMPLEX_TEST) $(LDFLAGS)

$(NORM_BIN): $(NORM_KERNEL) avian_norm.h avian_array.h $(NORM_TEST)
	$(CC) $(CFLAGS) -o $@ $(NORM_KERNEL) $(NORM_TEST) $(LDFLAGS)

fixtures: $(FIXDIR)/scale_f32.txt $(FIXDIR)/scale_i32.txt $(FIXDIR)/scale_f32_large.txt $(FIXDIR)/conv1d_f32.txt $(FIXDIR)/conv1d_f32_large.txt $(FIXDIR)/reduce_f32.txt $(FIXDIR)/reduce_f32_large.txt $(FIXDIR)/gather_f32.txt $(FIXDIR)/scatter_f32.txt $(FIXDIR)/gather_f32_large.txt $(FIXDIR)/scatter_f32_large.txt $(FIXDIR)/geom_f32.txt $(FIXDIR)/geom_f32_large.txt $(FIXDIR)/complex_f32.txt $(FIXDIR)/complex_f32_large.txt $(FIXDIR)/norm_f32.txt $(FIXDIR)/norm_f32_large.txt

$(FIXDIR)/scale_f32.txt: $(ROOT)/cbqn/scale.bqn
	$(CBQN) -f $(ROOT)/cbqn/scale.bqn 0 > $@

$(FIXDIR)/scale_i32.txt: $(ROOT)/cbqn/scale.bqn
	$(CBQN) -f $(ROOT)/cbqn/scale.bqn 1 > $@

$(FIXDIR)/scale_f32_large.txt: $(ROOT)/cbqn/scale.bqn
	$(CBQN) -f $(ROOT)/cbqn/scale.bqn 2 > $@

$(FIXDIR)/conv1d_f32.txt: $(ROOT)/cbqn/conv1d.bqn
	$(CBQN) -f $(ROOT)/cbqn/conv1d.bqn 0 > $@

$(FIXDIR)/conv1d_f32_large.txt: $(ROOT)/cbqn/conv1d.bqn
	$(CBQN) -f $(ROOT)/cbqn/conv1d.bqn 1 > $@

$(FIXDIR)/reduce_f32.txt: $(ROOT)/cbqn/reduce_segments.bqn
	$(CBQN) -f $(ROOT)/cbqn/reduce_segments.bqn 0 > $@

$(FIXDIR)/reduce_f32_large.txt: $(ROOT)/cbqn/reduce_segments.bqn
	$(CBQN) -f $(ROOT)/cbqn/reduce_segments.bqn 1 > $@

$(FIXDIR)/gather_f32.txt: $(ROOT)/cbqn/gather_scatter.bqn
	$(CBQN) -f $(ROOT)/cbqn/gather_scatter.bqn 0 > $@

$(FIXDIR)/scatter_f32.txt: $(ROOT)/cbqn/gather_scatter.bqn
	$(CBQN) -f $(ROOT)/cbqn/gather_scatter.bqn 1 > $@

$(FIXDIR)/gather_f32_large.txt: $(ROOT)/cbqn/gather_scatter.bqn
	$(CBQN) -f $(ROOT)/cbqn/gather_scatter.bqn 2 > $@

$(FIXDIR)/scatter_f32_large.txt: $(ROOT)/cbqn/gather_scatter.bqn
	$(CBQN) -f $(ROOT)/cbqn/gather_scatter.bqn 3 > $@

$(FIXDIR)/geom_f32.txt: $(ROOT)/cbqn/geom_transform.bqn
	$(CBQN) -f $(ROOT)/cbqn/geom_transform.bqn 0 > $@

$(FIXDIR)/geom_f32_large.txt: $(ROOT)/cbqn/geom_transform.bqn
	$(CBQN) -f $(ROOT)/cbqn/geom_transform.bqn 1 > $@

$(FIXDIR)/complex_f32.txt: $(ROOT)/cbqn/complex_rotate.bqn
	$(CBQN) -f $(ROOT)/cbqn/complex_rotate.bqn 0 > $@

$(FIXDIR)/complex_f32_large.txt: $(ROOT)/cbqn/complex_rotate.bqn
	$(CBQN) -f $(ROOT)/cbqn/complex_rotate.bqn 1 > $@

$(FIXDIR)/norm_f32.txt: $(ROOT)/cbqn/normalize.bqn
	$(CBQN) -f $(ROOT)/cbqn/normalize.bqn 0 > $@

$(FIXDIR)/norm_f32_large.txt: $(ROOT)/cbqn/normalize.bqn
	$(CBQN) -f $(ROOT)/cbqn/normalize.bqn 1 > $@

test: fixtures build
	./$(SCALE_BIN) $(FIXDIR)
	./$(CONV_BIN) $(FIXDIR)
	./$(REDUCE_BIN) $(FIXDIR)
	./$(GATHER_BIN) $(FIXDIR)
	./$(GEOM_BIN) $(FIXDIR)
	./$(COMPLEX_BIN) $(FIXDIR)
	./$(NORM_BIN) $(FIXDIR)

clean:
	rm -f $(SCALE_BIN) $(CONV_BIN) $(REDUCE_BIN) $(GATHER_BIN) $(GEOM_BIN) $(COMPLEX_BIN) $(NORM_BIN)
