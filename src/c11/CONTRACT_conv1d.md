Numerical contract — avian_conv1d_f32 (C11 kernel ABI)
========================================================

Scope
-----
This contract covers the receptive-field convolution kernel in
src/c11/avian_conv1d.c: avian_conv1d_f32. It states its exact
semantics, its error behaviour, and the limits of what is
verified here.

Semantics
---------
For each i in [0, input.count):
  output[i] = k0*input[i-1] + k1*input[i] + k2*input[i+1]
where out-of-range inputs (i-1 < 0 or i+1 >= count) read as 0
(zero padding). Output length equals input length.

Each tap is a fused multiply-add (fmaf), accumulated left to
right in the fixed order k0, then k1, then k2:
  acc = fmaf(k0, xm, 0.0f);
  acc = fmaf(k1, x0, acc);
  acc = fmaf(k2, xp, acc);
where xm is input[i-1] (or 0.0f at i == 0), x0 is input[i], and
xp is input[i+1] (or 0.0f at i == count-1). A fused multiply-add
computes k*x + acc with a single rounding (round-to-nearest-even),
so each tap contributes one correctly-rounded multiply-add.

There is no cross-element reduction beyond the 3-tap window:
output[i] depends only on input[i-1], input[i], and input[i+1].
The kernel is deterministic: the same inputs always give the same
outputs and the same status code.

Boundary rule (zero padding)
----------------------------
The first output uses only the centre and right taps
(output[0] = k1*input[0] + k2*input[1]); the last output uses
only the left and centre taps
(output[count-1] = k0*input[count-2] + k1*input[count-1]).
For a singleton input, output[0] = k1*input[0] (both neighbours
are padding zeros). The padding value is +0.0f. A window of all
zeros produces +0.0f: the accumulation starts from +0.0f, and
adding a zero of either sign to +0.0f gives +0.0f, so the sign
of a zero input does not survive the sum. Verified by test.

Preconditions and checks (deterministic order)
----------------------------------------------
1. count == 0 is valid. The kernel returns AVIAN_OK and writes
   nothing, even if a data pointer is NULL.
2. If count > 0, input.data and output.data must be non-NULL, else
   AVIAN_ERR_NULL.
3. output.count >= input.count, else AVIAN_ERR_COUNT.
4. The written output range [output.data, output.data + input.count)
   must not overlap the read input range
   [input.data, input.data + input.count), else AVIAN_ERR_OVERLAP.
   In-place convolution (output.data == input.data) is rejected.
5. All three kernel coefficients must be finite, every input
   element must be finite, and every accumulation must be finite,
   else AVIAN_ERR_NONFINITE. The accumulation is checked after
   each fused multiply-add. A non-finite intermediate makes the
   final accumulation non-finite (infinity plus a finite value
   stays infinity; infinity plus minus-infinity is NaN), so
   checking after each step is sufficient. Every input element
   is checked as the centre tap of its own output, so all inputs
   are covered; a non-finite neighbour is also caught because it
   makes the accumulation non-finite.

When any check fails the kernel writes nothing to the output
buffer and returns the error code. The check order above is
fixed, so when several preconditions fail at once the returned
code is the one from the earliest check. The kernel allocates
nothing.

Floating-point behaviour (f32)
-------------------------------
- Rounding: round-to-nearest-even (the default IEEE 754 mode),
  one rounding per fused multiply-add.
- Signed zero: a zero result is +0.0f (see the boundary rule
  above). Verified by test.
- NaN: never accepted as input or as a coefficient, and never
  produced. A NaN input, a NaN coefficient, or an accumulation
  that produces NaN returns AVIAN_ERR_NONFINITE.
- Infinity: never accepted as input or as a coefficient, and
  never produced. An infinite input, an infinite coefficient,
  a product that overflows to infinity, or a sum that overflows
  to infinity returns AVIAN_ERR_NONFINITE.
- Subnormals: an input subnormal is finite and is accepted; the
  accumulation is the correctly-rounded single-precision result
  (it may be a subnormal or zero). This follows IEEE 754 and is
  not specially cased.
- Overflow: a product or a sum whose magnitude exceeds FLT_MAX
  returns AVIAN_ERR_NONFINITE rather than writing an infinity.
  A sum can overflow even when every single product is finite;
  that case is checked and rejected. Verified by test.
- Underflow: an accumulation that rounds to zero or a subnormal
  is a finite result and is written normally.

Tolerance (f32 vs the CBQN reference)
-------------------------------------
CBQN computes in double precision (f64); the C kernel computes
in single precision (f32) with fused multiply-add. The fixtures
in fixtures/arrays/ use values exactly representable in f32
(small integers and halves and quarters), so the CBQN-vs-C
comparison in the test program is bit-exact for those fixtures:
out[i] == expected[i], no tolerance.

For a general f32 input the correct comparison is: the C result
must equal the exact (infinite-precision) sum of the three taps,
rounded once to the nearest single-precision value (ties to
even), because each fmaf rounds the exact product-plus-accumulator
once. The CBQN (f64) result is that exact sum rounded to f64,
which differs from the exact sum by far less than half an f32
ULP for values in the f32 range, so the C result is either equal
to the f64 result cast to float, or differs from it by at most
1 ULP of the f32 result. The test program does not assert a
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
- CBQN reference: implemented and run. cbqn/conv1d.bqn generates
  the fixtures; the test program compares the C output against
  them. Status: VERIFIED by execution (this VM, 2026-11-19).
- C11 CPU kernel: implemented and tested. src/c11/test_avian_conv1d.c
  covers empty input, singleton, the zero-padding boundary,
  hand-computed cross-checks on inputs not in any fixture, the
  CBQN-vs-C comparison, and every error path. Status: VERIFIED
  by execution (this VM, 2026-11-19).
- Cryptol specification: NOT WRITTEN. The Cryptol toolchain is not
  installed on this VM. No Cryptol spec exists for this kernel.
  Status: UNVERIFIED (toolchain missing).
- SAW verification: NOT RUN. The SAW toolchain is not installed on
  this VM. No SAWScript, no proof log, no equivalence result exists
  for this kernel. Status: UNVERIFIED (toolchain missing).
- Vulkan / SPIR-V equivalence: the shader is written and validated
  (see vulkan/shaders/CONTRACT_conv1d.md), but it is NOT executed
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
