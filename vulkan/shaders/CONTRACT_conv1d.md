Numerical contract — AVIAN-MORPH Vulkan conv1d shader
========================================================

Scope
-----
This contract covers the compute shader in
vulkan/shaders/conv1d_f32.comp. It mirrors the C11 kernel
avian_conv1d_f32 in src/c11/avian_conv1d.c. The invariant is:

  K_CBQN(x) == K_C(x) == K_Vulkan(x)

where K_CBQN is the CBQN reference, K_C is the C11 kernel, and
K_Vulkan is the shader. The first equality is verified by the
C11 test suite (1069 checks, 0 failures, CBQN fixtures regenerated
on every run). The second equality is NOT verified here, because
there is no GPU on this machine. See "Verification boundary" below.

Semantics
---------
For each i in [0, count):
  output[i] = k0*input[i-1] + k1*input[i] + k2*input[i+1]
where out-of-range inputs read as 0 (zero padding). Output length
equals input length.

Each tap is a fused multiply-add (fma), accumulated left to right
in the fixed order k0, then k1, then k2, matching the C kernel.
Each invocation computes exactly one output element from its
3-tap neighbourhood. There is no cross-element reduction beyond
the 3-tap window. The shader is deterministic: the same inputs
always give the same outputs.

Host-side validation (the boundary between host and device)
-----------------------------------------------------------
The shader performs the arithmetic only. The host validates before
dispatch, using exactly the same checks as the C kernel
(see src/c11/CONTRACT_conv1d.md):
  1. count == 0 is valid; the host dispatches zero workgroups.
  2. Buffers are non-NULL and non-overlapping.
  3. Output holds at least count elements.
  4. All three kernel coefficients and every input element are
     finite, and every accumulation is finite.

A dispatch on unvalidated buffers is a host error, not a shader
error. The shader assumes the preconditions hold. This is the
same division of labour as the C kernel, where the validation
pass runs before the compute pass.

Binding layout (set 0)
----------------------
  binding 0 — readonly input buffer
  binding 1 — output buffer
  push constant — k0, k1, k2 (float), count (uint)

Workgroup size is 256. The host dispatches ceil(count / 256)
workgroups. Invocations with index >= count return immediately.

Verification boundary (what is and is not verified here)
---------------------------------------------------------
VERIFIED (by execution on this machine):
  - The shader compiles to SPIR-V with glslangValidator.
  - The SPIR-V module passes spirv-val validation.
  - The SPIR-V disassembly contains the expected arithmetic
    (fused multiply-add operations, the zero-padding boundary
    checks, and the output store).

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
