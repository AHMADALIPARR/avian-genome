Numerical contract — avian_complex_rotate_f32 (C11 kernel ABI)
================================================================

Scope
-----
This contract covers the complex rotation kernel in
src/c11/avian_complex.c: avian_complex_rotate_f32. It states its
exact semantics, its error behaviour, and the limits of what is
verified here. This is the quantum-layer rotation: it rotates
complex amplitudes.

Semantics
---------
For each complex number i in [0, n):
  out_re[i] = in_re[i]*rot_re - in_im[i]*rot_im
  out_im[i] = in_re[i]*rot_im + in_im[i]*rot_re
The input is N complex numbers, stored flat as 2N f32 values in
interleaved order (re0, im0, re1, im1, ...). The output is N
complex numbers, stored flat the same way (2N values). The
caller passes n, the complex count.

The rotation is given directly as a complex number
rot = (rot_re, rot_im), NOT as an angle. The caller is
responsible for forming the rotation (for example from cos and
sin, or from a unit-magnitude complex number); the kernel only
multiplies. This keeps the C and CBQN implementations bit-exact
on exact values: no cos/sin is evaluated inside the kernel, so
there is no cross-implementation transcendental difference to
reconcile. The kernel does not require rot to have unit
magnitude: any finite complex number is a valid rotation (a
non-unit rotation also scales). A zero rotation maps every
input to (0, 0).

Each output component is accumulated with fused multiply-add
(fmaf), in the fixed order shown above. The inner product is
computed first (rounded once), then the outer fused
multiply-add combines the other product with that rounded
inner product:
  out_re = fmaf(in_re, rot_re, fmaf(-in_im, rot_im, 0.0f));
  out_im = fmaf(in_re, rot_im, fmaf(in_im, rot_re, 0.0f));
A fused multiply-add computes a*b + c with a single rounding
(round-to-nearest-even). The inner fused multiply-add has a
zero addend, so it is exactly the correctly-rounded product.
The accumulation order is fixed, so the kernel is
deterministic: the same inputs always give the same outputs
and the same status code.

There is no cross-element reduction: output number i depends
only on input number i and the rotation.

Preconditions and checks (deterministic order)
----------------------------------------------
1. n == 0 is valid. The kernel returns AVIAN_OK and writes
   nothing, even if a data pointer is NULL.
2. If n > SIZE_MAX/2, 2*n cannot be represented and the kernel
   returns AVIAN_ERR_COUNT. The output cannot hold 2*n values.
3. If n > 0, input.data and output.data must both be non-NULL,
   else AVIAN_ERR_NULL.
4. output.count >= 2*n and input.count >= 2*n, else
   AVIAN_ERR_COUNT.
5. The written output range [output.data, output.data + 2*n)
   must not overlap the read input range
   [input.data, input.data + 2*n), else AVIAN_ERR_OVERLAP.
   In-place rotation (output.data == input.data) is rejected.
6. Both rotation components must be finite, every input
   component must be finite, and every product and every
   accumulation must be finite, else AVIAN_ERR_NONFINITE. The
   accumulation is checked after each fused multiply-add. A
   non-finite intermediate makes the final accumulation
   non-finite (infinity plus a finite value stays infinity;
   infinity plus minus-infinity is NaN), so checking after each
   step is sufficient. Every input component is checked
   explicitly; a non-finite component would also make its
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
- Signed zero: a zero output component is +0.0f. The inner
  product of a zero is +0.0f (a zero product plus the +0.0f
  addend: a product of -0.0f plus +0.0f is +0.0f), and the
  outer fused multiply-add of a zero product plus +0.0f is
  +0.0f, so the sign of a zero input component or a zero
  rotation component does not survive the sum. A zero input
  with the identity rotation, with rotation by i, and with
  rotation by -1 all give (+0.0f, +0.0f); an input of negative
  zeros gives the same. Verified by test. A non-zero result
  keeps its sign: a negative real part through the identity
  rotation is still negative. Verified by test.
- NaN: never accepted as a rotation component or an input
  component, and never produced. A NaN rotation, a NaN input,
  or an accumulation that produces NaN returns
  AVIAN_ERR_NONFINITE.
- Infinity: never accepted as a rotation component or an input
  component, and never produced. An infinite rotation, an
  infinite input, a product that overflows to infinity, or a
  sum that overflows to infinity returns AVIAN_ERR_NONFINITE.
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

For a general f32 input the C result and the CBQN result can
differ, and the difference is NOT a bug in either side: it is
the specified accumulation order. The C kernel rounds the inner
product to f32 first, then performs one fused multiply-add of
the other product plus that rounded inner product. The CBQN
reference keeps the exact inner product (f64) and rounds the
exact sum once. The two therefore differ by at most 1 ULP of
the f32 result on inexact values.

Concrete case (found by a probe that searched random f32
inputs, verified against long double exact arithmetic):
  input (-0.753165364, -2.34717751),
  rotation (0.991736054, -1.76565647).
  C kernel (two-step fused multiply-add):
    (-4.89125013, -0.997949183)
  CBQN reference (exact sum, parsed to f32):
    (-4.89125061, -0.997949243)
  Both components differ by exactly 1 ULP. The test program
  pins the kernel to the two-step value on this case (computed
  with independent double arithmetic) and asserts it differs
  from the exact-sum value, so the distinction is exercised
  rather than hidden. The fixtures do not contain this case:
  they use exact values only, so the fixture comparison stays
  bit-exact.

For a general f32 input the correct comparison is: the C result
must equal the exact sum of the two products with the inner
product rounded to f32 first, rounded once to the nearest
single-precision value (ties to even). The test program does
not assert a tolerance on arbitrary inputs; it asserts exact
equality on the exact fixtures and on the fused multiply-add
order case above. Any future fixture with inexact values must
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
and does not pick rows. It extracts the real and imaginary parts
from the flat input with Select (⊏) on computed index lists,
computes the two output components elementwise, and rebuilds the
flat output by interleaving the components number by number.
This is documented in cbqn/complex_rotate.bqn. CBQN has no fused
multiply-add primitive; the reference computes the products and
the sum in f64, which is the exact sum rounded to f64 (see the
tolerance section above for the 1-ULP consequence).

Verification status (explicit boundaries, not claims)
-----------------------------------------------------
- CBQN reference: implemented and run. cbqn/complex_rotate.bqn
  generates the fixtures; the test program compares the C output
  against them. Status: VERIFIED by execution (this VM, 2026-11-19).
- C11 CPU kernel: implemented and tested. src/c11/test_avian_complex.c
  covers empty input, singleton, hand-computed cross-checks on
  inputs not in any fixture (including rotation by the imaginary
  unit and rotation by -1), the fused multiply-add order case,
  the CBQN-vs-C comparison, signed zero, and every error path.
  Status: VERIFIED by execution (this VM, 2026-11-19).
- Cryptol specification: NOT WRITTEN. The Cryptol toolchain is not
  installed on this VM. No Cryptol spec exists for this kernel.
  Status: UNVERIFIED (toolchain missing).
- SAW verification: NOT RUN. The SAW toolchain is not installed on
  this VM. No SAWScript, no proof log, no equivalence result exists
  for this kernel. Status: UNVERIFIED (toolchain missing).
- Vulkan / SPIR-V equivalence: the shader is written and validated
  (see vulkan/shaders/CONTRACT_complex.md), but it is NOT executed
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
