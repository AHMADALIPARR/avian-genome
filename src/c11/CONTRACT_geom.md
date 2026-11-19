Numerical contract — avian_geom_transform_f32 (C11 kernel ABI)
================================================================

Scope
-----
This contract covers the genomic coordinate transform kernel in
src/c11/avian_geom.c: avian_geom_transform_f32. It states its
exact semantics, its error behaviour, and the limits of what is
verified here. This is the geometry layer: it maps genomic
coordinates to spatial positions.

Semantics
---------
For each point i in [0, n) and each row r in 0..2:
  out[3i+r] = M[3r+0]*in[3i+0] + M[3r+1]*in[3i+1]
            + M[3r+2]*in[3i+2] + t[r]
The matrix M is 9 f32, row-major: M[0..2] is row 0, M[3..5] is
row 1, M[6..8] is row 2. The translation t is 3 f32. The input
is N points, each 3 f32 (x, y, z), stored flat (3N values).
The output is N points, each 3 f32, stored flat (3N values).
The caller passes n, the point count.

Each output component is accumulated left to right with fused
multiply-add (fmaf), starting from +0.0f, in the order shown
above (the matrix row term by term, then the translation):
  acc = fmaf(M[3r+0], x, 0.0f);
  acc = fmaf(M[3r+1], y, acc);
  acc = fmaf(M[3r+2], z, acc);
  acc = fmaf(1.0f, t[r], acc);
The translation step is a fused multiply-add by 1.0f, which is
exactly the correctly-rounded sum t[r] + acc (1.0f * t[r] is
exact), so it is the same as a plain addition. A fused
multiply-add computes a*b + c with a single rounding
(round-to-nearest-even), so each term contributes one
correctly-rounded multiply-add.

There is no cross-point reduction: output point i depends only
on input point i, the matrix, and the translation. The kernel
is deterministic: the same inputs always give the same outputs
and the same status code.

Preconditions and checks (deterministic order)
----------------------------------------------
1. n == 0 is valid. The kernel returns AVIAN_OK and writes
   nothing, even if a data pointer is NULL.
2. If n > SIZE_MAX/3, 3*n cannot be represented and the kernel
   returns AVIAN_ERR_COUNT. The output cannot hold 3*n values.
3. If n > 0, input.data, output.data, matrix.data, and
   translation.data must all be non-NULL, else AVIAN_ERR_NULL.
4. output.count >= 3*n and input.count >= 3*n, and
   matrix.count >= 9 and translation.count >= 3, else
   AVIAN_ERR_COUNT.
5. The written output range [output.data, output.data + 3*n)
   must not overlap the read input range
   [input.data, input.data + 3*n), the read matrix range
   [matrix.data, matrix.data + 9), or the read translation range
   [translation.data, translation.data + 3), else
   AVIAN_ERR_OVERLAP. In-place transform
   (output.data == input.data) is rejected, and writing into
   the matrix or the translation buffer is rejected.
6. Every input coordinate, every matrix entry, and every
   translation entry must be finite, and every product and
   every accumulation must be finite, else AVIAN_ERR_NONFINITE.
   The accumulation is checked after each fused multiply-add.
   A non-finite intermediate makes the final accumulation
   non-finite (infinity plus a finite value stays infinity;
   infinity plus minus-infinity is NaN), so checking after each
   step is sufficient. Every input coordinate is checked
   explicitly; a non-finite coordinate would also make its
   accumulation non-finite, but the explicit check names the
   cause.

When any check fails the kernel writes nothing to the output
buffer and returns the error code. The check order above is
fixed, so when several preconditions fail at once the returned
code is the one from the earliest check. The kernel allocates
nothing.

Floating-point behaviour (f32)
-------------------------------
- Rounding: round-to-nearest-even (the default IEEE 754 mode),
  one rounding per fused multiply-add.
- Signed zero: a zero output is +0.0f. The accumulation starts
  from +0.0f, and adding a zero of either sign to +0.0f gives
  +0.0f, so the sign of a zero input coordinate, a zero matrix
  entry, or a zero translation entry does not survive the sum.
  A zero point with the identity matrix and zero translation
  gives (+0.0f, +0.0f, +0.0f), a point of negative zeros gives
  the same, and a negative-zero translation gives the same.
  Verified by test. A non-zero result keeps its sign: a
  negative coordinate through the identity matrix is still
  negative. Verified by test.
- NaN: never accepted as an input coordinate, a matrix entry,
  or a translation entry, and never produced. A NaN input, a
  NaN matrix entry, a NaN translation entry, or an accumulation
  that produces NaN returns AVIAN_ERR_NONFINITE.
- Infinity: never accepted as an input coordinate, a matrix
  entry, or a translation entry, and never produced. An infinite
  input, an infinite matrix entry, an infinite translation entry,
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
must equal the exact (infinite-precision) sum of the four terms,
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

CBQN reference note
-------------------
Pick (⊑) with a scalar index does not work on a multi-dimensional
array in this CBQN build, so the reference does not form a matrix
and does not pick rows. It extracts the x, y, z coordinates from
the flat input with Select (⊏) on computed index lists, computes
the three output components elementwise, and rebuilds the flat
output by interleaving the components point by point. This is
documented in cbqn/geom_transform.bqn.

Verification status (explicit boundaries, not claims)
-----------------------------------------------------
- CBQN reference: implemented and run. cbqn/geom_transform.bqn
  generates the fixtures; the test program compares the C output
  against them. Status: VERIFIED by execution (this VM, 2026-11-19).
- C11 CPU kernel: implemented and tested. src/c11/test_avian_geom.c
  covers empty input, singleton, hand-computed cross-checks on
  inputs not in any fixture (including the identity transform and
  a pure translation), the CBQN-vs-C comparison, signed zero, and
  every error path. Status: VERIFIED by execution (this VM,
  2026-11-19).
- Cryptol specification: NOT WRITTEN. The Cryptol toolchain is not
  installed on this VM. No Cryptol spec exists for this kernel.
  Status: UNVERIFIED (toolchain missing).
- SAW verification: NOT RUN. The SAW toolchain is not installed on
  this VM. No SAWScript, no proof log, no equivalence result exists
  for this kernel. Status: UNVERIFIED (toolchain missing).
- Vulkan / SPIR-V equivalence: the shader is written and validated
  (see vulkan/shaders/CONTRACT_geom.md), but it is NOT executed
  and no CPU-vs-GPU comparison has been run. There is no GPU on
  this VM. Status: UNVERIFIED (no GPU).
- q geometry, Self objects, Ruby orchestration, Mercury gates: NOT
  PART of this contract. They are separate layers with their own
  contracts. The q, Self, and Mercury toolchains are not installed
  on this VM; those layers are documented as unavailable, not
  substituted. The q geometry layer itself is documented as a
  boundary in q/BOUNDARY.md; this kernel is the C11/Vulkan
  implementation of the transform, not a q program.

Reproduction
------------
From src/c11/: `make test` regenerates the fixtures from CBQN,
builds the kernels and the test programs with -Wall -Wextra
-Werror -O2 -std=c11, and runs the tests. It exits 0 only when
every check passes.
