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
