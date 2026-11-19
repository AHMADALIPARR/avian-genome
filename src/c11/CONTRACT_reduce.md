Numerical contract — avian_reduce_segments_f32 (C11 kernel ABI)
===============================================================

Scope
-----
This contract covers the segmented reduction kernel in
src/c11/avian_reduce.c: avian_reduce_segments_f32. It states its
exact semantics, its error behaviour, and the limits of what is
verified here.

Semantics
---------
For each s in [0, lengths.count):
  output[s] = sum of input[off[s] .. off[s]+len[s]-1]
where len[s] is lengths.data[s] and off is the exclusive prefix
sum of the lengths (off[0] = 0, off[s+1] = off[s] + len[s]).
Each segment is accumulated left to right in f32, starting from
+0.0f:
  acc = 0.0f;
  acc = acc + input[off + 0];
  acc = acc + input[off + 1];
  ...
An empty segment (length 0) sums to +0.0f. The total of all
segment lengths must equal input.count.

There is no cross-segment accumulation: output[s] depends only
on the input elements of segment s. The kernel is deterministic:
the same inputs always give the same outputs and the same status
code.

Accumulation order
------------------
The accumulation is strictly left to right within a segment.
Each addition is one correctly-rounded IEEE 754 single-precision
add (round-to-nearest-even). The order is fixed, so the result
does not depend on how the segments are scheduled or on the
number of segments. A different order (for example a tree
reduction) would give a different rounding and is NOT what this
kernel computes. The CBQN reference accumulates in the same
left-to-right order.

Empty segments and signed zero
------------------------------
An empty segment sums to +0.0f: the accumulation starts from
+0.0f and adds nothing. A segment of all zeros also sums to
+0.0f: the accumulation starts from +0.0f, and adding a zero of
either sign to +0.0f gives +0.0f, so the sign of a zero input
does not survive the sum. A zero sum is therefore always +0.0f,
never -0.0f. Verified by test.

Preconditions and checks (deterministic order)
----------------------------------------------
1. lengths.count == 0 and input.count == 0 is valid. The kernel
   returns AVIAN_OK and writes nothing, even if a data pointer
   is NULL.
2. If lengths.count > 0, lengths.data must be non-NULL; if
   input.count > 0, input.data must be non-NULL; if
   lengths.count > 0, output.data must be non-NULL; else
   AVIAN_ERR_NULL.
3. output.count >= lengths.count, else AVIAN_ERR_COUNT. The
   output holds one sum per segment.
4. Every segment length must be non-negative, and the total of
   all lengths must equal input.count, else AVIAN_ERR_COUNT. A
   negative length is a length error and is reported with
   AVIAN_ERR_COUNT; the status enum has no separate invalid-value
   code, and the length is part of the count contract. The
   running total is compared against input.count as it grows, so
   it can never exceed input.count and cannot wrap: once the
   running total exceeds input.count the total can never equal
   input.count, so the kernel returns immediately. A negative
   length is rejected before it is added.
5. The written output range
   [output.data, output.data + lengths.count) must not overlap
   the read input range [input.data, input.data + input.count)
   or the read lengths range
   [lengths.data, lengths.data + lengths.count),
   else AVIAN_ERR_OVERLAP. In-place reduction
   (output.data == input.data) is rejected, and writing into
   the lengths buffer is rejected: the lengths are read
   throughout the computation, so writing into them would
   corrupt the result.
6. Every input element must be finite, and every accumulation
   must be finite, else AVIAN_ERR_NONFINITE. The accumulation is
   checked after each addition. A non-finite input makes its
   accumulation non-finite at the step where it is added, and a
   sum that overflows to infinity is non-finite, so checking
   after each step is sufficient. Every input element belongs
   to exactly one segment (the lengths total input.count), so
   all inputs are covered.

When any check fails the kernel writes nothing to the output
buffer and returns the error code. The check order above is
fixed, so when several preconditions fail at once the returned
code is the one from the earliest check. The kernel allocates
nothing.

Floating-point behaviour (f32)
-------------------------------
- Rounding: round-to-nearest-even (the default IEEE 754 mode),
  one rounding per addition.
