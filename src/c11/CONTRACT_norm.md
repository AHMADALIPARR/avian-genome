Numerical contract — avian_normalize_f32 (C11 kernel ABI)
================================================================

Scope
-----
This contract covers the normalization kernel in
src/c11/avian_norm.c: avian_normalize_f32. It states its exact
semantics, its error behaviour, and the limits of what is
verified here. This is the L2 normalization to a unit vector:
it maps a vector to the unit vector in the same direction.

Semantics
---------
For each element i in [0, n):
  out[i] = in[i] / norm,  norm = sqrt(sum of in[i]^2).
The input is N f32 values. The output is N f32 values. The
caller passes n, the element count.

The norm is computed internally. The sum of squares is
accumulated left to right with fused multiply-add (fmaf),
starting from +0.0f:
  acc = fmaf(in[i], in[i], acc).
The norm is sqrtf(acc), the correctly-rounded f32 square root.
Each output is one correctly-rounded f32 division,
out[i] = in[i] / norm. A fused multiply-add computes a*b + c
with a single rounding (round-to-nearest-even). The
accumulation order is fixed, so the kernel is deterministic:
the same inputs always give the same outputs and the same
status code.

The division cannot overflow: every |in[i]| is at most the
norm (the sum of squares is at least in[i]^2), so every
|out[i]| is at most 1. The norm is checked finite and nonzero
before anything is written, so no division by zero and no
infinite result is ever produced.

Preconditions and checks (deterministic order)
----------------------------------------------
1. n == 0 is valid. The kernel returns AVIAN_OK and writes
   nothing, even if a data pointer is NULL.
2. If n > 0, input.data and output.data must both be non-NULL,
   else AVIAN_ERR_NULL.
3. output.count >= n and input.count >= n, else
   AVIAN_ERR_COUNT.
4. The written output range [output.data, output.data + n)
   must not overlap the read input range
   [input.data, input.data + n), else AVIAN_ERR_OVERLAP.
   In-place normalization (output.data == input.data) is
   rejected.
5. Every input element must be finite, else
   AVIAN_ERR_NONFINITE.
6. The sum of squares must be finite, else
   AVIAN_ERR_NONFINITE. A sum that overflows f32 (the norm
   would be +inf) is rejected here. The sum of squares is a
   sum of non-negative terms, so it cannot become NaN from
   finite inputs; it can only overflow to +inf.
7. The norm must be finite, else AVIAN_ERR_NONFINITE. The
   norm is the square root of a finite non-negative sum, so it
   is finite whenever the sum is; the check names the cause.
8. The norm must be nonzero, else AVIAN_ERR_COUNT (a domain
   error: the input does not satisfy the precondition that its
   norm is a representable positive f32). This happens when
   the input is all zeros, and also when every input is nonzero
   but so small that the sum of squares underflows f32 to
   +0.0f (each square is below the smallest f32 subnormal,
   about 1.4e-45). In both cases the f32-computable norm is
   zero and the division is undefined. Both cases are verified
   by test.
9. Every quotient in[i] / norm must be finite, else
   AVIAN_ERR_NONFINITE. This cannot happen once the norm is
   finite and nonzero (every |in[i]| is at most the norm), but
   the check proves it rather than assuming it.

When any check fails the kernel writes nothing to the output
buffer and returns the error code. The check order above is
fixed, so when several preconditions fail at once the returned
code is the one from the earliest check. The kernel allocates
nothing.

Floating-point behaviour (f32)
-------------------------------
- Rounding: round-to-nearest-even (the default IEEE 754 mode),
  one rounding per fused multiply-add, one per sqrtf, one per
  division.
- Signed zero: a zero input component divided by the
  (positive) norm keeps its sign: +0.0f / norm is +0.0f and
  -0.0f / norm is -0.0f (IEEE division sign rules). This
  differs from the other kernels, where the accumulation from
  +0.0f forces a zero result to +0.0f; here the output is a
  direct division, so the sign of a zero input survives.
  Verified by test. A nonzero result keeps its sign.
- NaN: never accepted as an input, and never produced. A NaN
  input, or a sum or norm that becomes NaN, returns
  AVIAN_ERR_NONFINITE.
- Infinity: never accepted as an input, and never produced.
  An infinite input, a sum of squares that overflows to
  infinity, or a norm that becomes infinite returns
  AVIAN_ERR_NONFINITE.
- Subnormals: an input subnormal is finite and is accepted;
  the accumulation is the correctly-rounded single-precision
  result (it may be a subnormal or zero). A vector of tiny
  inputs whose squares all underflow f32 to +0.0f has a
  f32-computable norm of zero and is rejected as
  AVIAN_ERR_COUNT (see check 8), not as a non-finite value.
  This follows IEEE 754 and is not specially cased beyond the
  zero-norm rule.
- Overflow: a sum of squares whose magnitude exceeds FLT_MAX
  returns AVIAN_ERR_NONFINITE rather than writing an infinity.
  Verified by test (1e20^2 = 1e40 overflows). A large but
  finite sum is accepted: 1e19^2 = 1e38 is below FLT_MAX, the
  norm is 1e19, and the output is ⟨1, 0⟩. Verified by test.
