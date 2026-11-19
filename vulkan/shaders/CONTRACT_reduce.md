Numerical contract — AVIAN-MORPH Vulkan segmented-reduction shader
====================================================================

Scope
-----
This contract covers the compute shader in
vulkan/shaders/reduce_segments.comp. It mirrors the C11 kernel
avian_reduce_segments_f32 in src/c11/avian_reduce.c. The invariant
is:

  K_CBQN(x) == K_C(x) == K_Vulkan(x)

where K_CBQN is the CBQN reference, K_C is the C11 kernel, and
K_Vulkan is the shader. The first equality is verified by the
C11 test suite (71 checks, 0 failures, CBQN fixtures regenerated
on every run). The second equality is NOT verified here, because
there is no GPU on this machine. See "Verification boundary" below.

Semantics
---------
For each s in [0, nseg):
  output[s] = sum of input[off[s] .. off[s]+len[s]-1],
accumulated left to right in f32, starting from +0.0f. An empty
segment (length 0) sums to +0.0f. There is no cross-segment
accumulation. Each addition is one correctly-rounded IEEE 754
single-precision add (round-to-nearest-even). The shader is
deterministic: the same inputs always give the same outputs.

Host-side validation (the boundary between host and device)
-----------------------------------------------------------
The shader performs the arithmetic only. The host validates before
dispatch, using exactly the same checks as the C kernel
(see src/c11/CONTRACT_reduce.md):
  1. nseg == 0 and input count == 0 is valid; the host dispatches
     zero workgroups.
  2. Buffers are non-NULL and non-overlapping (the written output
     must not overlap the input or the lengths buffer).
  3. Output holds one sum per segment.
  4. Every segment length is non-negative, and the total of the
     lengths equals the input count.
  5. Every input element is finite, and every accumulation is
     finite.

The host also computes the exclusive prefix sum of the lengths
and uploads it as the offsets buffer. The shader assumes
offsets[s] is the start of segment s and
offsets[s] + lengths[s] <= input count. A dispatch on
unvalidated buffers, or with a wrong offsets buffer, is a host
error, not a shader error. This is the same division of labour
as the C kernel, where the validation pass runs before the
compute pass.

Binding layout (set 0)
----------------------
  binding 0 — readonly input buffer (float data[])
  binding 1 — readonly lengths buffer (int data[])
  binding 2 — readonly offsets buffer (uint data[])
  binding 3 — output buffer (float data[])
  push constant — nseg (uint)

Workgroup size is 256. The host dispatches ceil(nseg / 256)
workgroups. Invocations with index >= nseg return immediately.
One invocation computes exactly one segment's sum.

Verification boundary (what is and is not verified here)
---------------------------------------------------------
VERIFIED (by execution on this machine):
  - The shader compiles to SPIR-V with glslangValidator.
  - The SPIR-V module passes spirv-val validation.
  - The SPIR-V disassembly contains the expected arithmetic
    (f32 addition in a loop, the segment guard, the length and
    offset loads, and the output store).

UNVERIFIED (no GPU on this machine):
  - The shader is not executed. No GPU output is produced.
  - No CPU-vs-GPU conformance comparison is run. The equality
    K_C(x) == K_Vulkan(x) is asserted by construction (the shader
    mirrors the C kernel) but not measured.
  - No SAW or Cryptol verification covers this shader. SAW
    verifies LLVM bitcode, not SPIR-V, and neither toolchain is
    installed here. A SPIR-V backend or clspv route would need
    target-specific validation of its own.

The shader is written and validated. It is not claimed to
have run on a GPU, and no GPU performance or equivalence result
is reported.
