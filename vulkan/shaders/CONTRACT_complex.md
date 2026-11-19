Numerical contract — AVIAN-MORPH Vulkan complex_rotate shader
================================================================

Scope
-----
This contract covers the compute shader in
vulkan/shaders/complex_rotate.comp. It mirrors the C11 kernel
avian_complex_rotate_f32 in src/c11/avian_complex.c. The invariant is:

  K_CBQN(x) == K_C(x) == K_Vulkan(x)

where K_CBQN is the CBQN reference, K_C is the C11 kernel, and
K_Vulkan is the shader. The first equality is verified by the
C11 test suite (CBQN fixtures regenerated on every run) on the
exact fixtures. The second equality is NOT verified here,
because there is no GPU on this machine. See "Verification
boundary" below.

Semantics
---------
For each complex number i in [0, n):
  out_re[i] = in_re[i]*rot_re - in_im[i]*rot_im
  out_im[i] = in_re[i]*rot_im + in_im[i]*rot_re
The input is N complex numbers, stored flat as 2N f32 values in
interleaved order (re0, im0, re1, im1, ...). The output is N
complex numbers, stored flat the same way. The rotation is a
single complex number rot = (rot_re, rot_im), given directly,
not as an angle.

Each output component is a fused multiply-add (fma), in the
fixed order of the C kernel: the inner product first, then the
outer fma of the other product plus that inner product.
GLSL fma() maps to the same fused operation as C fmaf()
(OpExtInst Fma in SPIR-V), so the shader's arithmetic matches
the C kernel's bit for bit on exact values, and matches the C
kernel's two-step rounding on inexact values. The shader, like
the C kernel, rounds the inner product first, while the CBQN
reference keeps the exact inner product; on inexact values the
shader and the C kernel agree with each other and can differ
from the CBQN reference by 1 ULP (see
src/c11/CONTRACT_complex.md for the concrete case). Each
invocation computes exactly one complex number (its 2 output
components) from its 2 input components and the rotation. There
is no cross-element reduction. The shader is deterministic: the
same inputs give the same outputs.

Host-side validation (the boundary between host and device)
-----------------------------------------------------------
The shader performs the arithmetic only. The host validates before
dispatch, using exactly the same checks as the C kernel
(see src/c11/CONTRACT_complex.md):
  1. n == 0 is valid; the host dispatches zero workgroups.
  2. Buffers are non-NULL and non-overlapping. The written output
     range must not overlap the input.
  3. Output and input each hold at least 2*n values.
  4. Both rotation components are finite, every input component
     is finite, and every accumulation is finite.

A dispatch on unvalidated buffers is a host error, not a shader
error. The shader assumes the preconditions hold. This is the
same division of labour as the C kernel, where the validation
pass runs before the compute pass.

Binding layout (set 0)
----------------------
  binding 0 — readonly input buffer (float data[], 2N values)
  binding 1 — output buffer (float data[], 2N values)
  push constant — rot_re, rot_im (float), n (uint, complex count)

Workgroup size is 256. The host dispatches ceil(n / 256)
workgroups. Invocations with index >= n return immediately.

Verification boundary (what is and is not verified here)
---------------------------------------------------------
VERIFIED (by execution on this machine):
  - The shader compiles to SPIR-V with glslangValidator.
  - The SPIR-V module passes spirv-val validation.
  - The SPIR-V disassembly contains the expected arithmetic
    (fused multiply-add operations, the complex-count guard,
    the input loads, and the output store).

UNVERIFIED (no GPU on this machine):
  - No shader is executed. No GPU output is produced.
  - No CPU-vs-GPU conformance comparison is run. The equality
    K_C(x) == K_Vulkan(x) is asserted by construction (the shader
    mirrors the C kernel line for line, and GLSL fma() is the
    same fused operation as C fmaf()) but not measured.
  - No SAW or Cryptol verification covers this shader. SAW
    verifies LLVM bitcode, not SPIR-V, and neither toolchain is
    installed here. A SPIR-V backend or clspv route would need
    target-specific validation of its own.

The shader is written and validated. It is not claimed to
have run on a GPU, and no GPU performance or equivalence result
is reported.
