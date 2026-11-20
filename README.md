# AVIAN-MORPH

![License](https://img.shields.io/badge/license-SNAPKITTY%20Opaque%201.0-blue)
![C11](https://img.shields.io/badge/language-C11-blue)
![CBQN](https://img.shields.io/badge/reference-CBQN-purple)
![Vulkan](https://img.shields.io/badge/compute-Vulkan%201.3-red)
![OpenQASM](https://img.shields.io/badge/circuits-OpenQASM%203-orange)
![Ruby](https://img.shields.io/badge/orchestration-Ruby-CC342D)
![Checks](https://img.shields.io/badge/checks-4304%20passed%2C%200%20failed-brightgreen)
![Shaders](https://img.shields.io/badge/shaders-8%20SPIR--V%20valid-brightgreen)

**Topics:** `avian` `neurogenomics` `quantum` `vulkan` `spir-v` `cbqn` `c11` `openqasm` `digital-twin` `retina` `tectum` `array-programming` `genomics` `systems-engineering` `open-source`

**AVIAN-MORPH is the Sovereign Morphic Avian Neurogenomic Quantum Array Engine.** It models a small avian retinal-tectal circuit as a morphic digital twin: one retinal ganglion cell driving one tectal cell through a controlled-NOT coupling, with the genome drawn the way the data defines it. Biological avian neurons are the reference system, and quantum circuits are their computational counterparts. The engine does not claim that real bird neurons are quantum computers. It uses the bird visual system as the biological reference and builds a verified computational model of it.

![The AVIAN-MORPH genome front end: a dark, glowing genome visualization with DNA helices, a quantum-circuit loop, a retina-to-tectum pathway, and labeled parts](docs/images/frontend_main.png)

*The AVIAN-MORPH genome front end. The genome, the retina, the retinal ganglion cell, the optic tectum, the tectal neuron, the quantum circuit, and the synapse are drawn on a dark field in glowing blue and green, the way a Cas9 genome diagram draws its parts. Every coordinate, spike probability, and check count on the page comes from the AVIAN-MORPH files.*

## What AVIAN-MORPH is

AVIAN-MORPH is an engine, not a model card and not a demo. It takes a biological reference system — the avian retinal-tectal circuit, the pathway that carries visual information from the retina to the optic tectum — and it builds that circuit as a stack of verified layers. Each layer owns one concern, each layer speaks one language, and each layer is checked against the layer below it by execution.

The engine starts from the genome. Genomic coordinates define positions in space, and a 3D affine transform maps those coordinates onto the circuit. The retinal ganglion cell reads the stimulus. The tectal cell answers through a controlled-NOT coupling. The quantum circuit is the computational counterpart of that coupling: a 6-qubit circuit with one neuron on qubits 0–2, a coupling on qubits 2–3, and a second neuron on qubits 3–5. The numerical kernels that move the data — scale, convolution, reduction, gather and scatter, coordinate transform, complex rotation, normalization — are written once in CBQN as the reference semantics, once in C11 as the kernel ABI, and once in Vulkan as SPIR-V compute shaders. The invariant across those three writings is the spine of the whole project:

```
K_CBQN(x) == K_C(x) == K_Vulkan(x)
```

The CBQN reference, the C kernel, and the Vulkan shader compute the same function. The first equality is verified by execution: CBQN regenerates the fixtures on every test run, and the C kernel is compared against the fresh CBQN result, bit-exact, on every fixture. The second equality is asserted by construction and awaits a GPU: the shaders compile to SPIR-V and pass spirv-val validation, and the disassembly confirms the expected arithmetic, but no GPU is present on this machine, so no shader executes here.

The engine is deterministic. Every kernel validates first and computes second, so a failed call writes nothing. Every accumulation runs in a fixed order. Every fixture uses exactly representable values, so the CBQN-to-C comparison is bit-exact and needs no tolerance. The test suites run twice from a clean tree and give identical results both times. The total across the seven C11 kernel families is 4304 checks, 0 failures.

The engine is also honest about what it does not do. SAW and Cryptol are not installed, so no proof is claimed. q, Self, and Mercury are not installed; those layers are recorded as missing toolchains, not substituted. Vulkan execution and CPU-vs-GPU conformance are unverified because there is no GPU. The verification record lives in `verification/VERIFICATION_STATUS.md`, and it lists every layer as verified or unverified, with the reason.

## The front end

AVIAN-MORPH has a front end. It is a Muse space, slug `avian-morph-genome`, and it looks like a Cas9 genome diagram: a black page, a glowing translucent blue-green genome, DNA helices, a quantum-circuit loop with a CNOT mark, and a retina-to-tectum neural pathway, with labels and thin leader lines pointing to each part. The front end is connected to the real data. Every coordinate, spike probability, and check count on the page is read from the AVIAN-MORPH files. The photos in this README are screenshots of that front end.

The hero visualization draws the genome the way the data defines it. Four genome points are plotted from the geometry fixture, and selecting a point shows its input-to-output mapping. The retina feeds the retinal ganglion cell, neuron A. Neuron A drives the tectal neuron, neuron B, through the controlled-NOT coupling. The synapse is the retinal-to-tectal projection between them. The quantum circuit loop sits over the optic tectum and carries the CNOT mark that couples the two neurons.


*The Bright stimulus state. The stimulus angle is pi, neuron A spikes at 1.0, neuron B spikes at 1.0, and the pathway between them is lit. The same page in the Dark state shows both neurons silent at 0.0, and in the Dim state both graded at 0.5.*

The front end carries a stimulus switch with three conditions: Dark, Dim, and Bright. Switching the stimulus replays the simulator's own values. Dark sets the retinal input angle to 0 and both neurons read silent at 0.0. Dim sets the angle to pi/2 and both neurons read graded at 0.5. Bright sets the angle to pi and both neurons read spiking at 1.0. The neuron glow, the pathway lighting, and the probability bars all update to match. The genome parameters sit alongside: the phase angles are 0.0, and the second neuron's input angle is 0.0.

Below the visualization, the front end shows the genome coordinates table: the 3x3 matrix, the translation vector, and the four points with their input and transformed output. Below that, it shows the kernel verification table: all seven C11 kernel families with their real check counts, totaling 4304 checks and 0 failures, plus the eight Vulkan shaders, all SPIR-V valid. Nothing on the page is invented. The page reads the files, and the files are the evidence.

## Architecture by language layer

The architecture assigns one language to each layer, and no language substitutes for another. The assignment is fixed.

**Self holds the morphic organism model.** Prototype neurons, delegation, adaptive morphology, and object identity live here. Each neuron is a Self prototype that preserves biological identity, morphology, connectivity, and developmental state. The Self toolchain is not installed on this machine, so this layer is recorded as a missing toolchain, not substituted.

**Ruby holds morphic composition and orchestration.** Neuron assemblies, metaprogramming, and developmental transformations live here. Ruby orchestrates the pipeline through defined interfaces. The retinal-tectal orchestrator, `ruby/retinal_tectal.rb`, is the working proof of this layer: it generates the OpenQASM circuits, drives the reference simulator, and reports the three stimulus conditions.

**q holds genomic geometry.** Genomic coordinate maps, spatial indexing, adjacency tensors, and geometric transformations live here. q is commercial and is not installed on this machine, so no q code runs here. The genome stands in as a plain Ruby parameter hash for the milestone only, and the coordinate transform itself is implemented and verified in CBQN, C11, and Vulkan. The boundary is documented in `q/BOUNDARY.md`.

**CBQN holds array algebra and kernel reference semantics.** Rank-polymorphic operations, reductions, scans, permutations, and tensor reference algorithms live here. CBQN is the reference oracle. Every C kernel is compared against a CBQN implementation, and CBQN regenerates the fixtures on every test run, so the comparison is always against a fresh oracle result.

**C11 holds the kernel ABI.** Shapes, strides, buffer ownership, and numerical representations live here. The C kernel consumes validated buffer descriptions only. All numerical kernels originate from explicit array semantics in q and CBQN. The C11 layer is the one that runs: seven kernel families, 4304 checks, 0 failures.

**LLVM IR with SAW and Cryptol holds machine-level contracts.** Equivalence proofs, bounds, and arithmetic live here. SAW verifies supported LLVM representations against formal specifications, and Cryptol holds the bit-vector specs. Neither toolchain is installed on this machine, so no proof is claimed for this layer.

**Vulkan holds the compute backend.** SPIR-V compute shaders run on GPU buffers. There is no CUDA anywhere in this project. Vulkan is an acceleration backend, not a dependency for running the biological model. CPU-only builds do not require an installed Vulkan GPU. The shaders are validated here — compiled to SPIR-V and checked with spirv-val — and they await a GPU for execution.

**Mercury holds symbolic quantum gate intelligence.** A typed circuit AST, gate rules, constraints, and deterministic validation live here. Mercury controls circuit semantics with a typed gate IR. The Mercury compiler is not installed on this machine, so no typed gate IR validation runs here; the circuits are validated by the reference simulator instead.

**OpenQASM 3 holds the resulting quantum circuits.** Parameterized gates, entangling operations, measurement, and feedback live here. The circuits are `openqasm/neuron_3q.qasm` and `openqasm/retinal_pair.qasm`, and they run under the C11 reference simulator in `openqasm/sim/`.

No JavaScript, TypeScript, Python, React, PyTorch, NumPy, TensorFlow, or CUDA appears in the numerical kernel path. The kernel path is CBQN, C11, and Vulkan, and that is the whole of it.

## The C11 kernel families

Seven kernel families make up the C11 layer. Each family carries its own header, its own implementation, its own test program, and its own numerical contract in `src/c11/`. Each family validates first and computes second, in the same check order: NULL, COUNT, OVERLAP, NONFINITE. A failed call writes nothing. There is no allocation anywhere in the kernels. The cost is a second pass over the input; correctness and a clean error semantic are worth it here.

### Scale — 1055 checks

The scale family is `avian_scale.c`, and it holds two kernels: an f32 scale kernel and an i32 scale kernel. The f32 kernel multiplies every input element by one scale factor. The i32 kernel does the same for integers, and it forms the product in 64 bits and range-checks it before narrowing, so an overflowing product returns an error instead of wrapping. The validation pass checks every input and every product for finiteness in the f32 case, and every product for range in the i32 case, before any output is written. The test suite runs 1055 checks with 0 failures, comparing the C kernel against the CBQN reference on the small f32 fixture, the i32 fixture, and the large f32 fixture, plus independent cross-checks on inputs outside the fixtures. The Vulkan side carries two shaders, `scale_f32.comp` and `scale_i32.comp`, and the disassembly confirms the f32 shader multiplies with OpFMul and the i32 shader multiplies on 64-bit values and narrows only after the range check.

### Receptive-field convolution — 1069 checks

The convolution family is `avian_conv1d.c`, and it computes a 1D 3-tap receptive-field convolution over f32. The output at index i is `k0 * input[i-1] + k1 * input[i] + k2 * input[i+1]`, with zero padding at the edges: an out-of-range input reads as 0. The output length equals the input length. Each tap is a fused multiply-add, accumulated left to right in the fixed order k0, then k1, then k2. The kernel validates the three coefficients, every input element, and every accumulation for finiteness before writing anything. The test suite runs 1069 checks with 0 failures: empty and singleton inputs, explicit zero-padding boundary tests, five hand-computed cross-check cases on inputs outside any fixture, every error path with no-partial-writes checks, a signed-zero test, the small fixture, and the large fixture of 1000 elements. The small fixture uses the kernel ⟨0.5, 1, -0.25⟩, with its negative coefficient, on the input ⟨1, 2, 4, 8, 16⟩, and the expected output is ⟨0.5, 1.5, 3, 6, 20⟩. The large fixture uses the kernel ⟨0.5, 1, 0.5⟩ on the input 0 through 999. The Vulkan shader is `conv1d_f32.comp`, one output element per invocation, with fma per tap and the coefficients in push constants, and the disassembly confirms three Fma instructions, the count guard, the zero-padding boundary checks, and the output store.

### Segmented reduction — 71 checks

The segmented reduction family is `avian_reduce.c`, and it is the dendritic reduce step. It takes a data array and a segment-length array, and it returns one f32 sum per segment, in order. Each segment sums left to right in f32, starting from +0.0f, and no accumulation crosses a segment boundary. A segment of length 0 is an empty segment, and its sum is +0.0f. The total of all segment lengths must equal the input count. The kernel validates each length as non-negative and compares the running total against the input count as it grows, so the total can never exceed the count or wrap, and the total must equal the input count at the end. A negative length is reported as a count error. The kernel also checks the output against the input and against the lengths for overlap, since the lengths are read throughout, and it checks every input element and every accumulation for finiteness. The test suite runs 71 checks with 0 failures, including the small fixture with lengths ⟨2, 0, 3⟩ on the input ⟨1, 2, 4, 8, 16⟩, giving the expected sums ⟨3, 0, 28⟩ with the empty segment in the middle, and the large fixture of 1000 inputs across four segments of 250, giving ⟨31125, 93625, 156125, 218625⟩. The Vulkan shader is `reduce_segments.comp`, one invocation per segment, with the lengths and host-computed offsets in buffers and left-to-right f32 accumulation from +0.0f.

### Sparse gather and scatter — 1212 checks

The gather and scatter family is `avian_gather.c`, and it is the synaptic sparse step. It holds two operations over f32, sharing one validation scheme. Gather takes an input array and an index array, and it returns `output[i] = input[index[i]]` for each index. The output length equals the index count. Scatter takes an input array and an index array, and it writes `output[index[i]] = input[i]` for each input element. The output starts as all +0.0f, and then each input element is written to its index. When two input elements target the same index, the last one in input order wins. An index that is never written stays +0.0f. Every index must be a non-negative integer below the bound, and an out-of-range index is reported as a count error. The kernel checks the output against the input and against the indices for overlap, and it checks every input element for finiteness. The test suite runs 1212 checks with 0 failures across four fixtures: a small gather, a small scatter, a large gather, and a large scatter. The small scatter fixture exercises both required cases at once: a duplicate index, where the last write wins, and an unwritten index, which stays +0.0f. The large fixtures carry 1000 elements each. The Vulkan shader is `gather_scatter.comp`, and it implements both operations in one file with a mode push constant. Scatter in the shader uses one invocation per output element, and each invocation scans all inputs in order and keeps the last input targeting its output element, or +0.0f when none does. That mapping is race-free by construction, because each invocation writes only its own output element, and it preserves last-wins deterministically because the scan order is the input order. The alternative, one invocation per input element, races on duplicate indices, and an atomic write picks an unspecified winner instead of the last in input order. The cost is O(output count times input count) work for scatter, and the shader contract states that plainly: the host uses the shader only for small index sets, and large scatters run on the C kernel or on a sorted scatter.

### Genomic coordinate transform — 407 checks

The coordinate transform family is `avian_geom.c`, and it is the geometry layer: it maps genomic coordinates to spatial positions. It takes N points, each three f32 values (x, y, z) stored flat, and it applies a 3D affine transform: a 3x3 matrix M in row-major order, nine f32 values, and a translation vector t of three f32 values. The output at point i and component r is `M[3r] * in[3i] + M[3r+1] * in[3i+1] + M[3r+2] * in[3i+2] + t[r]`. The accumulation is fused multiply-add, left to right, matrix row term by term, and then the translation through one more fused multiply-add. The output length is 3N. The kernel guards against the 3N product overflowing, checks the output against the input, the matrix, and the translation for overlap, and checks every input coordinate, every matrix entry, every translation entry, and every accumulation step for finiteness. The test suite runs 407 checks with 0 failures, including empty and singleton inputs, hand-computed cross-checks, every error path, and signed zero. The small fixture carries four points, the matrix ⟨1, 0.5, 0, 0, 2, -0.25, 0, 0, 1⟩ with its negative entry and off-diagonal terms, and the translation ⟨1, -2, 0.5⟩. The mapping reads:

| Point | Input (x, y, z) | Output (x, y, z) |
| --- | --- | --- |
| P0 | (1, 2, 4) | (3, 1, 4.5) |
| P1 | (-1, 0, 2) | (0, -2.5, 2.5) |
| P2 | (0, 0, 0) | (1, -2, 0.5) |
| P3 | (2, -4, 8) | (1, -12, 8.5) |

The large fixture carries 100 points, a diagonal scaling matrix, and the translation ⟨1, 0, -1⟩. The Vulkan shader is `geom_transform.comp`, one invocation per point, with four fused multiply-adds per output component — three matrix terms plus the translation — and the disassembly confirms all four, plus the point-count guard, the input, matrix, and translation loads, and the output store. The front end plots these same four points on the genome and tabulates the same mapping, so the diagram and the kernel agree on the same data.
