Verification status — AVIAN-MORPH
====================================

This file is the single honest record of what is verified by
execution on this machine and what is not. A claim appears here
only if a tool on this machine produced it. Anything else is
marked UNVERIFIED with the reason.

Last updated: 2026-11-19. Machine: this VM (x86-64 Linux,
~1.5 CPU, ~6 GB RAM, no GPU).

VERIFIED BY EXECUTION
---------------------
C11 kernel ABI (src/c11):
  - avian_scale_f32 and avian_scale_i32, with the avian_array.h
    interface and the avian_status error enum.
  - avian_conv1d_f32 (1D, 3-tap, zero padding), with the
    avian_conv1d.h interface. Fused multiply-add accumulation,
    left to right.
  - avian_reduce_segments_f32 (segmented sum, f32), with the
    avian_reduce.h interface. Left-to-right accumulation from
    +0.0f per segment; an empty segment sums to +0.0f.
  - avian_gather_f32 and avian_scatter_f32 (sparse gather /
    scatter, f32), with the avian_gather.h interface. Gather:
    output[i] = input[index[i]], one value per index.
    Scatter: output starts as all +0.0f, then each input
    element is written to its index in input order; the LAST
    input targeting an index wins; an unwritten index stays
    +0.0f. Both copy values only (bit-exact), and both reject
    a non-finite input element. A failed call writes nothing.
  - avian_geom_transform_f32 (3D affine coordinate transform,
    f32), with the avian_geom.h interface. For each point i
    and each row r: out[3i+r] = M[3r+0]*in[3i+0] +
    M[3r+1]*in[3i+1] + M[3r+2]*in[3i+2] + t[r]. The matrix
    is 9 f32 row-major, the translation is 3 f32. Accumulation
    is left to right with fused multiply-add, starting from
    +0.0f, the matrix row term by term, then the translation.
    A zero output is +0.0f. The output must hold at least 3N
    values and must not overlap the input, the matrix, or the
    translation.
  - avian_complex_rotate_f32 (complex rotation, f32), with
    the avian_complex.h interface. For each complex number i:
    out_re[i] = in_re[i]*rot_re - in_im[i]*rot_im and
    out_im[i] = in_re[i]*rot_im + in_im[i]*rot_re. The input
    and output are N complex numbers stored flat in
    interleaved re/im order (2N values). The rotation is
    given directly as a complex number, not as an angle.
    Each output component is two fused multiply-adds: the
    inner product first (rounded once), then the outer fused
    multiply-add of the other product plus that rounded inner
    product. A zero output component is +0.0f. The output must
    hold at least 2N values and must not overlap the input.
  - avian_normalize_f32 (L2 normalization to a unit vector,
    f32), with the avian_norm.h interface. For each element i:
    out[i] = in[i] / norm, norm = sqrt(sum of in[i]^2). The
    sum of squares is accumulated left to right with fused
    multiply-add from +0.0f, the norm is sqrtf, and each
    output is one f32 division. A zero norm returns
    AVIAN_ERR_COUNT, covering both the all-zeros vector and
    the case where the sum of squares underflows f32 to zero.
    A sum of squares that overflows f32 returns
    AVIAN_ERR_NONFINITE. A zero input component divided by
    the positive norm keeps its sign (IEEE division), which
    differs from the other kernels, where the accumulation
    from +0.0f forces a zero result to +0.0f. The output must
    hold at least N values and must not overlap the input.
  - Test suite: make test, 1055 checks (scale), 1069 checks
    (conv1d), 71 checks (reduce), 1212 checks (gather), 407
    checks (geom), 300 checks (complex), and 190 checks
    (norm) passed, 0 failed, exit 0. Run twice from a clean
    tree, identical both times.
  - CBQN references (cbqn/scale.bqn, cbqn/conv1d.bqn,
    cbqn/reduce_segments.bqn, cbqn/gather_scatter.bqn,
    cbqn/geom_transform.bqn, cbqn/complex_rotate.bqn,
    cbqn/normalize.bqn)
    regenerate the fixtures on every make test run, so the C
    kernels are compared against a fresh CBQN run, never a
    stale file. Bit-exact on all fixtures, plus independent
    cross-checks on inputs outside the fixtures (identical C
    and CBQN results, including the identity transform, a pure
    translation, and a rotation with scaling and translation
    for the geometry kernel, rotation by the imaginary
    unit and by -1 for the complex kernel, and ⟨3, 4⟩ to
    ⟨0.6, 0.8⟩ and the already-unit vector ⟨1, 0, 0⟩ for
    the normalization kernel). One honest limit:
    on inexact values the C complex kernel's two-step fused
    multiply-add rounding can differ from the CBQN reference's
    exact sum by 1 ULP (input (-0.753165364, -2.34717751),
    rotation (0.991736054, -1.76565647): C gives
    (-4.89125013, -0.997949183), CBQN gives (-4.89125061,
    -0.997949243)); the test pins the kernel to the two-step
    value. A second honest limit: on inexact values the C
    normalization kernel's f32-rounded norm (sqrtf) can differ
    from the CBQN reference's f64 norm by up to half an f32
    ULP, so the division rounds quotients computed from
    different norms and can land on adjacent f32 values,
    differing by exactly 1 ULP (input vector 0..99: element 1,
    C 0.00174514495 vs CBQN 0.00174514484; element 2, C
    0.00349028991 vs CBQN 0.00349028967); the test pins the
    kernel to the f32-norm value. The fixtures use exact
    values only, so the fixture comparison stays bit-exact.
    See src/c11/CONTRACT_complex.md and
    src/c11/CONTRACT_norm.md.
  - Contracts: src/c11/CONTRACT.md (scale),
    src/c11/CONTRACT_conv1d.md (convolution),
    src/c11/CONTRACT_reduce.md (segmented reduction),
    src/c11/CONTRACT_gather.md (gather / scatter),
    src/c11/CONTRACT_geom.md (coordinate transform),
    src/c11/CONTRACT_complex.md (complex rotation),
    src/c11/CONTRACT_norm.md (normalization).

