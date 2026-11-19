Shader contract — normalize.comp (Vulkan compute shader)
=========================================================

Scope
-----
This contract covers the normalization shader in
vulkan/shaders/normalize.comp. It states its exact semantics
and the limits of what is verified here.

Semantics
---------
For each element i in [0, n):
  out[i] = in[i] / norm.
The input is N f32 values. The output is N f32 values. The
caller passes n, the element count. The norm comes from a
push constant, computed by the host (see the design decision
below). Each invocation computes exactly one output element:
one correctly-rounded f32 division.

Design decision: the shader performs the division step only
------------------------------------------------------------
The norm is a reduction over the whole vector, and this shader
does NOT compute it. The host computes the norm with the same
fmaf accumulation order as the C kernel (left to right from
+0.0f, then sqrtf) and passes the validated norm in as a push
constant. Rationale:
  - A single-workgroup shared-memory reduction caps the vector
    at the workgroup size unless strided, and a strided tree
    reduction accumulates in a different order than the C
    kernel's strict left-to-right fmaf chain, so the shader's
    norm would differ from the C kernel's norm by rounding.
  - The host already computes the sum of squares to validate
    the preconditions (the norm must be finite and nonzero
    before dispatch), so passing that validated norm in is
    exact and keeps the shader's division identical to the C
    kernel's division step.
A dispatch with an unvalidated norm (zero, infinite, NaN, or
computed in a different accumulation order) is a host error,
not a shader error.

Host validation (same checks as the C kernel)
----------------------------------------------
The host validates before dispatch, using the same checks as
avian_normalize_f32 (see src/c11/CONTRACT_norm.md): buffers
are non-NULL and non-overlapping, output and input each hold
at least n values, every input element is finite, the sum of
squares is finite, and the norm is finite and nonzero. The
shader performs the arithmetic only; it does not re-validate.
A dispatch on unvalidated buffers is a host error, not a
shader error.

Floating-point behaviour (f32)
-------------------------------
- The division is one correctly-rounded f32 division per
  element (round-to-nearest-even). It cannot overflow: every
  |in[i]| is at most the norm, so every |out[i]| is at most 1.
- A zero input component divided by the (positive) norm keeps
  its sign (+0.0f / norm is +0.0f, -0.0f / norm is -0.0f),
  the same IEEE division sign rule as the C kernel.
- n == 0 is valid. The host dispatches zero workgroups and
  writes nothing.
- The shader's division matches the C kernel's division bit
  for bit, because both divide by the same host-computed norm
  (the C kernel's norm, computed in the same fmaf order).
  The 1-ULP difference documented in
  src/c11/CONTRACT_norm.md is between the C kernel and the
  CBQN reference (f32 norm vs f64 norm); the shader and the
  C kernel agree, because the shader uses the C kernel's norm.

Verification status (explicit boundaries, not claims)
-----------------------------------------------------
- SPIR-V compilation: the shader compiles to SPIR-V with
  glslangValidator and passes spirv-val validation. Status:
  VERIFIED by execution (this VM, 2026-11-19).
- Disassembly: the SPIR-V disassembly confirms the expected
  structure (the count guard, the input load, one OpFDiv,
  and the output store). Status: VERIFIED by inspection
  (this VM, 2026-11-19).
- Execution: NOT RUN. There is no GPU on this VM. The shader
  is validated, not executed, and no CPU-vs-GPU comparison
  has been run. Status: UNVERIFIED (no GPU).
- SAW and Cryptol: NOT RUN / NOT WRITTEN. Neither toolchain
  is installed on this VM. Status: UNVERIFIED (toolchain
  missing).
- q, Self, and Mercury toolchains are not installed on this
  VM. Those layers are documented as unavailable, not
  substituted.

Reproduction
------------
From vulkan/shaders/: `make test` compiles every shader to
SPIR-V with glslangValidator and validates each with
spirv-val. It exits 0 only when every shader compiles and
validates.
