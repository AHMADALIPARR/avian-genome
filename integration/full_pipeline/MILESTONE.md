First milestone — morphic digital twin of a small avian
retinal-tectal circuit
========================================================

Scope
-----
This milestone is a working digital twin of a small avian
retinal-tectal circuit: one retinal ganglion cell (neuron A)
driving one tectal cell (neuron B) through a controlled-NOT
coupling that represents the retinal-to-tectal projection.
Each neuron keeps its own 3-qubit circuit template. The
coupling is a single entangling gate between them, not one
entangled register per neuron.

What is built
-------------
  - OpenQASM templates: openqasm/neuron_3q.qasm (3-qubit
    neuron) and openqasm/retinal_pair.qasm (6-qubit
    retinal-tectal circuit). Both are verified by the
    reference simulator (openqasm/sim, 20 checks, 0 failures).
  - Reference simulator driver: openqasm/sim/qsim_run.
    It runs a template with concrete angles and prints the
    measurement probabilities.
  - Ruby orchestrator: ruby/retinal_tectal.rb. It defines
    the circuit, generates the concrete OpenQASM, drives
    the simulator, and reports the twin's state for three
    stimulus conditions (dark, dim, bright). It builds the
    simulator on demand, so it works from a clean tree.
  - Generated fixture: fixtures/quantum/retinal_pair_bright.qasm
    is the concrete OpenQASM for the bright condition,
    produced by the orchestrator.

How to run it
-------------
  ruby ruby/retinal_tectal.rb

It prints the spike probability of each neuron for each
condition and writes the bright-condition OpenQASM to
fixtures/quantum/retinal_pair_bright.qasm. It exits 0 only
when every simulator call succeeds and every reported
probability is in [0, 1].

Results (measured on this machine)
----------------------------------
  dark   (thetaA=0):    A silent (0.000000), B silent (0.000000)
  dim    (thetaA=pi/2): A graded (0.500000), B graded (0.500000)
  bright (thetaA=pi):   A spiking (1.000000), B spiking (1.000000)

The tectal cell follows the retinal cell: B spikes exactly
when A spikes, because the coupling is a controlled-NOT from
A's output to B's input and B is at rest (thetaB = 0).

Constraints (what is substituted and what is missing)
------------------------------------------------------
  - Self prototypes: the Self toolchain is not installed on
    this machine. The neurons are plain Ruby objects here,
    not Self prototypes. Self is the intended representation
    for morphic prototypes; the Ruby objects are a stand-in
    for the orchestration layer only.
    Boundary: self/BOUNDARY.md.
  - q genome: q/kdb+ is commercial and not installed here.
    The genome is a plain parameter hash in the orchestrator,
    not a q program. The genomic geometry (coordinate maps,
    spatial indexing, adjacency tensors) is not computed by
    q here.
    Boundary: q/BOUNDARY.md.
  - Mercury validation: the Mercury compiler (mmc) is not
    installed here. No typed gate IR validation is run. The
    templates are validated by the reference simulator.
    Boundary: mercury/BOUNDARY.md.
  - Vulkan geometry: the scale shaders are built and
    validated (vulkan/shaders), but no geometry shader for
    this circuit is written yet, and no GPU is present to
    run one. The twin's geometry is the circuit layout,
    not a rendered field.

None of these is silently substituted. Each is recorded here,
in the corresponding BOUNDARY.md, and in
verification/VERIFICATION_STATUS.md.
