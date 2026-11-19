Numerical contract — AVIAN-MORPH Vulkan geom_transform shader
================================================================

Scope
-----
This contract covers the compute shader in
vulkan/shaders/geom_transform.comp. It mirrors the C11 kernel
avian_geom_transform_f32 in src/c11/avian_geom.c. The invariant is:

  K_CBQN(x) == K_C(x) == K_Vulkan(x)

where K_CBQN is the CBQN reference, K_C is the C11 kernel, and
K_Vulkan is the shader. The first equality is verified by the
C11 test suite (CBQN fixtures regenerated on every run). The
second equality is NOT verified here, because there is no GPU
on this machine. See "Verification boundary" below.

Semantics
---------
For each point i in [0, n) and each row r in 0..2:
  out[3i+r] = M[3r+0]*in[3i+0] + M[3r+1]*in[3i+1]
            + M[3r+2]*in[3i+2] + t[r]
The matrix M is 9 f32, row-major. The translation t is 3 f32.
The input is N points, each 3 f32 (x, y, z), stored flat. The
output is N points, each 3 f32, stored flat.

Each output component is a fused multiply-add (fma), accumulated
left to right in the fixed order M row term by term, then the
translation, matching the C kernel. The translation step is a
fused multiply-add by 1.0, which is exactly the correctly-rounded
sum t[r] + acc. Each invocation computes exactly one point (its
3 output coordinates) from its 3 input coordinates, the matrix,
and the translation. There is no cross-point reduction. The
shader is deterministic: the same inputs give the same outputs.

Host-side validation (the boundary between host and device)
-----------------------------------------------------------
The shader performs the arithmetic only. The host validates before
dispatch, using exactly the same checks as the C kernel
(see src/c11/CONTRACT_geom.md):
  1. n == 0 is valid; the host dispatches zero workgroups.
  2. Buffers are non-NULL and non-overlapping. The written output
     range must not overlap the input, the matrix, or the
     translation.
  3. Output and input each hold at least 3*n values, the matrix
     holds at least 9 values, and the translation holds at least
     3 values.
  4. Every input coordinate, every matrix entry, and every
     translation entry is finite, and every accumulation is finite.

A dispatch on unvalidated buffers is a host error, not a shader
error. The shader assumes the preconditions hold. This is the
same division of labour as the C kernel, where the validation
pass runs before the compute pass.

Binding layout (set 0)
----------------------
  binding 0 — readonly input buffer (float data[], 3N values)
  binding 1 — readonly matrix buffer (float data[], 9 values)
  binding 2 — readonly translation buffer (float data[], 3 values)
  binding 3 — output buffer (float data[], 3N values)
  push constant — n (uint, point count)

Workgroup size is 256. The host dispatches ceil(n / 256)
workgroups. Invocations with index >= n return immediately.

Verification boundary (what is and is not verified here)
---------------------------------------------------------
VERIFIED (by execution on this machine):
  - The shader compiles to SPIR-V with glslangValidator.
  - The SPIR-V module passes spirv-val validation.
  - The SPIR-V disassembly contains the expected arithmetic
    (fused multiply-add operations, the point-count guard, the
    input/matrix/translation loads, and the output store).

UNVERIFIED (no GPU on this machine):
  - No shader is executed. No GPU output is produced.
  - No CPU-vs-GPU conformance comparison is run. The equality
    K_C(x) == K_Vulkan(x) is asserted by construction (the shader
    mirrors the C kernel line for line) but not measured.
  - No SAW or Cryptol verification covers this shader. SAW
    verifies LLVM bitcode, not SPIR-V, and neither toolchain is
    installed here. A SPIR-V backend or clspv route would need
    target-specific validation of its own.

The shader is written and validated. It is not claimed to
have run on a GPU, and no GPU performance or equivalence result
is reported.
