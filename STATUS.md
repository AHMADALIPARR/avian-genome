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
    Bit-exact on all fixtures, plus independent cross-checks.
  - CBQN binary: ~/workspace/.tools/cbqn/cbqn, commit
    588753c83963f5b1e9247bfba9c16758da22a348.

Vulkan shaders (vulkan/shaders):
  - Eight shaders compile to SPIR-V and pass spirv-val.
    make test exits 0, "ALL SHADERS VALID". No GPU execution.

OpenQASM circuits (openqasm/):
  - neuron_3q.qasm and retinal_pair.qasm.
  - Reference simulator (openqasm/sim/qsim): make test,
    20 checks passed, 0 failed, exit 0.

Ruby composition layers (ruby/):
  - morphic.rb — prototype neurons with clone/delegate slots,
    RetinalGanglion and Tectal prototypes, concrete Neuron
    instances with connectivity.
  - genome.rb — genomic layout (affine transform matching the
    C11/CBQN geom contract) deriving twin parameters from
    tissue coordinates.
  - gate_ir.rb — typed gate IR (ry, rz, cx, measure) with
    qubit bounds, arity, and measurement validation before
    OpenQASM emission; retinal_pair builder.
  - retinal_tectal.rb — orchestrator wired through morphic,
    genome, and gate_ir. Runs three stimulus conditions
    (dark / dim / bright), validates the IR, drives qsim_run,
    writes fixtures/quantum/retinal_pair_bright.qasm.

Verification record
-------------------
verification/VERIFICATION_STATUS.md is the single honest
record of what is verified and what is not.

Not verified (toolchain or hardware missing)
---------------------------------------------
  - SAW: toolchain not installed. No proof run.
  - Cryptol: toolchain not installed. No spec written.
  - Vulkan execution and CPU-vs-GPU conformance: no GPU.
  - q/kdb+: commercial, not installed.
  - Self language runtime: not installed.
  - Mercury compiler (mmc): not installed.

The morphic, genome, and gate_ir modules above implement the
behaviour those layers need inside the languages already in
the stack (Ruby + verified C11/CBQN geometry). They are not
claims that Self, q, or Mercury are present.

First milestone (integration/full_pipeline/MILESTONE.md):
  - Morphic digital twin of a small avian retinal-tectal
    circuit: one retinal ganglion cell driving one tectal
    cell through a controlled-NOT coupling.
  - Ruby orchestrator drives the verified simulator for three
    stimulus conditions. Measured: dark A silent B silent;
    dim A graded 0.5 B graded 0.5; bright A spiking B spiking.

Limits
------
  - No GPU on this machine. Vulkan compute cannot execute here.
  - No SAW, Cryptol, q, Self, or Mercury toolchain on this
    machine.
  - No Python, JavaScript, TypeScript, CUDA, PyTorch, or
    other framework is used in the numerical kernel path.
