Numerical contract — AVIAN-MORPH Vulkan gather/scatter shader
================================================================

Scope
-----
This contract covers the compute shader in
vulkan/shaders/gather_scatter.comp. It mirrors the C11 kernels
avian_gather_f32 and avian_scatter_f32 in src/c11/avian_gather.c.
The invariant is:

  K_CBQN(x) == K_C(x) == K_Vulkan(x)

where K_CBQN is the CBQN reference, K_C is the C11 kernel, and
K_Vulkan is the shader. The first equality is verified by the
C11 test suite (1212 checks, 0 failures, CBQN fixtures regenerated
on every run). The second equality is NOT verified here, because
there is no GPU on this machine. See "Verification boundary" below.

Semantics
---------
Gather (mode 0):
  output[i] = input[index[i]], for i in [0, index count).
  One invocation per output element. Output length equals index
  count. Each index must be a non-negative integer less than the
  input count.

Scatter (mode 1):
  output[index[i]] = input[i], for i in [0, input count),
  with output starting as all +0.0f. If two input elements target
  the same index, the LAST one in input order wins. An index that
  is never written stays +0.0f.
  One invocation per output element. For output element j, the
  invocation scans all input elements in order and keeps the
  value of the last input whose index equals j (or +0.0f when no
  input targets j).

Both operations copy values only. There is no arithmetic, so
there is no rounding and no overflow. A copied value is
bit-identical to the input value it copies, including the sign
of zero. The scatter zero fill writes +0.0f. The shader is
deterministic: the same inputs always give the same outputs.

Why the scatter shader scans (the race and the cost)
----------------------------------------------------
A scatter has a write hazard: two input elements can target the
same output index. The natural GPU shape (one invocation per
input element, each writing its output index) races when two
inputs target the same index, and the winner is unspecified.
An atomic write (atomic exchange) also does not fix it: the
winner is still unspecified, and the C kernel's rule is that
the LAST input in input order wins, which an atomic cannot
express. A sorted scatter would fix it, but it needs a sort
and a segmented write, which is a different kernel.

The shader therefore uses one invocation per OUTPUT element,
and each invocation scans all input elements in order, keeping
the value of the last input whose index equals its output
element. The scan order is the input order, so the last writer
wins by construction. Each invocation writes only its own
output element, so there is no race. The cost is O(output count
* input count) work for a scatter dispatch. That cost is the
price of a deterministic last-wins rule without a race, and it
is documented here, not hidden. The host is expected to use
the scatter shader only for small index sets; large scatters
belong on the C kernel or on a sorted scatter. The gather
dispatch is O(index count) work (one invocation per index),
the same as the C kernel.

Host-side validation (the boundary between host and device)
-----------------------------------------------------------
The shader performs the copying only. The host validates before
dispatch, using exactly the same checks as the C kernels
(see src/c11/CONTRACT_gather.md):
  1. Buffers are non-NULL and non-overlapping (the written
     output must not overlap the input or the indices).
  2. Gather: output holds one value per index. Scatter: the
     index count equals the input count, and the output count
     is the caller-given length (at least 1 when there is
     input to scatter).
  3. Every index is in range (gather: 0 <= index < input
     count; scatter: 0 <= index < output count).
  4. Every input element is finite.
A dispatch on unvalidated buffers is a host error, not a shader
error. This is the same division of labour as the C kernels,
where the validation pass runs before the compute pass.

Binding layout (set 0)
----------------------
  binding 0 — readonly input buffer (float data[])
  binding 1 — readonly index buffer (int data[])
  binding 2 — output buffer (float data[])
  push constant — mode (uint, 0 = gather, 1 = scatter),
                  in_count (uint), idx_count (uint),
                  out_count (uint)

Workgroup size is 256. For gather the host dispatches
ceil(index count / 256) workgroups. For scatter the host
dispatches ceil(output count / 256) workgroups. Invocations
with an index beyond the relevant count return immediately.
An empty dispatch (zero workgroups) is valid and writes nothing.
For scatter with input count 0 and output count > 0, the shader
writes +0.0f to every output element (the scan finds no input
targeting any index), which is the zero fill.

Verification boundary (what is and is not verified here)
---------------------------------------------------------
VERIFIED (by execution on this machine):
  - The shader compiles to SPIR-V with glslangValidator.
  - The SPIR-V module passes spirv-val validation.
  - The SPIR-V disassembly contains the expected operations:
    the gather index load and input load and output store, the
    scatter loop with the index comparison and the input load
    and output store, the mode branch, and the count guards.

UNVERIFIED (no GPU on this machine):
  - The shader is not executed. No GPU output is produced.
  - No CPU-vs-GPU conformance comparison is run. The equality
    K_C(x) == K_Vulkan(x) is asserted by construction (the shader
    mirrors the C kernels) but not measured.
  - No SAW or Cryptol verification covers this shader. SAW
    verifies LLVM bitcode, not SPIR-V, and neither toolchain is
    installed here. A SPIR-V backend or clspv route would need
    target-specific validation of its own.

The shader is written and validated. It is not claimed to
have run on a GPU, and no GPU performance or equivalence result
is reported.