CBQN reference oracle:
  - CBQN binary at ~/workspace/.tools/cbqn/cbqn, commit
    588753c83963f5b1e9247bfba9c16758da22a348, built with FFI
    and singeli X86_64. Verified by direct runs (+/ over
    iota, reductions, scans).

Vulkan shaders, SPIR-V validation (vulkan/shaders):
  - scale_f32.comp, scale_i32.comp, conv1d_f32.comp,
    reduce_segments.comp, gather_scatter.comp,
    geom_transform.comp, complex_rotate.comp, and
    normalize.comp compile to SPIR-V with glslangValidator
    11.16.6.0.
  - All eight SPIR-V modules pass spirv-val validation.
    make test exits 0, "ALL SHADERS VALID".
  - Disassembly confirms the expected arithmetic: f32 multiply
    (OpFMul), i64 multiply (OpIMul) with narrowing store, three
    fused multiply-adds (OpExtInst Fma) for the convolution,
    one f32 addition (OpFAdd) in a loop for the segmented
    reduction, the count guard (OpUGreaterThanEqual +
    OpBranchConditional), the zero-padding boundary checks,
    the segment length and offset loads, and the output store
    (OpStore). For gather_scatter: the mode branch (OpIEqual
    on the mode push constant), the gather index load plus
    input load plus output store, and the scatter loop
    (OpLoopMerge) with the index comparison (OpIEqual) and the
    last-writer overwrite of the per-output accumulator, which
    is initialised to +0.0f. The scatter shader is race-free
    by construction (one invocation per output element, each
    writing only its own output element) and preserves the
    last-wins rule by scanning the inputs in order; its cost
    is O(output count * input count) work, documented in
    vulkan/shaders/CONTRACT_gather.md. For geom_transform:
    four fused multiply-adds (OpExtInst Fma) per output
    component inside a row loop (OpLoopMerge), the point-count
    guard, the input/matrix/translation loads, and the output
    store. For complex_rotate: four fused multiply-adds
    (OpExtInst Fma), two per output component, the
    complex-count guard, the input loads, and the output
    store; the shader's fma is the same fused operation as
    the C kernel's fmaf, so the shader matches the C kernel's
    two-step rounding (see vulkan/shaders/CONTRACT_complex.md).
    For normalize: one f32 division (OpFDiv), the count guard,
    the input load, the norm load from the push constant, and
    the output store; the shader performs the division step
    only and the norm comes from the host, computed with the
    same fmaf accumulation order as the C kernel (see
    vulkan/shaders/CONTRACT_norm.md).
  - Contracts: vulkan/shaders/CONTRACT.md (scale),
    vulkan/shaders/CONTRACT_conv1d.md (convolution),
    vulkan/shaders/CONTRACT_reduce.md (segmented reduction),
    vulkan/shaders/CONTRACT_gather.md (gather / scatter),
    vulkan/shaders/CONTRACT_geom.md (coordinate transform),
    vulkan/shaders/CONTRACT_complex.md (complex rotation),
    vulkan/shaders/CONTRACT_norm.md (normalization).

