Numerical contract — avian_scale (C11 kernel ABI)
=================================================

Scope
-----
This contract covers the two scale kernels in src/c11/avian_scale.c:
avian_scale_f32 and avian_scale_i32. It states their exact semantics,
their error behaviour, and the limits of what is verified here.

Semantics
---------
For each i in [0, input.count):
  f32: output[i] = input[i] * scale, one correctly-rounded IEEE 754
       single-precision multiply.
  i32: output[i] = input[i] * scale, exact integer arithmetic.

There is no cross-element reduction, so element order does not affect
the result. The kernel is deterministic: the same inputs always give
the same outputs and the same status code.

Preconditions and checks (deterministic order)
----------------------------------------------
1. count == 0 is valid. The kernel returns AVIAN_OK and writes nothing,
   even if a data pointer is NULL.
2. If count > 0, input.data and output.data must be non-NULL, else
   AVIAN_ERR_NULL.
3. output.count >= input.count, else AVIAN_ERR_COUNT.
4. The written output range [output.data, output.data + input.count)
   must not overlap the read input range
   [input.data, input.data + input.count), else AVIAN_ERR_OVERLAP.
   In-place scaling (output.data == input.data) is rejected.
5. f32: scale and every input element must be finite, and every
   product must be finite, else AVIAN_ERR_NONFINITE.
6. i32: every product must fit in int32, else AVIAN_ERR_OVERFLOW.
   The product is formed in 64 bits and checked before the narrowing
   store, so signed overflow is impossible in this kernel.

When any check fails the kernel writes nothing to the output buffer
and returns the error code. The check order above is fixed, so when
several preconditions fail at once the returned code is the one from
the earliest check. The kernel allocates nothing.

Floating-point behaviour (f32)
-------------------------------
- Rounding: round-to-nearest-even (the default IEEE 754 mode).
- Signed zero: preserved. 0.0 * scale is +0.0 and -0.0 * scale is -0.0
  for finite scale. Verified by test.
- NaN: never accepted as input and never produced. A NaN input or a
  NaN scale returns AVIAN_ERR_NONFINITE.
- Infinity: never accepted as input and never produced. An infinite
  input, an infinite scale, or a product that overflows to infinity
  returns AVIAN_ERR_NONFINITE.
- Subnormals: an input subnormal is finite and is accepted; the product
  is the correctly-rounded single-precision result (it may be a
  subnormal or zero). This follows IEEE 754 and is not specially cased.
- Overflow: a product whose magnitude exceeds FLT_MAX returns
  AVIAN_ERR_NONFINITE rather than writing an infinity.
- Underflow: a product that rounds to zero or a subnormal is a finite
  result and is written normally.

Tolerance (f32 vs the CBQN reference)
-------------------------------------
CBQN computes in double precision (f64); the C kernel computes in
single precision (f32). The fixtures in fixtures/arrays/ use products
that are exactly representable in f32 (small integers times 2.5 and
0.5), so the CBQN-vs-C comparison in the test program is bit-exact
for those fixtures: out[i] == expected[i], no tolerance.

For a general f32 input the correct comparison is: the C result must
equal the CBQN (f64) product rounded to the nearest single-precision
value (ties to even). In practice that is the f64 result cast to float.
Where the f64 product is exactly representable in f32, the two agree
bit-exactly. Where it is not, the C result is the correctly-rounded
f32 value and may differ from the f64 value by at most 1 ULP of the
f32 result. The test program does not assert a tolerance on arbitrary
inputs; it asserts exact equality on the exact fixtures. Any future
fixture with inexact products must state its tolerance explicitly.

Integer behaviour (i32)
------------------------
- Exact. The CBQN reference uses exact integer arithmetic and the C
  kernel uses a 64-bit intermediate with a range check, so the
  comparison is bit-exact: out[i] == expected[i], no tolerance.
- Overflow is an error, not wrapping. There is no unchecked i32 scale
  kernel in this ABI.
- INT32_MIN * -1 overflows and returns AVIAN_ERR_OVERFLOW.
- INT32_MAX * 1 and INT32_MIN * 1 fit and are written exactly.

CBQN formatting note
--------------------
CBQN writes negative numbers with the high-minus character U+00AF
(UTF-8 bytes 0xC2 0xAF) instead of ASCII '-'. The fixture files
therefore contain that byte sequence for negative values. The test
program normalizes it to '-' before parsing. This is a formatting
artifact only; the values are unchanged. The CBQN reference itself is
unaffected: it computes with the true negative values.

Verification status (explicit boundaries, not claims)
-----------------------------------------------------
- CBQN reference: implemented and run. cbqn/scale.bqn generates the
  fixtures; the test program compares the C output against them.
  Status: VERIFIED by execution (this VM, 2026-11-19).
- C11 CPU kernel: implemented and tested. src/c11/test_avian_scale.c
  covers empty input, singleton, normal and large arrays, the
  CBQN-vs-C comparison, and every error path. Status: VERIFIED by
  execution (this VM, 2026-11-19).
- Cryptol specification: NOT WRITTEN. The Cryptol toolchain is not
  installed on this VM. No Cryptol spec exists for these kernels.
  Status: UNVERIFIED (toolchain missing).
- SAW verification: NOT RUN. The SAW toolchain is not installed on
  this VM. No SAWScript, no proof log, no equivalence result exists
  for these kernels. Status: UNVERIFIED (toolchain missing).
- Vulkan / SPIR-V equivalence: NOT TESTED. There is no GPU on this VM.
  No shader has been written for these kernels and no CPU-vs-GPU
  comparison has been run. Status: UNVERIFIED (no GPU).
- q geometry, Self objects, Ruby orchestration, Mercury gates: NOT
  PART of this contract. They are separate layers with their own
  contracts. The q, Self, and Mercury toolchains are not installed on
  this VM; those layers are documented as unavailable, not substituted.

Reproduction
------------
From src/c11/: `make test` regenerates the fixtures from CBQN, builds
the kernel and the test program with -Wall -Wextra -Werror -O2
-std=c11, and runs the tests. It exits 0 only when every check passes.