- Underflow: an accumulation that rounds to zero or a subnormal
  is a finite result and is written normally, except that a
  sum of squares that underflows all the way to +0.0f makes
  the norm zero and is rejected as AVIAN_ERR_COUNT (check 8).

Tolerance (f32 vs the CBQN reference)
-------------------------------------
CBQN computes in double precision (f64); the C kernel computes
in single precision (f32) with fused multiply-add and sqrtf.
The fixtures in fixtures/arrays/ use values whose norm is
exact (a perfect-square sum of squares), so the CBQN-vs-C
comparison in the test program is bit-exact for those fixtures:
out[i] == expected[i], no tolerance.

For a general f32 input the C result and the CBQN result can
differ, and the difference is NOT a bug in either side: it is
the specified arithmetic. The C kernel's norm is sqrtf (the
correctly-rounded f32 square root), while the reference's norm
is the f64 square root (essentially exact). The f32 norm
differs from the exact norm by up to half an f32 ULP, and the
division then rounds a quotient computed from the f32-rounded
norm, while the reference rounds a quotient computed from the
(essentially exact) f64 norm. When the exact quotient sits
within ~1 f32 ULP of a rounding boundary, the two land on
adjacent f32 values, differing by exactly 1 ULP.

Concrete case (found by a probe, verified against long double
exact arithmetic):
  input vector 0..99.
  The sum of squares is exactly 328350 in both paths, and
  sqrtf(328350) = (float)sqrt(328350.0) = 0x440f412c, which is
  1.376745e-05 SMALLER than the exact sqrt (long double).
  The exact quotient for element 1 sits 5.78e-12 below the
  midpoint between the two adjacent f32 values 0x3ae4bd58 and
  0x3ae4bd59, while the quotient by the slightly smaller f32
  norm sits 4.19e-11 above the exact quotient, i.e. above the
  midpoint.
  C kernel (f32 norm, f32 division):
    element 1: 0.00174514495 (0x3ae4bd59)
    element 2: 0.00349028991 (0x3b64bd59)
  CBQN reference (f64 norm, f64 division, parsed to f32):
    element 1: 0.00174514484 (0x3ae4bd58)
    element 2: 0.00349028967 (0x3b64bd58)
  Both elements differ by exactly 1 ULP. The test program
  pins the kernel to the f32-norm value on this case (computed
  with independent double arithmetic) and asserts it differs
  from the f64-norm value, so the distinction is exercised
  rather than hidden. The fixtures do not contain this case:
  they use exact norms only, so the fixture comparison stays
  bit-exact.

For a general f32 input the correct comparison is: the C
result must equal the exact quotient of each input by the
f32-rounded norm, rounded once to the nearest
single-precision value (ties to even). The test program does
not assert a tolerance on arbitrary inputs; it asserts exact
equality on the exact fixtures and on the double-rounding case
above. Any future fixture with inexact norms must state its
tolerance explicitly.

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
The reference computes the sum of squares with (+´ x×x), the
square root with √, and the division with x÷n, all in f64.
CBQN has no fused multiply-add primitive and no single-precision
square root; the reference keeps the exact sum and the exact
norm in f64 (see the tolerance section above for the 1-ULP
consequence). The fixture format is a sequence of records,
back to back until end of file; each record is N, then N input
lines, then N expected lines. This is documented in
cbqn/normalize.bqn. BQN evaluates right to left with no
operator precedence, so the sum of squares is written with the
product parenthesized by the fold, and the division is x÷n
with n the scalar norm.

Verification status (explicit boundaries, not claims)
-----------------------------------------------------
- CBQN reference: implemented and run. cbqn/normalize.bqn
  generates the fixtures; the test program compares the C output
  against them. Status: VERIFIED by execution (this VM, 2026-11-19).
- C11 CPU kernel: implemented and tested. src/c11/test_avian_norm.c
  covers empty input, singleton, hand-computed cross-checks on
  inputs not in any fixture (including ⟨3, 4⟩ to ⟨0.6, 0.8⟩
  and the already-unit vector ⟨1, 0, 0⟩), the double-rounding
  case, the CBQN-vs-C comparison, signed zero, and every error
  path (including the zero-vector and underflow domain errors
  and the overflow rejection). Status: VERIFIED by execution
  (this VM, 2026-11-19).
- Cryptol specification: NOT WRITTEN. The Cryptol toolchain is not
  installed on this VM. No Cryptol spec exists for this kernel.
  Status: UNVERIFIED (toolchain missing).
- SAW verification: NOT RUN. The SAW toolchain is not installed on
  this VM. No SAWScript, no proof log, no equivalence result exists
  for this kernel. Status: UNVERIFIED (toolchain missing).
- Vulkan / SPIR-V equivalence: the shader is written and validated
  (see vulkan/shaders/CONTRACT_norm.md), but it is NOT executed
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
