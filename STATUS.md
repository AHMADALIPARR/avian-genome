STATUS — AVIAN-MORPH
====================

Last updated: 2026-11-19. Machine: this VM (x86-64 Linux,
~1.5 CPU, ~6 GB RAM, no GPU).

Completed and verified by execution
------------------------------------
C11 kernel ABI (src/c11):
  - avian_array.h, avian_scale.c (f32 and i32 scale kernels),
    test_avian_scale.c, Makefile, CONTRACT.md.
  - avian_conv1d.h, avian_conv1d.c (f32 1D 3-tap convolution,
    zero padding, fused multiply-add), test_avian_conv1d.c,
    CONTRACT_conv1d.md.
  - avian_reduce.h, avian_reduce.c (f32 segmented sum reduction,
    left-to-right accumulation from +0.0f, empty segment sums
    to +0.0f), test_avian_reduce.c, CONTRACT_reduce.md.
  - avian_gather.h, avian_gather.c (f32 sparse gather and
    scatter; gather output[i] = input[index[i]]; scatter zero
    fills +0.0f then writes in input order, last-wins for
    duplicate indices, unwritten indices stay +0.0f),
    test_avian_gather.c, CONTRACT_gather.md.
  - avian_geom.h, avian_geom.c (f32 3D affine coordinate
    transform; out[3i+r] = M[3r]*in[3i] + M[3r+1]*in[3i+1] +
    M[3r+2]*in[3i+2] + t[r], matrix 9 f32 row-major,
    translation 3 f32, fused multiply-add accumulation left
    to right from +0.0f, a zero output is +0.0f),
    test_avian_geom.c, CONTRACT_geom.md.
  - avian_complex.h, avian_complex.c (f32 complex rotation;
    out_re = in_re*rot_re - in_im*rot_im and
    out_im = in_re*rot_im + in_im*rot_re per complex number,
    interleaved re/im layout, the rotation given directly as
    a complex number rather than an angle, two-step fused
    multiply-add per component, a zero output component is
    +0.0f), test_avian_complex.c, CONTRACT_complex.md.
  - avian_norm.h, avian_norm.c (f32 L2 normalization to a unit
    vector; out[i] = in[i] / norm, norm = sqrt(sum of squares),
    sum of squares accumulated left to right with fused
    multiply-add from +0.0f, sqrtf norm, one f32 division per
    element; a zero norm returns AVIAN_ERR_COUNT, covering
    both the all-zeros vector and the case where the sum of
    squares underflows f32 to zero; a zero input component
    divided by the positive norm keeps its sign),
    test_avian_norm.c, CONTRACT_norm.md.
  - make test: 1055 checks (scale), 1069 checks (conv1d),
    71 checks (reduce), 1212 checks (gather), 407 checks
    (geom), 300 checks (complex), and 190 checks (norm)
    passed, 0 failed, exit 0. Run twice from a clean tree,
    identical both times.
  - CBQN references regenerate fixtures on every run.
    Bit-exact on all fixtures, plus independent
    cross-checks on inputs outside the fixtures (including
    the identity transform, a pure translation, and a rotation
    with scaling and translation for the geometry kernel, and
    rotation by the imaginary unit and by -1 for the complex
    kernel). One honest limit: on inexact values the C
    kernel's two-step fused multiply-add rounding can differ
    from the CBQN reference's exact sum by 1 ULP; the concrete
    case is documented in src/c11/CONTRACT_complex.md and
    pinned by test. A second honest limit: on inexact values
    the C normalization kernel's f32-rounded norm (sqrtf)
    can differ from the CBQN reference's f64 norm by up to
    half an f32 ULP, so the division rounds quotients computed
    from different norms and can land on adjacent f32 values,
    differing by exactly 1 ULP (the 0..99 vector: elements 1
    and 2); the concrete case is documented in
    src/c11/CONTRACT_norm.md and pinned by test. The fixtures
    use exact values only, so the fixture comparison stays
    bit-exact.
  - CBQN binary: ~/workspace/.tools/cbqn/cbqn, commit
    588753c83963f5b1e9247bfba9c16758da22a348.

