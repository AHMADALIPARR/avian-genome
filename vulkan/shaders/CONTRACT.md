Numerical contract — AVIAN-MORPH Vulkan scale shaders
====================================================

Scope
-----
This contract covers the two compute shaders in
vulkan/shaders/scale_f32.comp and vulkan/shaders/scale_i32.comp.
They mirror the C11 kernels avian_scale_f32 and avian_scale_i32
in src/c11/avian_scale.c. The invariant is:

  K_CBQN(x) == K_C(x) == K_Vulkan(x)

where K_CBQN is the CBQN reference, K_C is the C11 kernel, and
K_Vulkan is the shader. The first equality is verified by the
C11 test suite (1055 checks, 0 failures, CBQN fixtures regenerated
on every run). The second equality is NOT verified here, because
there is no GPU on this machine. See "Verification boundary" below.

Semantics
---------
For each i in [0, count):
  f32: output[i] = input[i] * scale, one correctly-rounded IEEE 754
       single-precision multiply.
  i32: output[i] = input[i] * scale, exact integer arithmetic.
       The product is formed in 64 bits and narrowed to int32.

There is no cross-element reduction, so element order does not
affect the result. The shaders are deterministic: the same inputs
always give the same outputs.

Host-side validation (the boundary between host and device)
-----------------------------------------------------------
The shaders perform the arithmetic only. The host validates before
dispatch, using exactly the same checks as the C kernel
(see src/c11/CONTRACT.md):
  1. count == 0 is valid; the host dispatches zero workgroups.
  2. Buffers are non-NULL and non-overlapping.
  3. Output holds at least count elements.
  4. f32: scale and every input element are finite, and every
     product is finite.
  5. i32: every product fits in int32.

A dispatch on unvalidated buffers is a host error, not a shader
error. The shader assumes the preconditions hold. This is the
same division of labour as the C kernel, where the validation
pass runs before the compute pass.

Binding layout (set 0, both shaders)
------------------------------------
  binding 0 — readonly input buffer
  binding 1 — output buffer
  push constant — scale, count

Workgroup size is 256. The host dispatches ceil(count / 256)
workgroups. Invocations with index >= count return immediately.

Verification boundary (what is and is not verified here)
---------------------------------------------------------
VERIFIED (by execution on this machine):
  - Each shader compiles to SPIR-V with glslangValidator.
  - Each SPIR-V module passes spirv-val validation.
  - The SPIR-V disassembly contains the expected arithmetic
    (f32 multiply; i64 multiply plus narrowing store).

UNVERIFIED (no GPU on this machine):
  - No shader is executed. No GPU output is produced.
  - No CPU-vs-GPU conformance comparison is run. The equality
    K_C(x) == K_Vulkan(x) is asserted by construction (the shader
    mirrors the C kernel line for line) but not measured.
  - No SAW or Cryptol verification covers these shaders. SAW
    verifies LLVM bitcode, not SPIR-V, and neither toolchain is
    installed here. A SPIR-V backend or clspv route would need
    target-specific validation of its own.

The shaders are written and validated. They are not claimed to
have run on a GPU, and no GPU performance or equivalence result
is reported.