First milestone (morphic digital twin, retinal-tectal):
  - VERIFIED by execution. The Ruby orchestrator
    (ruby/retinal_tectal.rb) drives the verified simulator
    for three stimulus conditions. Measured: dark A silent
    (0.0) B silent (0.0); dim A graded (0.5) B graded (0.5);
    bright A spiking (1.0) B spiking (1.0). Exit 0.
    Works from a clean tree.
  - The twin's Self prototypes, q genome, and Mercury
    validation are NOT built (toolchains missing). Those
    are documented in integration/full_pipeline/MILESTONE.md,
    not substituted.

UNVERIFIED (with the reason)
----------------------------
SAW verification:
  - NOT RUN. The SAW toolchain is not installed on this VM.
  - No SAWScript, no proof log, no equivalence result exists
    for any kernel. Status: UNVERIFIED (toolchain missing).

Cryptol specifications:
  - NOT WRITTEN. The Cryptol toolchain is not installed on
    this VM. No Cryptol spec exists for any kernel.
    Status: UNVERIFIED (toolchain missing).

Vulkan execution and CPU-vs-GPU conformance:
  - NOT RUN. There is no GPU on this VM. No shader is executed
    and no GPU output is produced, so the equality
    K_C(x) == K_Vulkan(x) is asserted by construction (the shader
    mirrors the C kernel) but not measured.
    Status: UNVERIFIED (no GPU).
  - Note: SAW verifies LLVM bitcode, not SPIR-V. A SPIR-V
    backend or clspv route would need target-specific
    validation of its own.

q geometry layer:
  - NOT BUILT. q/kdb+ is commercial and is not installed on
    this VM. No q code is executed here.
    Status: UNVERIFIED (toolchain missing, commercial).

Self morphic objects:
  - NOT BUILT. The Self toolchain is not installed on this VM.
    Status: UNVERIFIED (toolchain missing).

OpenQASM circuits:
  - WRITTEN and VERIFIED by the reference simulator.
  - openqasm/neuron_3q.qasm (3-qubit neuron template) and
    openqasm/retinal_pair.qasm (6-qubit retinal-tectal
    circuit) exist.
  - The reference simulator (openqasm/sim/qsim) reproduces
    the hand-derived measurement probabilities for both
    templates at the tested parameter values, to a tolerance
    of 1e-12. Test suite: make test in openqasm/sim,
    20 checks passed, 0 failed, exit 0. Every simulated
    state is normalized.
  - The templates are not run on a real quantum device or
    a third-party OpenQASM simulator. They are validated by
    this reference simulator only.

Mercury quantum gates:
  - NOT BUILT. The Mercury compiler (mmc) is not installed on
    this VM. Status: UNVERIFIED (toolchain missing). The
    OpenQASM templates are validated by the reference
    simulator instead; no Mercury check is claimed.

Ruby orchestration:
  - BUILT and VERIFIED by execution. ruby/retinal_tectal.rb
    drives the verified simulator for the first milestone.
    See the milestone entry above.

Rule
----
A status changes from UNVERIFIED to VERIFIED only when a tool
on a real machine produces the result. A missing toolchain is
recorded as a missing toolchain, never silently substituted.