Vulkan shaders (vulkan/shaders):
  - scale_f32.comp, scale_i32.comp, conv1d_f32.comp,
    reduce_segments.comp, gather_scatter.comp,
    geom_transform.comp, complex_rotate.comp, normalize.comp,
    Makefile, CONTRACT.md, CONTRACT_conv1d.md,
    CONTRACT_reduce.md, CONTRACT_gather.md,
    CONTRACT_geom.md, CONTRACT_complex.md,
    CONTRACT_norm.md.
  - make test: all eight shaders compile to SPIR-V with
    glslangValidator 11.16.6.0 and pass spirv-val validation.
    Exit 0, "ALL SHADERS VALID".
  - Disassembly confirms the expected arithmetic (OpFMul for
    f32, OpIMul on 64-bit values for i32, three OpExtInst Fma
    for the convolution, one OpFAdd in a loop for the segmented
    reduction, four OpExtInst Fma per output component inside
    a row loop for the geometry transform, four OpExtInst Fma
    for the complex rotation, two per output component, one
    OpFDiv for the normalization), the count guard, the
    zero-padding boundary checks, the segment length and
    offset loads, and the output store. For gather_scatter:
    the mode branch, the gather index load plus input load
    plus output store, and the scatter loop with the index
    comparison and the last-writer overwrite of the per-output
    accumulator (initialised to +0.0f). The scatter shader is
    race-free by construction (one invocation per output
    element) and preserves last-wins by scanning the inputs
    in order, at O(output count * input count) work,
    documented in CONTRACT_gather.md. The complex rotation
    shader's fma matches the C kernel's fma (GLSL fma() is the
    same fused operation as C fmaf()), documented in
    CONTRACT_complex.md. The normalization shader performs
    the division step only: the norm comes from a push
    constant, computed by the host with the same fmaf
    accumulation order as the C kernel (a single-workgroup
    reduction would accumulate in a different order and a
    strided tree reduction would too, so the shader's norm
    would differ by rounding; the host already computes the
    sum of squares to validate the preconditions), documented
    in CONTRACT_norm.md.

OpenQASM circuits (openqasm/):
  - neuron_3q.qasm and retinal_pair.qasm.
  - Reference simulator (openqasm/sim/qsim): make test,
    20 checks passed, 0 failed, exit 0. The simulator
    reproduces the hand-derived measurement probabilities
    for both templates to a tolerance of 1e-12, and every
    simulated state is normalized.
  - Mercury compiler (mmc) is not installed, so no typed
    gate IR validation is run. The templates are validated
    by the reference simulator instead. See mercury/BOUNDARY.md.

Verification record
-------------------
verification/VERIFICATION_STATUS.md is the single honest
record of what is verified and what is not.

Not verified (toolchain or hardware missing)
---------------------------------------------
  - SAW: toolchain not installed. No proof run.
    Boundary: saw/BOUNDARY.md.
  - Cryptol: toolchain not installed. No spec written.
    Boundary: cryptol/BOUNDARY.md.
  - Vulkan execution and CPU-vs-GPU conformance: no GPU.
    The shaders are validated, not executed.
  - q: commercial, not installed. No q code runs here.
    Boundary: q/BOUNDARY.md.
  - Self: toolchain not installed.
    Boundary: self/BOUNDARY.md.
  - Mercury: compiler (mmc) not installed.
    Boundary: mercury/BOUNDARY.md.

First milestone (integration/full_pipeline/MILESTONE.md):
  - Morphic digital twin of a small avian retinal-tectal
    circuit: one retinal ganglion cell driving one tectal
    cell through a controlled-NOT coupling.
  - Ruby orchestrator (ruby/retinal_tectal.rb) drives the
    verified simulator for three stimulus conditions.
    Measured: dark A silent B silent; dim A graded 0.5
    B graded 0.5; bright A spiking B spiking. Exit 0,
    ALL CONDITIONS OK. Works from a clean tree (builds
    the simulator on demand).
  - Generated fixture: fixtures/quantum/retinal_pair_bright.qasm.
  - Constraints: Self prototypes, q genome, and Mercury
    validation are missing toolchains, documented in
    MILESTONE.md and the corresponding BOUNDARY.md files,
    not substituted.

Not started
-----------
  - All seven kernel families are complete (scale,
    convolution, segmented reduction, gather/scatter,
    coordinate transform, complex rotation, normalization).
    The remaining layers (Self prototypes, q geometry,
    Mercury gates, SAW proofs, Cryptol specs) are blocked
    on missing toolchains, not started. Each has a finished
    BOUNDARY.md that records the missing toolchain and the
    stand-in used for the first milestone only.

Limits
------
  - No GPU on this machine. Vulkan compute cannot execute
    here; SPIR-V is written and validated only.
  - No SAW, Cryptol, q, Self, or Mercury toolchain on this
    machine. Those layers are documented as missing, not
    substituted with another language or framework.
  - No Python, JavaScript, TypeScript, CUDA, PyTorch, or
    other framework is used in the numerical kernel path,
    per the repository enforcement rules.
