Numerical contract — avian_gather_f32 and avian_scatter_f32 (C11 kernel ABI)
=============================================================================

Scope
-----
This contract covers the two sparse gather/scatter kernels in
src/c11/avian_gather.c: avian_gather_f32 and avian_scatter_f32.
It states their exact semantics, their error behaviour, and the
limits of what is verified here.

Semantics
---------
Gather:
  For each i in [0, index.count):
    output[i] = input[index[i]].
  Output length equals index count. Each index must be a
  non-negative integer less than input.count.

Scatter:
  For each i in [0, input.count):
    output[index[i]] = input[i].
  The output starts as all +0.0f, then each input element is
  written to its index, in input order. If two input elements
  target the same index, the LAST one in input order wins. An
  index that is never written stays +0.0f. Output length is
  given by the caller and must be at least 1 when there is
  input to scatter. Each index must be a non-negative integer
  less than output.count.

Both operations copy values only. There is no arithmetic, so
there is no rounding and no overflow. A copied value is
bit-identical to the input value it copies, including the sign
of zero. Both kernels are deterministic: the same inputs always
give the same outputs and the same status code.

Last-wins and the zero fill (scatter)
--------------------------------------
The scatter writes happen in input order, i = 0, 1, 2, ...,
so when two input elements target the same index, the later
one overwrites the earlier one and the last one in input order
is the value left in the output. This is a defined rule, not
an accident of scheduling: there is exactly one write order
(the input order), and the kernel is single-threaded, so the
result is fully determined. A GPU implementation must preserve
this rule; a parallel scatter with atomic writes does NOT
preserve it, because the winning writer is then unspecified.
See vulkan/shaders/CONTRACT_gather.md for how the shader
preserves last-wins without a race.

The zero fill writes +0.0f (positive zero) to every output
element before any input element is written. An index that is
never written therefore holds +0.0f, never -0.0f and never an
uninitialised value. A scattered -0.0f is written as -0.0f
(the sign of a copied value is preserved); only the fill is
+0.0f. Verified by test.

Preconditions and checks (deterministic order)
----------------------------------------------
Gather:
1. index.count == 0 is valid. The kernel returns AVIAN_OK and
   writes nothing, even if a data pointer is NULL.
2. If index.count > 0, indices.data and output.data must be
   non-NULL, and if input.count > 0, input.data must be
   non-NULL, else AVIAN_ERR_NULL.
3. output.count >= index.count, else AVIAN_ERR_COUNT. The
   output holds one value per index.
4. Every index must satisfy 0 <= index < input.count, else
   AVIAN_ERR_COUNT. A negative index is out of range. The
   status enum has no separate invalid-index code, and an
   index is part of the count contract. An index is out of
   range when the input is empty (input.count == 0), because
   every index is then >= input.count.
5. The written output range
   [output.data, output.data + index.count) must not overlap
   the read input range [input.data, input.data + input.count)
   or the read indices range
   [indices.data, indices.data + index.count),
   else AVIAN_ERR_OVERLAP. In-place gather
   (output.data == input.data) is rejected, and writing into
   the indices buffer is rejected: the indices are read
   throughout the computation, so writing into them would
   corrupt the result.
6. Every input element must be finite, else AVIAN_ERR_NONFINITE.
   This covers every element of the input, not only the ones
   that are gathered: the kernel validates the whole input
   buffer. A non-finite element at a position that is never
   gathered is still rejected.

Scatter:
1. input.count == 0 and output.count == 0 is valid. The kernel
   returns AVIAN_OK and writes nothing, even if a data pointer
   is NULL.
2. If input.count > 0, input.data and indices.data must be
   non-NULL; if output.count > 0, output.data must be non-NULL;
   else AVIAN_ERR_NULL.
3. indices.count must equal input.count, else AVIAN_ERR_COUNT.
   Each input element needs exactly one index. (If input.count
   is 0, indices.count must also be 0; the zero fill is the only
   work, and it needs no indices.)
4. Every index must satisfy 0 <= index < output.count, else
   AVIAN_ERR_COUNT. A negative index is out of range. An
   output.count of 0 with input to scatter is caught here,
   because every index is then out of range.
5. The written output range
   [output.data, output.data + output.count) must not overlap
   the read input range [input.data, input.data + input.count)
   or the read indices range
   [indices.data, indices.data + indices.count),
   else AVIAN_ERR_OVERLAP. The written range is the whole
   output, because the zero fill writes every element. In-place
   scatter (output.data == input.data) is rejected, and writing
   into the indices buffer is rejected.
6. Every input element must be finite, else AVIAN_ERR_NONFINITE.

When any check fails the kernel writes nothing to the output
buffer and returns the error code. For scatter, "writes nothing"
includes the zero fill: a failed scatter call leaves the output
buffer untouched, it does not zero it and then fail. The check
order above is fixed, so when several preconditions fail at once
the returned code is the one from the earliest check. The kernels
allocate nothing.

Floating-point behaviour (f32)
-------------------------------
- There is no arithmetic. No rounding, no overflow, no
  underflow, and no subnormal handling beyond copying the value
  as it is.
- Signed zero: preserved by copying. Gathering -0.0f gives
  -0.0f, gathering +0.0f gives +0.0f. Scattering -0.0f writes
  -0.0f, scattering +0.0f writes +0.0f. The scatter zero fill
  writes +0.0f. Verified by test.
- NaN: never accepted as input and never produced. A NaN input
  returns AVIAN_ERR_NONFINITE.
- Infinity: never accepted as input and never produced. An
  infinite input returns AVIAN_ERR_NONFINITE.
- Subnormals: an input subnormal is finite and is accepted; it
  is copied exactly.

Tolerance (f32 vs the CBQN reference)
-------------------------------------
CBQN computes in double precision (f64); the C kernels copy
single-precision values. Gather and scatter copy values only,
so the CBQN-vs-C comparison in the test program is bit-exact
for any finite input: out[i] == expected[i], no tolerance.
The fixtures use small integers, which are exactly
representable in f32, and the comparison is exact.

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
- CBQN reference: implemented and run. cbqn/gather_scatter.bqn
  generates the fixtures; the test program compares the C output
  against them. Status: VERIFIED by execution (this VM, 2026-11-19).
- C11 CPU kernel: implemented and tested.
  src/c11/test_avian_gather.c covers empty input, empty output,
  singleton, the zero fill, last-wins duplicates, unwritten
  indices, the gather-then-scatter round trip, hand-computed
  cross-checks on inputs not in any fixture, the CBQN-vs-C
  comparison, and every error path. Status: VERIFIED by
  execution (this VM, 2026-11-19).
- Cryptol specification: NOT WRITTEN. The Cryptol toolchain is not
  installed on this VM. No Cryptol spec exists for these kernels.
  Status: UNVERIFIED (toolchain missing).
- SAW verification: NOT RUN. The SAW toolchain is not installed on
  this VM. No SAWScript, no proof log, no equivalence result exists
  for these kernels. Status: UNVERIFIED (toolchain missing).
- Vulkan / SPIR-V equivalence: the shader is written and validated
  (see vulkan/shaders/CONTRACT_gather.md), but it is NOT executed
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