- Signed zero: a zero result is +0.0f (see the empty-segment
  rule above). Verified by test.
- NaN: never accepted as input and never produced. A NaN input,
  or an accumulation that produces NaN, returns
  AVIAN_ERR_NONFINITE.
- Infinity: never accepted as input and never produced. An
  infinite input, or a sum that overflows to infinity, returns
  AVIAN_ERR_NONFINITE.
- Subnormals: an input subnormal is finite and is accepted; the
  accumulation is the correctly-rounded single-precision result
  (it may be a subnormal or zero). This follows IEEE 754 and is
  not specially cased.
- Overflow: a sum whose magnitude exceeds FLT_MAX returns
  AVIAN_ERR_NONFINITE rather than writing an infinity. A sum can
  overflow even when every single element is finite; that case
  is checked and rejected. Verified by test.
- Underflow: an accumulation that rounds to zero or a subnormal
  is a finite result and is written normally.

Tolerance (f32 vs the CBQN reference)
-------------------------------------
CBQN computes in double precision (f64); the C kernel computes
in single precision (f32) with plain additions. The fixtures
in fixtures/arrays/ use values exactly representable in f32
(small integers), and every partial sum is an integer below
2^24, so every addition is exact in f32. The CBQN-vs-C
comparison in the test program is therefore bit-exact for those
fixtures: out[s] == expected[s], no tolerance.

For a general f32 input the correct comparison is: the C result
must equal the exact (infinite-precision) left-to-right sum,
rounded to the nearest single-precision value (ties to even)
after each addition. The CBQN (f64) result is the exact sum
rounded to f64 after each addition, which differs from the
exact sum by far less than half an f32 ULP for values in the
f32 range, so the C result is either equal to the f64 result
cast to float, or differs from it by at most 1 ULP of the f32
result per addition step. The test program does not assert a
tolerance on arbitrary inputs; it asserts exact equality on the
exact fixtures. Any future fixture with inexact values must
state its tolerance explicitly.

CBQN formatting note
--------------------
CBQN writes negative numbers with the high-minus character U+00AF
(UTF-8 bytes 0xC2 0xAF) instead of ASCII '-'. The fixture files
therefore contain that byte sequence for negative values. The test
program normalizes it to '-' before parsing. This is a formatting
artifact only; the values are unchanged. The CBQN reference itself
is unaffected: it computes with the true negative values.

Verification status (explicit boundaries, not claims)
-----------------------------------------------------
- CBQN reference: implemented and run. cbqn/reduce_segments.bqn
  generates the fixtures; the test program compares the C output
  against them. Status: VERIFIED by execution (this VM, 2026-11-19).
- C11 CPU kernel: implemented and tested.
  src/c11/test_avian_reduce.c covers empty input, empty segments,
  singleton, hand-computed cross-checks on inputs not in any
  fixture, the CBQN-vs-C comparison, and every error path.
  Status: VERIFIED by execution (this VM, 2026-11-19).
- Cryptol specification: NOT WRITTEN. The Cryptol toolchain is not
  installed on this VM. No Cryptol spec exists for this kernel.
  Status: UNVERIFIED (toolchain missing).
- SAW verification: NOT RUN. The SAW toolchain is not installed on
  this VM. No SAWScript, no proof log, no equivalence result exists
  for this kernel. Status: UNVERIFIED (toolchain missing).
- Vulkan / SPIR-V equivalence: the shader is written and validated
  (see vulkan/shaders/CONTRACT_reduce.md), but it is NOT executed
  and no CPU-vs-GPU comparison has been run. There is no GPU on
  this VM. Status: UNVERIFIED (no GPU).
- q geometry, Self objects, Ruby orchestration, Mercury gates: NOT
  PART of this contract. They are separate layers with their own
  contracts. The q, Self, and Mercury toolchains are not installed
  on this VM; those layers are documented as unavailable, not
  substituted.

Reproduction
------------
From src/c11/: `make test` regenerates the fixtures from CBQN,
builds the kernels and the test programs with -Wall -Wextra
-Werror -O2 -std=c11, and runs the tests. It exits 0 only when
every check passes.
